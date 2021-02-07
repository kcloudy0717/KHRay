#pragma once
#include <cmath>
#include <compare>
#include <algorithm>

class SampledSpectrum;
class RGBSpectrum;

template<int NumSpectrumSamples>
class CoefficientSpectrum
{
public:
	static constexpr int NumCoefficients = NumSpectrumSamples;

	CoefficientSpectrum(float v = 0.0f)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] = v;
		}
	}

	CoefficientSpectrum(const CoefficientSpectrum& s2)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] = s2.c[i];
		}
	}

	auto operator<=>(const CoefficientSpectrum&) const = default;

	float operator[](int i) const
	{
		return c[i];
	}

	float& operator[](int i)
	{
		return c[i];
	}

	CoefficientSpectrum operator-() const
	{
		CoefficientSpectrum ret;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] = -c[i];
		}
		return ret;
	}

	CoefficientSpectrum& operator+=(const CoefficientSpectrum& s2)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] += s2.c[i];
		}
		return *this;
	}

	CoefficientSpectrum operator+(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] += s2.c[i];
		}
		return ret;
	}

	CoefficientSpectrum& operator-=(const CoefficientSpectrum& s2)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] -= s2.c[i];
		}
		return *this;
	}

	CoefficientSpectrum operator-(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] -= s2.c[i];
		}
		return ret;
	}

	CoefficientSpectrum& operator*=(const CoefficientSpectrum& s2)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] *= s2.c[i];
		}
		return *this;
	}

	CoefficientSpectrum operator*(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] *= s2.c[i];
		}
		return ret;
	}

	CoefficientSpectrum& operator/=(const CoefficientSpectrum& s2)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] /= s2.c[i];
		}
		return *this;
	}

	CoefficientSpectrum operator/(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] /= s2.c[i];
		}
		return ret;
	}

	CoefficientSpectrum& operator/=(float a)
	{
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			c[i] /= a;
		}
		return *this;
	}

	CoefficientSpectrum operator/(float a) const
	{
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] /= a;
		}
		return ret;
	}

	CoefficientSpectrum Clamp(float low = 0, float high = INFINITY) const
	{
		CoefficientSpectrum ret;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] = std::clamp(c[i], low, high);
		}
		return ret;
	}

	CoefficientSpectrum Sqrt()
	{
		CoefficientSpectrum ret;
		for (int i = 0; i < NumSpectrumSamples; ++i)
		{
			ret.c[i] = sqrt(c[i]);
		}
		return ret;
	}

	float MaxComponentValue() const
	{
		float m = c[0];
		for (int i = 1; i < NumSpectrumSamples; ++i)
		{
			m = std::max(m, c[i]);
		}
		return m;
	}

	bool HasNans() const
	{
		for (const auto& l : c)
		{
			if (std::isnan(l))
			{
				return true;
			}
		}
		return false;
	}

	bool IsBlack() const
	{
		for (const auto& l : c)
		{
			if (l != 0.0f)
				return false;
		}
		return true;
	}
protected:
	float c[NumSpectrumSamples];
};

template<int NumSpectrumSamples>
inline CoefficientSpectrum<NumSpectrumSamples> operator*(float a, const CoefficientSpectrum<NumSpectrumSamples>& s)
{
	return s * a;
}

// Sampled Spectrum
static constexpr int SampledLambdaStart = 400;
static constexpr int SampledLambdaEnd = 700;
static constexpr int NumSpectralSamples = 60;

enum class SpectrumType
{
	Reflectance,
	Illuminant
};

bool IsSpectrumSamplesSorted(const float* lambda, const float* vals, int n);
void SortSpectrumSamples(float* lambda, float* vals, int n);

float AverageSpectrumSamples(const float* lambda, const float* vals, int n, float lambdaStart, float lambdaEnd);
float InterpolateSpectrumSamples(const float* lambda, const float* vals, int n, float l);

void XYZToRGB(const float xyz[3], float rgb[3]);
void RGBToXYZ(const float rgb[3], float xyz[3]);

class SampledSpectrum : public CoefficientSpectrum<NumSpectralSamples>
{
public:
	SampledSpectrum(float v = 0.0f);
	SampledSpectrum(const CoefficientSpectrum<NumSpectralSamples>& v);
	SampledSpectrum(const RGBSpectrum& s, SpectrumType Type = SpectrumType::Reflectance);

	static SampledSpectrum FromSampled(const float* lambda, const float* v, int n);
	static SampledSpectrum FromRGB(const float rgb[3], SpectrumType Type = SpectrumType::Illuminant);
	static SampledSpectrum FromXYZ(const float xyz[3], SpectrumType Type = SpectrumType::Reflectance);

	float y() const;
	void ToXYZ(float xyz[3]) const;
	void ToRGB(float rgb[3]) const;
	RGBSpectrum ToRGBSpectrum() const;
};

class RGBSpectrum : public CoefficientSpectrum<3>
{
public:
	RGBSpectrum(float v = 0.0f);
	RGBSpectrum(float r, float g, float b);
	RGBSpectrum(const CoefficientSpectrum<3>& v);
	RGBSpectrum(const RGBSpectrum& s, SpectrumType Type = SpectrumType::Reflectance);

	static RGBSpectrum FromSampled(const float* lambda, const float* v, int n);
	static RGBSpectrum FromRGB(const float rgb[3], SpectrumType Type = SpectrumType::Reflectance);
	static RGBSpectrum FromXYZ(const float xyz[3], SpectrumType Type = SpectrumType::Reflectance);

	float y() const;
	void ToXYZ(float xyz[3]) const;
	void ToRGB(float rgb[3]) const;
	RGBSpectrum ToRGBSpectrum() const;
};

inline static SampledSpectrum X, Y, Z;
inline static SampledSpectrum rgbRefl2SpectWhite, rgbRefl2SpectCyan;
inline static SampledSpectrum rgbRefl2SpectMagenta, rgbRefl2SpectYellow;
inline static SampledSpectrum rgbRefl2SpectRed, rgbRefl2SpectGreen;
inline static SampledSpectrum rgbRefl2SpectBlue;
inline static SampledSpectrum rgbIllum2SpectWhite, rgbIllum2SpectCyan;
inline static SampledSpectrum rgbIllum2SpectMagenta, rgbIllum2SpectYellow;
inline static SampledSpectrum rgbIllum2SpectRed, rgbIllum2SpectGreen;
inline static SampledSpectrum rgbIllum2SpectBlue;

void InitializeSampledSpectrums();

#ifdef SAMPLED_SPECTRUM
using Spectrum = SampledSpectrum;
#else
using Spectrum = RGBSpectrum;
#endif