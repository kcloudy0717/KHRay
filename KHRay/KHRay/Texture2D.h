#pragma once
#include <memory>
#include <windows.h>
#include <DirectXMath.h>

inline int Flatten2DTo1D(int x, int y, int width)
{
	return y * width + x;
}

template<typename T>
struct Texture2D
{
	Texture2D(UINT Width, UINT Height)
		: Resolution(Width, Height), NumPixels(Width* Height)
	{
		Pixels = std::make_unique<T[]>(NumPixels);
	}

	operator auto()
	{
		return &Pixels[0];
	}

	Texture2D& operator=(const Texture2D& rhs)
	{
		assert(NumPixels == rhs.NumPixels);
		if (this != &rhs)
		{
			memcpy(Pixels.get(), rhs.Pixels.get(), NumPixels * sizeof(T));
		}

		return *this;
	}

	void Clear(T Value = (T)0)
	{
		for (UINT64 i = 0; i < NumPixels; ++i)
		{
			Pixels[i] = Value;
		}
	}

	bool IsWithinBounds(UINT X, UINT Y) const
	{
		return (X >= 0 && X < Resolution.x) && (Y >= 0 && Y < Resolution.y);
	}

	void SetPixel(UINT X, UINT Y, T Color)
	{
		if (IsWithinBounds(X, Y))
		{
			Pixels[Y * Width + X] = Color;
		}
	}

	T GetPixel(UINT X, UINT Y) const
	{
		if (IsWithinBounds(X, Y))
		{
			return Pixels[Y * Width + X];
		}
		return 0;
	}

	std::unique_ptr<T[]> Pixels;
	union
	{
		DirectX::XMUINT2 Resolution;
		struct
		{
			UINT Width, Height;
		};
	};
	UINT64 NumPixels;
};