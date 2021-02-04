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
#include "Integrator/PathIntegrator.h"
#include "Integrator/AOIntegrator.h"

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
	Scene.Camera.Transform.Translate(0, 5, 0);

	//BottomLevelAccelerationStructure BurrPuzzle(Device);
	//BurrPuzzle.AddGeometry(ModelFolderPath / "BurrPuzzle.obj");
	//BurrPuzzle.Generate();

	//RAYTRACING_INSTANCE_DESC BurrPuzzleInstance = {};
	//BurrPuzzleInstance.Transform.SetScale(20, 20, 20);
	//BurrPuzzleInstance.Transform.Translate(0, 5, 5);
	//BurrPuzzleInstance.Transform.Rotate(0, 30.0_Deg, 0);
	//BurrPuzzleInstance.pBLAS = &BurrPuzzle;

	//Scene.AddBottomLevelAccelerationStructure(BurrPuzzleInstance);

	BottomLevelAccelerationStructure BreakfastRoom(Device);
	BreakfastRoom.AddGeometry(ModelFolderPath / "breakfast_room" / "breakfast_room.obj");
	BreakfastRoom.Generate();

	RAYTRACING_INSTANCE_DESC BreakfastRoomInstance = {};
	BreakfastRoomInstance.Transform.SetScale(5, 5, 5);
	BreakfastRoomInstance.Transform.Translate(0, 0, 20);
	BreakfastRoomInstance.Transform.Rotate(0, 180.0_Deg, 0);
	BreakfastRoomInstance.pBLAS = &BreakfastRoom;

	Scene.AddBottomLevelAccelerationStructure(BreakfastRoomInstance);

	constexpr int NumSamplesPerPixel = 8;
	Random Random(NumSamplesPerPixel);

	//constexpr int MaxDepth = 1;
	//auto Integrator = CreatePathIntegrator(MaxDepth);

	constexpr int NumSamples = 64;
	auto Integrator = CreateAOIntegrator(NumSamples);

	return Integrator->Render(Scene, Random);
}