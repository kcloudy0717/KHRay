#pragma once
#include "Math.h"

struct Vertex
{
	Vector3f Position;
	Vector2f TextureCoordinate;
	Vector3f Normal;

	void TransformToWorld(DirectX::XMMATRIX M)
	{
		Position = XMVector3TransformCoord(Position.ToXMVECTOR(true), M);
		Normal = XMVector3TransformNormal(Normal.ToXMVECTOR(), M);
	}
};

inline float BarycentricInterpolation(float v0, float v1, float v2, Vector3f barycentric)
{
	return v0 * barycentric.x + v1 * barycentric.y + v2 * barycentric.z;
}

inline Vector2f BarycentricInterpolation(Vector2f v0, Vector2f v1, Vector2f v2, Vector3f barycentric)
{
	return
	{
		BarycentricInterpolation(v0.x, v1.x, v2.x, barycentric),
		BarycentricInterpolation(v0.y, v1.y, v2.y, barycentric)
	};
}

inline Vector3f BarycentricInterpolation(Vector3f v0, Vector3f v1, Vector3f v2, Vector3f barycentric)
{
	return
	{
		BarycentricInterpolation(v0.x, v1.x, v2.x, barycentric),
		BarycentricInterpolation(v0.y, v1.y, v2.y, barycentric),
		BarycentricInterpolation(v0.z, v1.z, v2.z, barycentric)
	};
}

inline Vertex BarycentricInterpolation(Vertex v0, Vertex v1, Vertex v2, Vector3f barycentric)
{
	Vertex vertex;
	vertex.Position = BarycentricInterpolation(v0.Position, v1.Position, v2.Position, barycentric);
	vertex.TextureCoordinate = BarycentricInterpolation(v0.TextureCoordinate, v1.TextureCoordinate, v2.TextureCoordinate, barycentric);
	vertex.Normal = BarycentricInterpolation(v0.Normal, v1.Normal, v2.Normal, barycentric);

	return vertex;
}