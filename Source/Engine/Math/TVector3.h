#pragma once
#include <cmath>
#include <compare>
#include <DirectXMath.h>

template<typename T>
struct TVector3
{
	TVector3() { x = y = z = static_cast<T>(0); }

	TVector3(T v)
		: x(v)
		, y(v)
		, z(v)
	{
	}

	TVector3(T x, T y, T z)
		: x(x)
		, y(y)
		, z(z)
	{
	}

	auto operator<=>(const TVector3&) const = default;

	T operator[](int i) const { return e[i]; }

	T& operator[](int i) { return e[i]; }

	TVector3 operator-() const { return TVector3(-x, -y, -z); }

	TVector3 operator+(const TVector3& v) const { return TVector3(x + v.x, y + v.y, z + v.z); }

	TVector3& operator+=(const TVector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	TVector3 operator-(const TVector3& v) const { return TVector3(x - v.x, y - v.y, z - v.z); }

	TVector3& operator-=(const TVector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	TVector3 operator*(T s) const { return TVector3(x * s, y * s, z * s); }

	TVector3& operator*=(T s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	TVector3 operator/(T s) const
	{
		float inv = static_cast<T>(1) / s;
		return TVector3<T>(x * inv, y * inv, z * inv);
	}

	TVector3& operator/=(T s)
	{
		float inv = static_cast<T>(1) / s;
		x *= inv;
		y *= inv;
		z *= inv;
		return *this;
	}

	T LengthSquared() const { return x * x + y * y + z * z; }

	T Length() const { return std::sqrt(LengthSquared()); }

	bool HasNans() const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }

	DirectX::XMVECTOR ToXMVECTOR(bool Homogeneous = false) const
	{
		float w = Homogeneous ? 1.0f : 0.0f;
		return DirectX::XMVectorSet(x, y, z, w);
	}

	void operator=(DirectX::FXMVECTOR v) { XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), v); }

	union
	{
		T e[3];
		struct
		{
			T x, y, z;
		};
	};
};

template<typename T>
[[nodiscard]] TVector3<T> operator*(T s, const TVector3<T>& v)
{
	return v * s;
}

template<typename T>
[[nodiscard]] TVector3<T> abs(const TVector3<T>& v)
{
	return TVector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template<typename T>
[[nodiscard]] T dot(const TVector3<T>& v1, const TVector3<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template<typename T>
[[nodiscard]] T absdot(const TVector3<T>& v1, const TVector3<T>& v2)
{
	return std::abs(dot(v1, v2));
}

template<typename T>
[[nodiscard]] TVector3<T> cross(const TVector3<T>& v1, const TVector3<T>& v2)
{
	float v1x = v1.x, v1y = v1.y, v1z = v1.z;
	float v2x = v2.x, v2y = v2.y, v2z = v2.z;
	return TVector3<T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z), (v1x * v2y) - (v1y * v2x));
}

template<typename T>
[[nodiscard]] TVector3<T> normalize(const TVector3<T>& v)
{
	return v / v.Length();
}

template<typename T>
[[nodiscard]] TVector3<T> faceforward(const TVector3<T>& n, const TVector3<T>& v)
{
	return (dot(n, v) < 0.0f) ? -n : n;
}

template<typename T>
[[nodiscard]] float distance(const TVector3<T>& p1, const TVector3<T>& p2)
{
	return (p1 - p2).Length();
}

template<typename T>
[[nodiscard]] float distancesquared(const TVector3<T>& p1, const TVector3<T>& p2)
{
	return (p1 - p2).LengthSquared();
}

template<typename T>
void coordinatesystem(const TVector3<T>& v1, TVector3<T>* v2, TVector3<T>* v3)
{
	float sign = std::copysign(1.0f, v1.z);
	float a	   = -1.0f / (sign + v1.z);
	float b	   = v1.x * v1.y * a;
	*v2		   = TVector3<T>(1 + sign * v1.x * v1.x * a, sign * b, -sign * v1.x);
	*v3		   = TVector3<T>(b, sign + v1.y * v1.y * a, -v1.y);
}

using Vector3i = TVector3<int>;
using Vector3f = TVector3<float>;

inline Vector3f sphericaldirection(float sinTheta, float cosTheta, float phi)
{
	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}

inline Vector3f sphericaldirection(float sinTheta, float cosTheta, float phi, Vector3f x, Vector3f y, Vector3f z)
{
	return sinTheta * std::cos(phi) * x + sinTheta * std::sin(phi) * y + cosTheta * z;
}
