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

#include <embree/rtcore.h>

#include <vector>
#include <unordered_map>
#include <random>
#include <filesystem>

#include "Utility.h"

#include "Ray.h"
#include "Vertex.h"
#include "Camera.h"
#include "Spectrum.h"
#include "AccelerationStructure.h"

#include "Texture2D.h"

using namespace DirectX;

static RTCDevice g_RTCDevice = nullptr;

void errorFunction(void* userPtr, RTCError error, const char* str)
{
	printf("error %d: %s\n", error, str);
}

void InitializeRTCDevice()
{
	g_RTCDevice = rtcNewDevice(NULL);

	if (!g_RTCDevice)
	{
		printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));
	}

	rtcSetDeviceErrorFunction(g_RTCDevice, errorFunction, NULL);
}

struct RayPayload
{
	RGBSpectrum Radiance;
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

		RayPayload.Radiance = RGBSpectrum(v.Normal.x, v.Normal.y, v.Normal.z);
		/* Note how geomID and primID identify the geometry we just hit.
		 * We could use them here to interpolate geometry information,
		 * compute shading, etc.
		 * Since there is only a single triangle in this scene, we will
		 * get geomID=0 / primID=0 for all hits.
		 * There is also instID, used for instancing. See
		 * the instancing tutorials for more information */
	}
	else
	{
		float t = 0.5f * (Ray.Direction.y + 1.0);
		RayPayload.Radiance = (1.0f - t) * RGBSpectrum(1.0f, 1.0f, 1.0f) + t * RGBSpectrum(0.5f, 0.7f, 1.0f);
	}
}

using namespace DirectX;

const INT Width = 1280;
const INT Height = 720;
const INT NumChannels = 3;
static std::filesystem::path ExecutableFolderPath;
static std::filesystem::path ModelFolderPath;

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

int main(int argc, char** argv)
{
#if defined(_DEBUG)
	ENABLE_LEAK_DETECTION();
	SET_LEAK_BREAKPOINT(-1);
#endif
	ExecutableFolderPath = std::filesystem::path(argv[0]).parent_path();
	ModelFolderPath = ExecutableFolderPath / "Models";

	InitializeSampledSpectrums();
	/* Initialization. All of this may fail, but we will be notified by
	* our errorFunction. */
	InitializeRTCDevice();

	BottomLevelAccelerationStructure BurrPuzzle(g_RTCDevice);
	BurrPuzzle.AddGeometry(ModelFolderPath / "BurrPuzzle.obj");
	BurrPuzzle.Generate();

	TopLevelAccelerationStructure Scene(g_RTCDevice);

	RAYTRACING_INSTANCE_DESC BurrPuzzleInstance = {};
	BurrPuzzleInstance.Transform.SetScale(20, 20, 20);
	BurrPuzzleInstance.Transform.Translate(0, 0, 2);
	BurrPuzzleInstance.Transform.Rotate(0, 30.0_Deg, 0);
	BurrPuzzleInstance.pBLAS = &BurrPuzzle;

	Scene.AddBottomLevelAccelerationStructure(BurrPuzzleInstance);

	Camera Camera;
	Camera.AspectRatio = float(Width) / float(Height);

	Texture2D<RGBSpectrum> Output(Width, Height);

	Scene.Generate();
	for (INT y = 0; y < Height; ++y)
	{
		for (INT x = 0; x < Width; ++x)
		{
			float u = float(x) / (float(Width) - 1.0f);
			float v = float(y) / (float(Height) - 1.0f);

			RayPayload RayPayload = {};

			Ray ray = Camera.GetRay(u, v);
			TraceRay(Scene, ray, RayPayload);

			RGBSpectrum color = RayPayload.Radiance;

			Output.SetPixel(x, y, color);
		}
	}

	/* Though not strictly necessary in this example, you should
	* always make sure to release resources allocated through Embree. */
	rtcReleaseDevice(g_RTCDevice);

	return Save(Output);
}