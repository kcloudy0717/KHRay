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

#include <vector>
#include <random>

#include "Ray.h"
#include "Triangle.h"
#include "Camera.h"
#include "Spectrum.h"
#include "Texture2D.h"

using namespace DirectX;

const INT Width = 1280;
const INT Height = 720;
const INT NumChannels = 3;

int Save(const Texture2D<RGBSpectrum>& Image)
{
	std::unique_ptr<BYTE[]> Pixels = std::make_unique<BYTE[]>(Width * Height * NumChannels);

	INT index = 0;
	for (INT y = Height - 1; y >= 0; --y)
	{
		for (INT x = 0; x < Width; ++x)
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

RGBSpectrum Color(const Ray& Ray)
{
	Vertex v0({ -.5, -.5, 2 });
	Vertex v1({ 0.0, +.5, 2 });
	Vertex v2({ +.5, -.5, 2 });
	Triangle tri(v0, v1, v2, { 1, 0, 0 });
	XMFLOAT3 barycentrics;
	float t;
	if (RayIntersectsTriangle(Ray, tri, barycentrics, t))
	{
		return RGBSpectrum(1, 0, 0);
	}

	t = 0.5 * (Ray.Direction.y + 1.0);
	return (1.0 - t) * RGBSpectrum(1.0, 1.0, 1.0) + t * RGBSpectrum(0.5, 0.7, 1.0);
}

int main(int argc, char** argv)
{
#if defined(_DEBUG)
	ENABLE_LEAK_DETECTION();
	SET_LEAK_BREAKPOINT(-1);
#endif

	InitSampledSpectrums();

	Texture2D<RGBSpectrum> Output(Width, Height);

	Camera Camera;
	Camera.AspectRatio = float(Width) / float(Height);

	for (INT y = 0; y < Height; ++y)
	{
		for (INT x = 0; x < Width; ++x)
		{
			auto u = double(x) / (Width - 1);
			auto v = double(y) / (Height - 1);

			Ray ray = Camera.GetRay(u, v);
			RGBSpectrum color = Color(ray);

			Output.SetPixel(x, y, color);
		}
	}

	return Save(Output);
}