#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>

struct Ray
{
	Ray() = default;
	Ray(const DirectX::XMFLOAT3& Origin, const DirectX::XMFLOAT3& Direction, float Time);

	DirectX::XMVECTOR XM_CALLCONV At(float T);

	DirectX::XMFLOAT3 Origin;
	DirectX::XMFLOAT3 Direction;
	float Time;
};