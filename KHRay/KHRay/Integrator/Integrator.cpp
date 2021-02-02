#include "Integrator.h"
#include "../Texture2D.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

#include <iostream>
#include <future>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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
			color *= (1.0f / 2.2f); // Gamma correct

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

class ProgressReport
{
public:
	ProgressReport(const std::string& Header)
		: Header(Header)
	{

	}

	void Update(double NewProgress)
	{
		CurrentProgress += NewProgress;
	}
	void Print()
	{
		constexpr double BarWidth = 70.0;
		constexpr double MaxProgress = 100.0;

		std::cout << Header + ": [";
		double pos = BarWidth * CurrentProgress;
		for (double i = 0; i < BarWidth; ++i)
		{
			if (i <= pos) std::cout << "=";
			else std::cout << " ";
		}
		std::cout << "] " << int(std::min(CurrentProgress * 100.0, MaxProgress)) << "%\r";
		std::cout.flush();
	}
private:
	double CurrentProgress = 0.0;
	std::string Header;
};

int Integrator::Render(Scene& Scene, Sampler& Sampler)
{
	Scene.Camera.AspectRatio = float(Width) / float(Height);

	// Multi-threaded tile rendering
	static_assert(Width % NumXTiles == 0);
	static_assert(Height % NumYTiles == 0);
	constexpr int WidthIncrement = Width / NumXTiles;
	constexpr int HeightIncrement = Height / NumYTiles;
	std::future<FilmTile> Futures[NumTiles];
	for (int y = 0; y < NumYTiles; ++y)
	{
		RECT Rect = {};
		Rect.top = y * HeightIncrement;
		Rect.bottom = (y + 1) * HeightIncrement;

		for (int x = 0; x < NumXTiles; ++x)
		{
			Rect.left = x * WidthIncrement;
			Rect.right = (x + 1) * WidthIncrement;

			int index = y * NumXTiles + x;
			Futures[index] = std::async(std::launch::async, [&]() -> FilmTile
			{
				auto pSampler = Sampler.Clone();
				pSampler->StartPixel(Rect.left, Rect.top);
				float InvNumSPP = 1.0f / float(pSampler->GetNumSamplesPerPixel());

				int TileWidth = Rect.right - Rect.left;
				int TileHeight = Rect.bottom - Rect.top;

				FilmTile Tile = {};
				Tile.Rect = Rect;
				Tile.Data.reserve(TileWidth * TileHeight);

				// Render
				{
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

							L *= InvNumSPP;

							Tile.Data.push_back(L);
						}
					}
				}

				return Tile;
			});
		}
	}

	constexpr double NewProgress = 100.0 / NumTiles / 100.0;

	// Reports the progress of the rendering and waits for each
	// future
	ProgressReport ProgressReport("Render");
	for (auto& Future : Futures)
	{
		ProgressReport.Print();
		Future.wait();
		ProgressReport.Update(NewProgress);
	}
	ProgressReport.Print();
	std::cout << std::endl;

	// Get the result of each tile and save it for later
	FilmTile FilmTiles[NumTiles];
	for (int i = 0; i < NumTiles; ++i)
	{
		FilmTiles[i] = Futures[i].get();
	}

	// Write per tile data to a output texture and save it on disk
	Texture2D<RGBSpectrum> Output(Width, Height);
	for (auto& Tile : FilmTiles)
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