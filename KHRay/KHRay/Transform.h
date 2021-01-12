#pragma once
#include <DirectXMath.h>

struct Transform
{
	Transform();

	void SetTransform(DirectX::FXMMATRIX M);
	void Translate(float DeltaX, float DeltaY, float DeltaZ);
	void SetScale(float ScaleX, float ScaleY, float ScaleZ);
	// Radians
	void SetOrientation(float AngleX, float AngleY, float AngleZ);
	void Rotate(float AngleX, float AngleY, float AngleZ);

	DirectX::XMMATRIX Matrix() const;

	DirectX::XMVECTOR Right() const;
	DirectX::XMVECTOR Up() const;
	DirectX::XMVECTOR Forward() const;

	bool operator==(const Transform& Transform) const;
	bool operator!=(const Transform& Transform) const;

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT4 Orientation;
};