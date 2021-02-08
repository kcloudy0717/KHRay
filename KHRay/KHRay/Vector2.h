#pragma once
#include <cmath>
#include <compare>
#include <DirectXMath.h>

template <typename T>
struct Vector2
{
	Vector2()
	{
		x = y = static_cast<T>(0);
	}

	Vector2(T v)
		: x(v), y(v)
	{

	}

	Vector2(T x, T y)
		: x(x), y(y)
	{

	}

	auto operator<=>(const Vector2&) const = default;

	T operator[](int i) const
	{
		return e[i];
	}

	T& operator[](int i)
	{
		return e[i];
	}

	Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}

	Vector2 operator+(const Vector2& v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2& operator+=(const Vector2& v)
	{
		x += v.x; y += v.y;
		return *this;
	}

	Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	Vector2& operator-=(const Vector2& v)
	{
		x -= v.x; y -= v.y;
		return *this;
	}

	Vector2 operator*(T s) const
	{
		return Vector2(x * s, y * s);
	}

	Vector2& operator*=(T s)
	{
		x *= s; y *= s;
		return *this;
	}

	Vector2 operator/(T s) const
	{
		float inv = static_cast<T>(1) / s;
		return Vector2<T>(x * inv, y * inv);
	}

	Vector2& operator/=(T s)
	{
		float inv = static_cast<T>(1) / s;
		x *= inv; y *= inv;
		return *this;
	}

	T LengthSquared() const
	{
		return x * x + y * y;
	}

	T Length() const
	{
		return std::sqrt(LengthSquared());
	}

	bool HasNans() const
	{
		return std::isnan(x) || std::isnan(y);
	}

	DirectX::XMVECTOR ToXMVECTOR() const
	{
		return DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
	}

	void operator=(DirectX::FXMVECTOR v)
	{
		XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(this), v);
	}

	union
	{
		T e[2];
		struct
		{
			T x, y;
		};
	};
};

template <typename T>
inline Vector2<T> operator*(T s, const Vector2<T>& v)
{
	return v * s;
}

template <typename T>
Vector2<T> Abs(const Vector2<T>& v)
{
	return Vector2<T>(std::abs(v.x), std::abs(v.y));
}

template <typename T>
T Dot(const Vector2<T>& v1, const Vector2<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

template <typename T>
T AbsDot(const Vector2<T>& v1, const Vector2<T>& v2)
{
	return std::abs(Dot(v1, v2));
}

template <typename T>
Vector2<T> Normalize(const Vector2<T>& v)
{
	return v / v.Length();
}

using Vector2f = Vector2<float>;
using CVector2fRef = const Vector2f&;