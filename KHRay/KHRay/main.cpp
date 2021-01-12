// main.cpp : Defines the entry point for the application.
//
#if defined(_DEBUG)
// memory leak
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#define ENABLE_LEAK_DETECTION() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#define SET_LEAK_BREAKPOINT(x) _CrtSetBreakAlloc(x)
#else
#define ENABLE_LEAK_DETECTION() 0
#define SET_LEAK_BREAKPOINT(X) X
#endif

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "Vector2.h"
#include "Vector3.h"
#include "Ray.h"
#include "Spectrum.h"
#include "Texture2D.h"

const UINT Width = 1280;
const UINT Height = 720;
const UINT NumChannels = 3;

int Save(const Texture2D<RGBSpectrum>& Image)
{
	std::unique_ptr<BYTE[]> Pixels = std::make_unique<BYTE[]>(Width * Height * NumChannels);

	UINT index = 0;
	for (UINT y = 0; y < Height; ++y)
	{
		for (UINT x = 0; x < Width; ++x)
		{
			auto color = Image.GetPixel(x, y);
			auto ir = int(255.99 * color[0]);
			auto ig = int(255.99 * color[1]);
			auto ib = int(255.99 * color[2]);

			Pixels[index++] = ir;
			Pixels[index++] = ig;
			Pixels[index++] = ib;
		}
	}

	if (stbi_write_png("Output.png", Width, Height, 3, Pixels.get(), Width * NumChannels))
	{
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int main(int argc, char** argv)
{
#if defined(_DEBUG)
	ENABLE_LEAK_DETECTION();
	SET_LEAK_BREAKPOINT(-1);
#endif

	InitSampledSpectrums();

	Texture2D<RGBSpectrum> Output(Width, Height);

	for (UINT y = 0; y < Height; ++y)
	{
		for (UINT x = 0; x < Width; ++x)
		{

			RGBSpectrum color(1, 0, 0);
			Output.SetPixel(x, y, color);
		}
	}

	return Save(Output);
}