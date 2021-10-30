// main.cpp : Defines the entry point for the application.
//
#if defined(_DEBUG)
// memory leak
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#define ENABLE_LEAK_DETECTION() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#define SET_LEAK_BREAKPOINT(x)	_CrtSetBreakAlloc(x)
#else
#define ENABLE_LEAK_DETECTION()
#define SET_LEAK_BREAKPOINT(X)
#endif

#include "RTXDevice.h"
#include "Scene.h"

#include "Material/Disney.h"

#include "Sampler/Sampler.h"
#include "Sampler/Random.h"
#include "Sampler/Sobol.h"

#include "Integrator/NormalIntegrator.h"
#include "Integrator/AOIntegrator.h"
#include "Integrator/PathIntegrator.h"

int main(int argc, char** argv)
{
	ENABLE_LEAK_DETECTION();
	SET_LEAK_BREAKPOINT(-1);

	std::filesystem::path ExecutableFolderPath = std::filesystem::path(argv[0]).parent_path();
	std::filesystem::path ModelFolderPath	   = ExecutableFolderPath / "Assets/Models";

	InitializeSampledSpectrums();

	RTXDevice Device;
	Scene	  Scene(Device);
	Scene.Camera.Transform.Translate(0, 15, 5);
	Scene.Camera.Transform.Rotate(DirectX::XMConvertToRadians(30.0f), 0, 0);

	BottomLevelAccelerationStructure BreakfastRoom(Device);
	BreakfastRoom.AddGeometry(ModelFolderPath / "breakfast_room" / "breakfast_room.obj");
	BreakfastRoom.Generate();

	auto& leftLamp	= BreakfastRoom[2];
	auto& rightLamp = BreakfastRoom[0];
	auto& teapot	= BreakfastRoom[16];

	std::shared_ptr<Disney> disney = std::make_shared<Disney>();
	std::shared_ptr<Mirror> mirror = std::make_shared<Mirror>(Spectrum(0.9f));

	// leftLamp.BSDF.SetBxDF(disney);
	rightLamp.BSDF.SetBxDF(disney);
	teapot.BSDF.SetBxDF(mirror);

	RAYTRACING_INSTANCE_DESC BreakfastRoomInstance = {};
	BreakfastRoomInstance.Transform.SetScale(5, 5, 5);
	BreakfastRoomInstance.Transform.Translate(0, 0, 20);
	BreakfastRoomInstance.pBLAS = &BreakfastRoom;
	Scene.AddBottomLevelAccelerationStructure(BreakfastRoomInstance);

	PointLight PL0(Spectrum(216.0f / 255.0f, 247.0f / 255.0f, 255.0f / 255.0f) * 200.0f);
	PL0.Transform.Translate(3, 15, 20);
	Scene.AddLight(&PL0);

	int NumSamplesPerPixel = 16;

	// Random Random(NumSamplesPerPixel);
	Sobol Sobol(NumSamplesPerPixel, Integrator::Width, Integrator::Height);

	// auto Integrator = CreateNormalIntegrator(Shading);

	// constexpr int NumSamples = 16;
	// auto Integrator = CreateAOIntegrator(NumSamples, SamplingStrategy::Cosine);

	int	 MaxDepth	= 5;
	auto Integrator = CreatePathIntegrator(MaxDepth);

	Integrator->Initialize(Scene);
	return Integrator->Render(Scene, Sobol);
}
