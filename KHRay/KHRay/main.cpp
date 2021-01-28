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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <embree/rtcore.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <filesystem>
#include <future>

#include "Utility.h"

#include "Ray.h"
#include "Vertex.h"
#include "Camera.h"
#include "Random.h"
#include "Spectrum.h"
#include "Device.h"
#include "AccelerationStructure.h"

#include "Texture2D.h"

using namespace DirectX;

class ProgressBar
{
public:
	void Update(double NewProgress)
	{
		CurrentProgress += NewProgress;
	}
	void Print()
	{
		constexpr int BarWidth = 70;
		constexpr double MaxProgress = 100.0;

		std::cout << "[";
		int pos = BarWidth * CurrentProgress;
		for (int i = 0; i < BarWidth; ++i)
		{
			if (i <= pos) std::cout << "=";
			else std::cout << " ";
		}
		std::cout << "] " << int(std::min(CurrentProgress * 100.0, MaxProgress)) << "%\r";
		std::cout.flush();
	}
private:
	double CurrentProgress = 0.0;
};

constexpr INT MaxDepth = 1;
constexpr INT NumSamplesPerPixel = 10;

struct FilmTile
{
	RECT Rect;
	std::vector<Spectrum> Data;
};

constexpr INT NumXTiles = 3;
constexpr INT NumYTiles = 3;
constexpr INT NumTiles = NumXTiles * NumYTiles;

struct RayPayload
{
	Spectrum Radiance = Spectrum(0);
	Spectrum Throughput = Spectrum(1);

	XMFLOAT3 Position = {};
	XMFLOAT3 Direction = {};
};

void TraceRay(const TopLevelAccelerationStructure& Scene, const Ray& Ray, RayPayload& RayPayload)
{
	/*
	* The intersect context can be used to set intersection
	* filters or flags, and it also contains the instance ID stack
	* used in multi-level instancing.
	*/
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	/*
	 * The ray hit structure holds both the ray and the hit.
	 * The user must initialize it properly -- see API documentation
	 * for rtcIntersect1() for details.
	 */
	RTCRayHit rayhit = Ray;

	/*
	* There are multiple variants of rtcIntersect. This one
	* intersects a single ray with the scene.
	*/
	rtcIntersect1(Scene, &context, &rayhit);

	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		const auto& hit = rayhit.hit;
		auto Instance = Scene[rayhit.hit.geomID];
		auto GeometryDesc = (*Instance.pBLAS)[rayhit.hit.instID[0]];

		const unsigned int idx0 = GeometryDesc.pIndices[hit.primID * 3 + 0];
		const unsigned int idx1 = GeometryDesc.pIndices[hit.primID * 3 + 1];
		const unsigned int idx2 = GeometryDesc.pIndices[hit.primID * 3 + 2];

		const Vertex& vtx0 = GeometryDesc.pVertices[idx0];
		const Vertex& vtx1 = GeometryDesc.pVertices[idx1];
		const Vertex& vtx2 = GeometryDesc.pVertices[idx2];

		XMFLOAT3 barycentrics = { 1.f - hit.u - hit.v, hit.u, hit.v };
		Vertex v = BarycentricInterpolation(vtx0, vtx1, vtx2, barycentrics);

		RayPayload.Radiance += Spectrum(v.Normal.x, v.Normal.y, v.Normal.z);
		/* Note how geomID and primID identify the geometry we just hit.
		* We could use them here to interpolate geometry information,
		* compute shading, etc.
		* Since there is only a single triangle in this scene, we will
		* get geomID=0 / primID=0 for all hits.
		* There is also instID, used for instancing. See
		* the instancing tutorials for more information
		*/
	}
	else
	{
		float t = 0.5f * (Ray.Direction.y + 1.0f);
		RayPayload.Radiance += (1.0f - t) * Spectrum(1.0f, 1.0f, 1.0f) + t * Spectrum(0.5f, 0.7f, 1.0f);
	}
}

Spectrum PathTrace(const TopLevelAccelerationStructure& Scene, Ray Ray)
{
	RayPayload RayPayload = {};
	RayPayload.Radiance = Spectrum(0);
	RayPayload.Throughput = Spectrum(1);

	for (INT i = 0; i < MaxDepth; ++i)
	{
		TraceRay(Scene, Ray, RayPayload);

		Ray.Origin = RayPayload.Position;
		Ray.Direction = RayPayload.Direction;
	}

	return RayPayload.Radiance;
}

FilmTile Render(XMINT2 Resolution, RECT Rect, const TopLevelAccelerationStructure& Scene, const Camera& Camera)
{
	INT TileWidth = Rect.right - Rect.left;
	INT TileHeight = Rect.bottom - Rect.top;

	FilmTile Tile = {};
	Tile.Rect = Rect;
	Tile.Data.reserve(TileWidth * TileHeight);

	// Render
	{
		for (INT y = Rect.top; y < Rect.bottom; ++y)
		{
			for (INT x = Rect.left; x < Rect.right; ++x)
			{
				Spectrum color(0);
				for (INT sample = 0; sample < NumSamplesPerPixel; ++sample)
				{
					auto u = (float(x) + random()) / (float(Resolution.x) - 1);
					auto v = (float(y) + random()) / (float(Resolution.y) - 1);

					Ray ray = Camera.GetRay(u, v);

					color += PathTrace(Scene, ray);
				}

				color /= NumSamplesPerPixel;

				Tile.Data.push_back(color);
			}
		}
	}

	return Tile;
}

int Save(const Texture2D<RGBSpectrum>& Image, UINT NumChannels)
{
	std::unique_ptr<BYTE[]> Pixels = std::make_unique<BYTE[]>(Image.Width * Image.Height * NumChannels);

	INT index = 0;
	for (INT y = INT(Image.Height) - 1; y >= 0; --y)
	{
		for (INT x = 0; x < INT(Image.Width); ++x)
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

	if (stbi_write_png("Output.png", Image.Width, Image.Height, 3, Pixels.get(), Image.Width * NumChannels))
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
	const INT Width = 1920;
	const INT Height = 1080;
	const INT NumChannels = 3;

	std::filesystem::path ExecutableFolderPath = std::filesystem::path(argv[0]).parent_path();
	std::filesystem::path ModelFolderPath = ExecutableFolderPath / "Models";

	InitializeSampledSpectrums();

	Device Device;
	TopLevelAccelerationStructure Scene(Device);

	/*BottomLevelAccelerationStructure BurrPuzzle(Device);
	BurrPuzzle.AddGeometry(ModelFolderPath / "BurrPuzzle.obj");
	BurrPuzzle.Generate();

	RAYTRACING_INSTANCE_DESC BurrPuzzleInstance = {};
	BurrPuzzleInstance.Transform.SetScale(20, 20, 20);
	BurrPuzzleInstance.Transform.Translate(0, 0, 2);
	BurrPuzzleInstance.Transform.Rotate(0, 30.0_Deg, 0);
	BurrPuzzleInstance.pBLAS = &BurrPuzzle;

	g_Scene.AddBottomLevelAccelerationStructure(BurrPuzzleInstance);*/

	BottomLevelAccelerationStructure BreakfastRoom(Device);
	BreakfastRoom.AddGeometry(ModelFolderPath / "breakfast_room" / "breakfast_room.obj");
	BreakfastRoom.Generate();

	RAYTRACING_INSTANCE_DESC BreakfastRoomInstance = {};
	BreakfastRoomInstance.Transform.SetScale(5, 5, 5);
	BreakfastRoomInstance.Transform.Translate(0, 0, 20);
	BreakfastRoomInstance.Transform.Rotate(0, 180.0_Deg, 0);
	BreakfastRoomInstance.pBLAS = &BreakfastRoom;

	Scene.AddBottomLevelAccelerationStructure(BreakfastRoomInstance);
	Scene.Generate();

	Camera Camera;
	Camera.AspectRatio = float(Width) / float(Height);
	Camera.Transform.Translate(0, 5, 0);

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
			Futures[index] = std::async(std::launch::async, Render, XMINT2(Width, Height), Rect, std::cref(Scene), std::cref(Camera));
		}
	}

	constexpr double NewProgress = 100.0 / NumTiles / 100.0;

	ProgressBar ProgressBar;
	for (auto& Future : Futures)
	{
		ProgressBar.Update(NewProgress);
		ProgressBar.Print();
		Future.wait();
	}
	ProgressBar.Print();
	std::cout << std::endl;

	FilmTile FilmTiles[NumTiles];
	for (INT i = 0; i < NumTiles; ++i)
	{
		FilmTiles[i] = Futures[i].get();
	}

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