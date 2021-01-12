#pragma once
#include <DirectXMath.h>
#include "Ray.h"

struct Vertex
{
	Vertex(const DirectX::XMFLOAT3& Position)
		: Position(Position)
	{

	}

	DirectX::XMFLOAT3 Position;
};

struct Triangle
{
	Triangle(Vertex V0, Vertex V1, Vertex V2, const DirectX::XMFLOAT3& Color)
		: V0(V0), V1(V1), V2(V2), Color(Color)
	{
	}

	Vertex V0;
	Vertex V1;
	Vertex V2;
	DirectX::XMFLOAT3 Color;
};

bool XM_CALLCONV RayIntersectsTriangle(const Ray& Ray, const Triangle& Triangle, DirectX::XMFLOAT3& Barycentrics, float& t)
{
	using namespace DirectX;

	constexpr float Epsilon = 1e-8;
	XMVECTOR vEpsilon = XMVectorSet(Epsilon, Epsilon, Epsilon, Epsilon);

	XMVECTOR RayOrigin = XMLoadFloat3(&Ray.Origin);
	XMVECTOR RayDirection = XMLoadFloat3(&Ray.Direction);

	XMVECTOR vV0, vV1, vV2;
	vV0 = XMLoadFloat3(&Triangle.V0.Position);
	vV1 = XMLoadFloat3(&Triangle.V1.Position);
	vV2 = XMLoadFloat3(&Triangle.V2.Position);

	auto edge1 = vV1 - vV0;
	auto edge2 = vV2 - vV0;

	// Face normal
	auto n = XMVector3Cross(edge1, edge2);

	auto q = XMVector3Cross(RayDirection, edge2);
	auto a = XMVector3Dot(edge1, q);

	// Backfacing or nearly parallel?
	if (XMVector3GreaterOrEqual(XMVector3Dot(n, RayDirection), g_XMZero) || XMVector3LessOrEqual(a, vEpsilon))
	{
		return false;
	}

	auto s = (RayOrigin - vV0) / a;
	auto r = XMVector3Cross(s, edge1);

	float bX = XMVectorGetX(XMVector3Dot(s, q));
	float bY = XMVectorGetX(XMVector3Dot(r, RayDirection));
	float bZ = 1 - bX - bY;

	Barycentrics = { bX, bY, bZ };

	// Intersected outside triangle?
	if ((bX < 0.0f) || (bY < 0.0f) || (bZ < 0.0f))
	{
		return false;
	}

	t = XMVectorGetX(XMVector3Dot(edge2, r));

	return t >= 0.0f;
}