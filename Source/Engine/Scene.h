#pragma once
#include "RTXDevice.h"
#include "Math/Math.h"
#include "Camera.h"
#include "AccelerationStructure.h"
#include "BSDF.h"
#include "Interaction.h"

#include "Light/Light.h"

struct Scene;

struct VisibilityTester
{
	bool	 Unoccluded(const Scene& Scene) const;
	Spectrum Tr(const Scene& Scene, Sampler& sampler) const;

	Interaction I0;
	Interaction I1;
};

struct Scene
{
	Scene(const RTXDevice& Device);

	[[nodiscard]] std::optional<SurfaceInteraction> Intersect(const RayDesc& Ray) const;
	[[nodiscard]] bool								Occluded(const RayDesc& Ray) const;
	[[nodiscard]] bool								IntersectTr(RayDesc ray, Sampler& sampler, Spectrum* OutTr);

	void AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc);

	void AddLight(Light* pLight);

	void Generate();

	Camera						  Camera;
	TopLevelAccelerationStructure TopLevelAccelerationStructure;
	std::vector<Light*>			  Lights;
};
