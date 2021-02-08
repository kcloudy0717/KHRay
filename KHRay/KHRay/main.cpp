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

#include <filesystem>

#include "Utility.h"
#include "Device.h"
#include "Scene.h"
#include "Sampler/Sampler.h"
#include "Sampler/Random.h"
#include "Integrator/NormalIntegrator.h"
#include "Integrator/AOIntegrator.h"
#include "Integrator/PathIntegrator.h"

int main(int argc, char** argv)
{
#if defined(_DEBUG)
	ENABLE_LEAK_DETECTION();
	SET_LEAK_BREAKPOINT(-1);
#endif
	std::filesystem::path ExecutableFolderPath = std::filesystem::path(argv[0]).parent_path();
	std::filesystem::path ModelFolderPath = ExecutableFolderPath / "Models";

	InitializeSampledSpectrums();

	Device Device;
	Scene Scene(Device);
	Scene.Camera.Transform.Translate(0, 15, 5);
	Scene.Camera.Transform.Rotate(30.0_Deg, 0, 0);

	BottomLevelAccelerationStructure BreakfastRoom(Device);
	BreakfastRoom.AddGeometry(ModelFolderPath / "breakfast_room" / "breakfast_room.obj");
	BreakfastRoom.Generate();

	RAYTRACING_INSTANCE_DESC BreakfastRoomInstance = {};
	BreakfastRoomInstance.Transform.SetScale(5, 5, 5);
	BreakfastRoomInstance.Transform.Translate(0, 0, 20);
	BreakfastRoomInstance.pBLAS = &BreakfastRoom;
	Scene.AddBottomLevelAccelerationStructure(BreakfastRoomInstance);

	PointLight PL0(Spectrum(216.0f / 255.0f, 247.0f / 255.0f, 255.0f / 255.0f) * 200.0f);
	PL0.Transform.Translate(3, 15, 20);
	Scene.AddLight(&PL0);

	int NumSamplesPerPixel = 8;
	Random Random(NumSamplesPerPixel);

	//auto Integrator = CreateNormalIntegrator(Geometric);

	//constexpr int NumSamples = 16;
	//auto Integrator = CreateAOIntegrator(NumSamples, SamplingStrategy::Uniform);

	int MaxDepth = 5;
	auto Integrator = CreatePathIntegrator(MaxDepth);

	Integrator->Initialize(Scene);
	return Integrator->Render(Scene, Random);
}