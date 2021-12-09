#pragma once
#include "TVector3.h"

struct Frame
{
	Frame() = default;
	Frame(Vector3f s, Vector3f t, Vector3f n)
		: s(s)
		, t(t)
		, n(n)
	{
	}

	Frame(const Vector3f& n)
		: n(n)
	{
		coordinatesystem(n, &s, &t);
	}

	Vector3f ToWorld(const Vector3f& v) const { return s * v.x + t * v.y + n * v.z; }

	Vector3f ToLocal(const Vector3f& v) const { return Vector3f(dot(v, s), dot(v, t), dot(v, n)); }

	// tangent, bitangent, normal
	Vector3f s;
	Vector3f t;
	Vector3f n;
};
