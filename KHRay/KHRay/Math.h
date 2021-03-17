#pragma once
#include <cstdint>

static constexpr float g_PI = 3.141592654f;
static constexpr float g_2PI = 6.283185307f;
static constexpr float g_1DIVPI = 0.318309886f;
static constexpr float g_1DIV2PI = 0.159154943f;
static constexpr float g_PIDIV2 = 1.570796327f;
static constexpr float g_PIDIV4 = 0.785398163f;

template <typename T>
inline bool IsPowerOf2(T v)
{
	return v && !(v & (v - 1));
}

inline int32_t RoundUpPow2(int32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

inline int64_t RoundUpPow2(int64_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	return v + 1;
}

inline int Log2Int(uint32_t v)
{
	unsigned long lz = 0;
	if (_BitScanReverse(&lz, v))
	{
		return lz;
	}
	return 0;
}

inline int Log2Int(int32_t v)
{
	return Log2Int((uint32_t)v);
}

#include "Vector2.h"
#include "Vector3.h"
#include "Transform.h"

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

	Vector3f ToWorld(const Vector3f& v) const
	{
		return s * v.x + t * v.y + n * v.z;
	}

	Vector3f ToLocal(const Vector3f& v) const
	{
		return Vector3f(Dot(v, s), Dot(v, t), Dot(v, n));
	}

	// tangent, bitangent, normal
	Vector3f s;
	Vector3f t;
	Vector3f n;
};