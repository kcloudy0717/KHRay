#pragma once
#include "Vector3.h"

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
		CoordinateSystem(n, &s, &t);
	}

	Vector3f ToWorld(const Vector3f& v) const { return s * v.x + t * v.y + n * v.z; }

	Vector3f ToLocal(const Vector3f& v) const { return Vector3f(Dot(v, s), Dot(v, t), Dot(v, n)); }

	// tangent, bitangent, normal
	Vector3f s;
	Vector3f t;
	Vector3f n;
};
