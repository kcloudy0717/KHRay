#pragma once
#include <cmath>
#include <compare>
#include <DirectXMath.h>

template <typename T>
struct Vector3
{
	Vector3()
	{
		x = y = z = static_cast<T>(0);
	}

	Vector3(T v)
		: x(v), y(v), z(v)
	{

	}

	Vector3(T x, T y, T z)
		: x(x), y(y), z(z)
	{

	}

	auto operator<=>(const Vector3&) const = default;

	T operator[](int i) const
	{
		return e[i];
	}

	T& operator[](int i)
	{
		return e[i];
	}

	Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 operator+(const Vector3& v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	Vector3 operator-(const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	Vector3 operator*(T s) const
	{
		return Vector3(x * s, y * s, z * s);
	}

	Vector3& operator*=(T s)
	{
		x *= s; y *= s; z *= s;
		return *this;
	}

	Vector3 operator/(T s) const
	{
		float inv = static_cast<T>(1) / s;
		return Vector3<T>(x * inv, y * inv, z * inv);
	}

	Vector3& operator/=(T s)
	{
		float inv = static_cast<T>(1) / s;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}

	T LengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	T Length() const
	{
		return std::sqrt(LengthSquared());
	}

	bool HasNans() const
	{
		return std::isnan(x) || std::isnan(y) || std::isnan(z);
	}

	DirectX::XMVECTOR ToXMVECTOR(bool Homogeneous = false) const
	{
		float w = Homogeneous ? 1.0f : 0.0f;
		return DirectX::XMVectorSet(x, y, z, w);
	}

	void operator=(DirectX::FXMVECTOR v)
	{
		XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), v);
	}

	union
	{
		T e[3];
		struct
		{
			T x, y, z;
		};
	};
};

template <typename T>
[[nodiscard]] inline Vector3<T> operator*(T s, const Vector3<T>& v)
{
	return v * s;
}

template <typename T>
[[nodiscard]] inline Vector3<T> Abs(const Vector3<T>& v)
{
	return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template <typename T>
[[nodiscard]] inline T Dot(const Vector3<T>& v1, const Vector3<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
[[nodiscard]] inline T AbsDot(const Vector3<T>& v1, const Vector3<T>& v2)
{
	return std::abs(Dot(v1, v2));
}

template <typename T>
[[nodiscard]] inline Vector3<T> Cross(const Vector3<T>& v1, const Vector3<T>& v2)
{
	float v1x = v1.x, v1y = v1.y, v1z = v1.z;
	float v2x = v2.x, v2y = v2.y, v2z = v2.z;
	return Vector3<T>((v1y * v2z) - (v1z * v2y),
		(v1z * v2x) - (v1x * v2z),
		(v1x * v2y) - (v1y * v2x));
}

template <typename T>
[[nodiscard]] inline Vector3<T> Normalize(const Vector3<T>& v)
{
	return v / v.Length();
}

template <typename T>
[[nodiscard]] inline Vector3<T> Faceforward(const Vector3<T>& n, const Vector3<T>& v)
{
	return (Dot(n, v) < 0.0f) ? -n : n;
}

template <typename T>
[[nodiscard]] inline float DistanceSquared(const Vector3<T>& p1, const Vector3<T>& p2)
{
	return (p1 - p2).LengthSquared();
}

template <typename T>
inline void CoordinateSystem(const Vector3<T>& v1, Vector3<T>* v2, Vector3<T>* v3)
{
	if (std::abs(v1.x) > std::abs(v1.y))
	{
		*v2 = Vector3<T>(-v1.z, 0.0f, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
	}
	else
	{
		*v2 = Vector3<T>(0.0f, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
	}
	*v3 = Normalize(Cross(v1, *v2));
}

using Vector3f = Vector3<float>;
using CVector3fRef = const Vector3f&;