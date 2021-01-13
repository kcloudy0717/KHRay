#pragma once
#include <DirectXMath.h>

struct Vertex
{
	Vertex() = default;
	Vertex(const DirectX::XMFLOAT3& Position)
		: Position(Position)
	{

	}

	DirectX::XMFLOAT3 Position;
};