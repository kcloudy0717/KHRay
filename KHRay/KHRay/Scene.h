#pragma once
#include "Ray.h"
#include "Camera.h"
#include "AccelerationStructure.h"

class Device;

struct Intersection
{
	RAYTRACING_INSTANCE_DESC Instance;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TextureCoordinate;
	DirectX::XMFLOAT3 Normal;
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