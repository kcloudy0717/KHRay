#pragma once
#include "Math.h"
#include "Camera.h"
#include "AccelerationStructure.h"
#include "BxDF.h"

#include "Light/Light.h"

class Device;
struct Scene;

struct OrthonormalBasis
{
	OrthonormalBasis() = default;
	OrthonormalBasis(const Vector3f& n)
		: n(n)
	{
		CoordinateSystem(n, &s, &t);
	}

	Vector3f ToWorld(const Vector3f& v)
	{
		return s * v.x + t * v.y + n * v.z;
	}

	Vector3f ToLocal(const Vector3f& v)
	{
		return Vector3f(Dot(v, s), Dot(v, t), Dot(v, n));
	}

	// tangent, bitangent, normal
	Vector3f s;
	Vector3f t;
	Vector3f n;
};

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
	OrthonormalBasis GeometryBasis;
	OrthonormalBasis ShadingBasis;
	BSDF* BSDF;
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

	bool Intersect(const Ray& Ray, SurfaceInteraction* pSurfaceInteraction) const;

	void AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc)
	{
		TopLevelAccelerationStructure.AddBottomLevelAccelerationStructure(Desc);
	}

	void AddLight(Light* pLight)
	{
		Lights.push_back(pLight);
	}

	void Generate()
	{
		TopLevelAccelerationStructure.Generate();
	}

	Camera Camera;
	TopLevelAccelerationStructure TopLevelAccelerationStructure;
	std::vector<Light*> Lights;
};