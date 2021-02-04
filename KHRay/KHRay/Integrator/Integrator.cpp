#include "Integrator.h"
#include "../Texture2D.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

#include <iostream>
#include <mutex>
#include <thread>
#include <future>

#include <ppl.h>
using namespace concurrency;

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#ifdef _DEBUG
#define MULTI_THREADED 0
#else
#define MULTI_THREADED 1
#endif

int Save(const Texture2D<RGBSpectrum>& Image, int NumChannels)
{
	// Saves a input image as a png using stb
	std::unique_ptr<BYTE[]> Pixels = std::make_unique<BYTE[]>(Image.Width * Image.Height * NumChannels);

	int index = 0;
	for (int y = int(Image.Height) - 1; y >= 0; --y)
	{
		for (int x = 0; x < int(Image.Width); ++x)
		{
			auto color = Image.GetPixel(x, y);
			//color *= (1.0f / 2.2f); // Gamma correct

			auto ir = int(255.99 * color[0]);
			auto ig = int(255.99 * color[1]);
			auto ib = int(255.99 * color[2]);

			Pixels[index++] = ir;
			Pixels[index++] = ig;
			Pixels[index++] = ib;
		}
	}

	if (stbi_write_png("Output.png", Image.Width, Image.Height, 3, Pixels.get(), Image.Width * NumChannels))
	{
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

static int TerminalWidth()
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h == INVALID_HANDLE_VALUE || !h)
	{
		fprintf(stderr, "GetStdHandle() call failed");
		return 80;
	}
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
	GetConsoleScreenBufferInfo(h, &bufferInfo);
	return bufferInfo.dwSize.X;
}

class ProgressReport
{
public:
	ProgressReport(const std::string& Header, int TotalProgress)
		: Header(Header)
		, TotalProgress(TotalProgress)
	{
		Thread = std::thread([this]()
		{
			Print();
		});
	}

	~ProgressReport()
	{
		Shutdown = true;
		Thread.join();
		printf("\n");
	}

	void Update(int NewProgress = 1)
	{
		CurrentProgress += NewProgress;
	}

	void Print()
	{
		auto barLength = TerminalWidth() - 28;
		auto totalPlusses = std::max(2, barLength - (int)Header.size());
		auto progressPrinted = 0;

		// Initialize progress string
		auto bufLen = Header.size() + totalPlusses + 64;
		std::unique_ptr<char[]> buf(new char[bufLen]);
		snprintf(buf.get(), bufLen, "\r%s: [", Header.c_str());
		char* curSpace = buf.get() + strlen(buf.get());
		char* s = curSpace;
		for (int i = 0; i < totalPlusses; ++i) *s++ = ' ';
		*s++ = ']';
		*s++ = ' ';
		*s++ = '\0';
		fputs(buf.get(), stdout);
		fflush(stdout);

		std::chrono::milliseconds sleepDuration(250);
		int iterCount = 0;
		while (!Shutdown)
		{
			std::this_thread::sleep_for(sleepDuration);

			// Periodically increase sleepDuration to reduce overhead of
			// updates.
			++iterCount;
			if (iterCount == 10)
				// Up to 0.5s after ~2.5s elapsed
				sleepDuration *= 2;
			else if (iterCount == 70)
				// Up to 1s after an additional ~30s have elapsed.
				sleepDuration *= 2;
			else if (iterCount == 520)
				// After 15m, jump up to 5s intervals
				sleepDuration *= 5;

			float percentDone = float(CurrentProgress) / float(TotalProgress);
			int ProgressNeeded = (int)std::round(totalPlusses * percentDone);
			while (progressPrinted < ProgressNeeded)
			{
				*curSpace++ = '=';
				++progressPrinted;
			}
			fputs(buf.get(), stdout);

			printf(" %i %%", int(std::min(percentDone * 100.0f, 100.0f)));

			fflush(stdout);
		}
	}
private:
	std::string Header;
	std::atomic<int> TotalProgress;

	std::atomic<int> CurrentProgress = 0;
	std::atomic<bool> Shutdown = false;
	std::thread Thread;
};

class TileManager
{
public:
	void Initialize(int Width, int Height)
	{
		for (int y = 0; y < Height; y += FilmTile::TILE_SIZE)
		{
			for (int x = 0; x < Width; x += FilmTile::TILE_SIZE)
			{
				int minX = x, minY = y;
				int maxX = x + FilmTile::TILE_SIZE, maxY = y + FilmTile::TILE_SIZE;
				maxX = maxX <= Width ? maxX : Width;
				maxY = maxY <= Height ? maxY : Height;

				FilmTile Tile;

				RECT Rect = { minX, minY, maxX, maxY };
				int TileWidth = Rect.right - Rect.left;
				int TileHeight = Rect.bottom - Rect.top;

				Tile.Rect = Rect;
				Tile.Data.reserve(TileWidth * TileHeight);

				FilmTiles.emplace_back(std::move(Tile));
			}
		}
	}

	FilmTile& operator[](int i)
	{
		return FilmTiles[i];
	}

	auto size() const
	{
		return FilmTiles.size();
	}

	auto begin()  { return FilmTiles.begin(); }
	auto end() { return FilmTiles.end(); }
private:
	std::vector<FilmTile> FilmTiles;
};

int Integrator::Render(Scene& Scene, Sampler& Sampler)
{
	Scene.Generate();

	Scene.Camera.AspectRatio = float(Width) / float(Height);

	TileManager TileManager;
	TileManager.Initialize(Width, Height);

	ProgressReport ProgressReport("Render", (int)TileManager.size());

	int NumTiles = (int)TileManager.size();
	parallel_for(0, NumTiles, [&](int i)
	{
		auto& Tile = TileManager[i];
		auto Rect = Tile.Rect;

		auto pSampler = Sampler.Clone();
		pSampler->StartPixel(Rect.left, Rect.top);

		// Render
		// For each pixel and pixel sample
		for (int y = Rect.top; y < Rect.bottom; ++y)
		{
			for (int x = Rect.left; x < Rect.right; ++x)
			{
				Spectrum L(0);
				for (int sample = 0; sample < pSampler->GetNumSamplesPerPixel(); ++sample)
				{
					auto sampleJitter = pSampler->Get2D();

					auto u = (float(x) + sampleJitter.x) / (float(Width) - 1);
					auto v = (float(y) + sampleJitter.y) / (float(Height) - 1);

					Ray ray = Scene.Camera.GetRay(u, v);

					L += Li(ray, Scene, *pSampler);
				}

				L /= float(pSampler->GetNumSamplesPerPixel());

				Tile.Data.push_back(L);
			}
		}

		ProgressReport.Update();
	});

	// Write per tile data to a output texture and save it on disk
	Texture2D<RGBSpectrum> Output(Width, Height);
	for (auto& Tile : TileManager)
	{
		int Index = 0;
		for (int y = Tile.Rect.top; y < Tile.Rect.bottom; ++y)
		{
			for (int x = Tile.Rect.left; x < Tile.Rect.right; ++x)
			{
				Output.SetPixel(x, y, Tile.Data[Index]);
				Index++;
			}
		}
	}

	return Save(Output, NumChannels);
}