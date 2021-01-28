#pragma once
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TextureCoordinate;
	DirectX::XMFLOAT3 Normal;
};

inline float BarycentricInterpolation(float v0, float v1, float v2, DirectX::XMFLOAT3 barycentric)
{
	return v0 * barycentric.x + v1 * barycentric.y + v2 * barycentric.z;
}

inline DirectX::XMFLOAT2 BarycentricInterpolation(DirectX::XMFLOAT2 v0, DirectX::XMFLOAT2 v1, DirectX::XMFLOAT2 v2, DirectX::XMFLOAT3 barycentric)
{
	return
	{
		BarycentricInterpolation(v0.x, v1.x, v2.x, barycentric),
		BarycentricInterpolation(v0.y, v1.y, v2.y, barycentric)
	};
}

inline DirectX::XMFLOAT3 BarycentricInterpolation(DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2, DirectX::XMFLOAT3 barycentric)
{
	return
	{
		BarycentricInterpolation(v0.x, v1.x, v2.x, barycentric),
		BarycentricInterpolation(v0.y, v1.y, v2.y, barycentric),
		BarycentricInterpolation(v0.z, v1.z, v2.z, barycentric)
	};
}

inline DirectX::XMFLOAT4 BarycentricInterpolation(DirectX::XMFLOAT4 v0, DirectX::XMFLOAT4 v1, DirectX::XMFLOAT4 v2, DirectX::XMFLOAT3 barycentric)
{
	return
	{
		BarycentricInterpolation(v0.x, v1.x, v2.x, barycentric),
		BarycentricInterpolation(v0.y, v1.y, v2.y, barycentric),
		BarycentricInterpolation(v0.z, v1.z, v2.z, barycentric),
		BarycentricInterpolation(v0.w, v1.w, v2.w, barycentric)
	};
}

inline Vertex BarycentricInterpolation(Vertex v0, Vertex v1, Vertex v2, DirectX::XMFLOAT3 barycentric)
{
	Vertex vertex;
	vertex.Position = BarycentricInterpolation(v0.Position, v1.Position, v2.Position, barycentric);
	vertex.TextureCoordinate = BarycentricInterpolation(v0.TextureCoordinate, v1.TextureCoordinate, v2.TextureCoordinate, barycentric);
	vertex.Normal = BarycentricInterpolation(v0.Normal, v1.Normal, v2.Normal, barycentric);

	return vertex;
}