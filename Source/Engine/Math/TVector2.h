#pragma once
#include <cmath>
#include <compare>
#include <DirectXMath.h>

template<typename T>
struct TVector2
{
	TVector2() { x = y = static_cast<T>(0); }

	TVector2(T v)
		: x(v)
		, y(v)
	{
	}

	TVector2(T x, T y)
		: x(x)
		, y(y)
	{
	}

	auto operator<=>(const TVector2&) const = default;

	T operator[](int i) const { return e[i]; }

	T& operator[](int i) { return e[i]; }

	TVector2 operator-() const { return TVector2(-x, -y); }

	TVector2 operator+(const TVector2& v) const { return TVector2(x + v.x, y + v.y); }

	TVector2& operator+=(const TVector2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	TVector2 operator-(const TVector2& v) const { return TVector2(x - v.x, y - v.y); }

	TVector2& operator-=(const TVector2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	TVector2 operator*(T s) const { return TVector2(x * s, y * s); }

	TVector2& operator*=(T s)
	{
		x *= s;
		y *= s;
		return *this;
	}

	TVector2 operator/(T s) const
	{
		float inv = static_cast<T>(1) / s;
		return TVector2<T>(x * inv, y * inv);
	}

	TVector2& operator/=(T s)
	{
		float inv = static_cast<T>(1) / s;
		x *= inv;
		y *= inv;
		return *this;
	}

	T LengthSquared() const { return x * x + y * y; }

	T Length() const { return std::sqrt(LengthSquared()); }

	bool HasNans() const { return std::isnan(x) || std::isnan(y); }

	DirectX::XMVECTOR ToXMVECTOR() const { return DirectX::XMVectorSet(x, y, 0.0f, 0.0f); }

	void operator=(DirectX::FXMVECTOR v) { XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(this), v); }

	union
	{
		T e[2];
		struct
		{
			T x, y;
		};
	};
};

template<typename T>
TVector2<T> operator*(T s, const TVector2<T>& v)
{
	return v * s;
}

template<typename T>
TVector2<T> abs(const TVector2<T>& v)
{
	return TVector2<T>(std::abs(v.x), std::abs(v.y));
}

template<typename T>
T dot(const TVector2<T>& v1, const TVector2<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

template<typename T>
T absdot(const TVector2<T>& v1, const TVector2<T>& v2)
{
	return std::abs(Dot(v1, v2));
}

template<typename T>
TVector2<T> normalize(const TVector2<T>& v)
{
	return v / v.Length();
}

using Vector2i = TVector2<int>;
using Vector2f = TVector2<float>;
