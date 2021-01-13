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

#include "WaveFrontReader.h"
#include "Utility.h"

#include "Ray.h"
#include "Vertex.h"
#include "Camera.h"
#include "Spectrum.h"
#include "Texture2D.h"

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

void TraceRay(RTCScene RTCScene, const Ray& Ray, RayPayload& RayPayload)
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
	rtcIntersect1(RTCScene, &context, &rayhit);

	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		RayPayload.Radiance = RGBSpectrum(1, 0, 0);
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

struct Mesh
{
	RTCScene Scene;
	RTCGeometry Geometry;
};

class AccelerationStructure
{
public:
	AccelerationStructure()
	{
		Scene = rtcNewScene(g_RTCDevice);
	}

	~AccelerationStructure()
	{
		if (Scene)
		{
			rtcReleaseScene(Scene);
			Scene = nullptr;
		}
	}

	operator RTCScene() const
	{
		return Scene;
	}
protected:
	RTCScene Scene;
};

class BottomLevelAccelerationStructure : public AccelerationStructure
{
public:
	void AddGeometry(const std::filesystem::path& Path)
	{
		WaveFrontReader<unsigned int> Reader;
		if (SUCCEEDED(Reader.Load(Path.c_str())))
		{
			auto Geometry = rtcNewGeometry(g_RTCDevice, RTC_GEOMETRY_TYPE_TRIANGLE);

			/*
			* Create a triangle mesh geometry
			* You can look up geometry types in the API documentation to
			* find out which type expects which buffers.
			*
			* We create buffers directly on the device, but you can also use
			* shared buffers. For shared buffers, special care must be taken
			* to ensure proper alignment and padding. This is described in
			* more detail in the API documentation.
			*/

			auto pVertices = reinterpret_cast<Vertex*>(rtcSetNewGeometryBuffer(Geometry,
				RTC_BUFFER_TYPE_VERTEX,
				0,
				RTC_FORMAT_FLOAT3,
				sizeof(Vertex),
				Reader.vertices.size()));

			assert(Reader.indices.size() % 3 == 0);
			size_t NumTriangles = Reader.indices.size() / 3;
			auto pIndices = reinterpret_cast<unsigned*>(rtcSetNewGeometryBuffer(Geometry,
				RTC_BUFFER_TYPE_INDEX,
				0,
				RTC_FORMAT_UINT3,
				sizeof(unsigned int) * 3,
				NumTriangles));

			if (pVertices && pIndices)
			{
				for (auto [i, v] : enumerate(Reader.vertices))
				{
					pVertices[i] = Vertex(v.position);
				}

				for (auto [i, index] : enumerate(Reader.indices))
				{
					pIndices[i] = index;
				}
			}

			/*
			 * You must commit geometry objects when you are done setting them up,
			 * or you will not get any intersections.
			 */
			rtcCommitGeometry(Geometry);

			/*
			 * In rtcAttachGeometry(...), the scene takes ownership of the geom
			 * by increasing its reference count. This means that we don't have
			 * to hold on to the geom handle, and may release it. The geom object
			 * will be released automatically when the scene is destroyed.
			 *
			 * rtcAttachGeometry() returns a geometry ID. We could use this to
			 * identify intersected objects later on.
			 */
			rtcAttachGeometry(Scene, Geometry);
			rtcReleaseGeometry(Geometry);

			Geometries.push_back(Geometry);
		}
	}

	void Generate()
	{
		rtcCommitScene(Scene);
	}
private:
	std::vector<RTCGeometry> Geometries;
};

struct RAYTRACING_INSTANCE_DESC
{
	Transform Transform;
	BottomLevelAccelerationStructure* pBLAS;
};

class TopLevelAccelerationStructure : public AccelerationStructure
{
public:
	void AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc)
	{
		auto InstanceGeometry = rtcNewGeometry(g_RTCDevice, RTC_GEOMETRY_TYPE_INSTANCE);

		rtcSetGeometryInstancedScene(InstanceGeometry, *Desc.pBLAS);
		rtcAttachGeometry(Scene, InstanceGeometry);
		rtcReleaseGeometry(InstanceGeometry);

		/*
		* In rtcAttachGeometry(...), the scene takes ownership of the geom
		* by increasing its reference count. This means that we don't have
		* to hold on to the geom handle, and may release it. The geom object
		* will be released automatically when the scene is destroyed.
		*
		* rtcAttachGeometry() returns a geometry ID. We could use this to
		* identify intersected objects later on.
		*/
		Instances.push_back(Desc);
		InstanceGeometries.push_back(InstanceGeometry);
		NumInstances++;
	}

	void Generate()
	{
		for (size_t i = 0; i < NumInstances; ++i)
		{
			auto& Instance = Instances[i];
			auto& InstanceGeometry = InstanceGeometries[i];

			XMMATRIX mMatrix = Instance.Transform.Matrix();
			XMFLOAT3X4 Matrix;
			XMStoreFloat3x4(&Matrix, mMatrix);

			rtcSetGeometryTransform(InstanceGeometry, 0, RTC_FORMAT_FLOAT3X4_ROW_MAJOR, reinterpret_cast<float*>(&Matrix));
			rtcCommitGeometry(InstanceGeometry);
		}

		/*
		* Like geometry objects, scenes must be committed. This lets
		* Embree know that it may start building an acceleration structure.
		*/
		rtcCommitScene(Scene);
	}
private:
	size_t NumInstances = 0;
	std::vector<RAYTRACING_INSTANCE_DESC> Instances;
	std::vector<RTCGeometry> InstanceGeometries;
};

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

	BottomLevelAccelerationStructure BurrPuzzle;
	BurrPuzzle.AddGeometry(ModelFolderPath / "BurrPuzzle.obj");
	BurrPuzzle.Generate();

	TopLevelAccelerationStructure Scene;

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