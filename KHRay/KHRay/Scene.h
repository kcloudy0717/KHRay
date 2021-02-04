#pragma once
#include "Math.h"
#include "Camera.h"
#include "AccelerationStructure.h"

class Device;

// ONB: Orthonormal basis
struct ONB
{
	ONB() = default;
	ONB(const Vector3f& n)
		: n(n)
	{
		CoordinateSystem(n, &s, &t);
	}

	Vector3f ToWorld(const Vector3f& v)
	{
		return s * v.x + t * v.y + n * v.z;
	}

	// AKA tangent, bitangent, normal
	Vector3f s;
	Vector3f t;
	Vector3f n;
};

struct Intersection
{
	RAYTRACING_INSTANCE_DESC Instance;
	Vector3f p; // Hit point
	Vector2f uv; // Texture coord

	ONB geoFrame; // Geometry frame
	ONB shFrame; // Shading frame
};

struct Scene
{
	Scene(const Device& Device);

	bool Intersect(const Ray& Ray, Intersection* pIntersection) const;

	void AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc)
	{
		TopLevelAccelerationStructure.AddBottomLevelAccelerationStructure(Desc);
	}

	void Generate()
	{
		TopLevelAccelerationStructure.Generate();
	}

	Camera Camera;
	TopLevelAccelerationStructure TopLevelAccelerationStructure;
};