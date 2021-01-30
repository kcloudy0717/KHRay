#pragma once
#include "Transform.h"
#include "Ray.h"

class Sampler;

struct Camera
{
	Ray GetRay(float U, float V) const;

	void SetLookAt(DirectX::FXMVECTOR EyePosition, DirectX::FXMVECTOR FocusPosition, DirectX::FXMVECTOR UpDirection);

	void SetPosition(float x, float y, float z);

	Transform Transform;
	float VerticalFOV;
	float AspectRatio;
	float Aperture;
	float FocalLength = 1;
	// shutter open/close times
	float Time0 = 0;
	float Time1 = 0;
};