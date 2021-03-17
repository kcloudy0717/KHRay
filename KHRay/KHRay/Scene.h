#pragma once
#include "Math.h"
#include "Camera.h"
#include "AccelerationStructure.h"
#include "BSDF.h"

#include "Light/Light.h"

class Device;
struct Scene;

struct Interaction
{
	Interaction() = default;
	Interaction(const Vector3f& p, const Vector3f& wo, const Vector3f& n)
		: p(p)
		, wo(wo)
		, n(n)
	{

	}

	Ray SpawnRay(const Vector3f& d) const;
	Ray SpawnRayTo(const Interaction& Interaction) const;

	Vector3f p; // Hit point
	Vector3f wo;
	Vector3f n; // Normal
};

struct SurfaceInteraction : Interaction
{
	RAYTRACING_INSTANCE_DESC Instance;
	Vector2f uv; // Texture coord
	Frame GeometryFrame;
	Frame ShadingFrame;
	BSDF BSDF;
};

struct VisibilityTester
{
	bool Unoccluded(const Scene& Scene) const;

	Interaction I0;
	Interaction I1;
};

struct Scene
{
	Scene(const Device& Device);

	std::optional<SurfaceInteraction> Intersect(const Ray& Ray) const;
	bool Occluded(const Ray& Ray) const;

	void AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc);

	void AddLight(Light* pLight);

	void Generate();

	Camera Camera;
	TopLevelAccelerationStructure TopLevelAccelerationStructure;
	std::vector<Light*> Lights;
};