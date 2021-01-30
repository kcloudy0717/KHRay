#include "Integrator.h"
#include "../Texture2D.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

#include <iostream>
#include <future>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

int Save(const Texture2D<RGBSpectrum>& Image, UINT NumChannels)
{
	// Saves a input image as a png using stb
	std::unique_ptr<BYTE[]> Pixels = std::make_unique<BYTE[]>(Image.Width * Image.Height * NumChannels);

	INT index = 0;
	for (INT y = INT(Image.Height) - 1; y >= 0; --y)
	{
		for (INT x = 0; x < INT(Image.Width); ++x)
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
	constexpr INT WidthIncrement = Width / NumXTiles;
	constexpr INT HeightIncrement = Height / NumYTiles;
	std::future<FilmTile> Futures[NumTiles];
	for (INT y = 0; y < NumYTiles; ++y)
	{
		RECT Rect = {};
		Rect.top = y * HeightIncrement;
		Rect.bottom = (y + 1) * HeightIncrement;

		for (INT x = 0; x < NumXTiles; ++x)
		{
			Rect.left = x * WidthIncrement;
			Rect.right = (x + 1) * WidthIncrement;

			INT index = y * NumXTiles + x;
			Futures[index] = std::async(std::launch::async, [&]() -> FilmTile
			{
				auto pSampler = Sampler.Clone();
				pSampler->StartPixel(Rect.left, Rect.top);
				float InvNumSPP = 1.0f / float(pSampler->GetNumSamplesPerPixel());

				INT TileWidth = Rect.right - Rect.left;
				INT TileHeight = Rect.bottom - Rect.top;

				FilmTile Tile = {};
				Tile.Rect = Rect;
				Tile.Data.reserve(TileWidth * TileHeight);

				// Render
				{
					// For each pixel and pixel sample
					for (INT y = Rect.top; y < Rect.bottom; ++y)
					{
						for (INT x = Rect.left; x < Rect.right; ++x)
						{
							Spectrum L(0);
							for (INT sample = 0; sample < pSampler->GetNumSamplesPerPixel(); ++sample)
							{
								auto sampleJitter = pSampler->Get2D();

								auto u = (float(x) + sampleJitter.x) / (float(Width) - 1);
								auto v = (float(y) + sampleJitter.y) / (float(Height) - 1);

								Ray ray = Scene.Camera.GetRay(u, v);

								L += Li(ray, Scene);
							}

							L *= InvNumSPP;

							Tile.Data.push_back(L);
						}
					}
				}

				return Tile;
			});

			Futures[index].wait();
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
	for (INT i = 0; i < NumTiles; ++i)
	{
		FilmTiles[i] = Futures[i].get();
	}

	// Write per tile data to a output texture and save it on disk
	Texture2D<RGBSpectrum> Output(Width, Height);
	for (auto& Tile : FilmTiles)
	{
		INT Index = 0;
		for (INT y = Tile.Rect.top; y < Tile.Rect.bottom; ++y)
		{
			for (INT x = Tile.Rect.left; x < Tile.Rect.right; ++x)
			{
				Output.SetPixel(x, y, Tile.Data[Index]);
				Index++;
			}
		}
	}

	return Save(Output, NumChannels);
}

PathIntegrator::PathIntegrator(int MaxDepth)
	: MaxDepth(MaxDepth)
{

}

Spectrum PathIntegrator::Li(const Ray& Ray, const Scene& Scene)
{
	Spectrum L(0), beta(1);

	for (int bounce = 0; ; ++bounce)
	{
		Intersection Intersection = {};
		bool foundIntersection = Scene.Intersect(Ray, &Intersection);

		if (foundIntersection)
		{
			L += beta * Spectrum(Intersection.Normal.x, Intersection.Normal.y, Intersection.Normal.z);
		}
		else
		{
			float t = 0.5f * (Ray.Direction.y + 1.0f);
			L += beta * (1.0f - t) * Spectrum(1.0f, 1.0f, 1.0f) + t * Spectrum(0.5f, 0.7f, 1.0f);
		}

		if (!foundIntersection || bounce >= MaxDepth)
		{
			break;
		}
	}

	return L;
}

std::unique_ptr<PathIntegrator> CreatePathIntegrator(int MaxDepth)
{
	return std::unique_ptr<PathIntegrator>(new PathIntegrator(MaxDepth));
}
