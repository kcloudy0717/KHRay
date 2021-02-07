#pragma once
#include <memory>
#include "Math.h"
#include "Spectrum.h"
#include "Sampling.h"

struct SurfaceInteraction;

inline float CosTheta(const Vector3f& w)
{
	return w.z;
}
inline float Cos2Theta(const Vector3f& w)
{
	return w.z * w.z;
}
inline float AbsCosTheta(const Vector3f& w)
{
	return std::abs(w.z);
}

inline bool SameHemisphere(const Vector3f& v0, const Vector3f& v1)
{
	return v0.z * v1.z > 0;
}

// Interface for BRDF and BTDF
struct BxDF
{
	enum Type
	{
		BSDF_Reflection = 1 << 0,
		BSDF_Transmission = 1 << 1,

		BSDF_Diffuse = 1 << 2,
		BSDF_Glossy = 1 << 3,
		BSDF_Specular = 1 << 4,

		BSDF_All = BSDF_Reflection | BSDF_Transmission | BSDF_Diffuse | BSDF_Glossy | BSDF_Specular
	};

	BxDF(Type Type)
		: _Type(Type)
	{

	}
	virtual ~BxDF() = default;

	bool MatchesFlags(Type Type) const
	{
		return (Type & _Type) == _Type;
	}

	virtual Spectrum f(const Vector3f& wo, const Vector3f& wi) const = 0;
	virtual Spectrum Samplef(const Vector3f& wo, Vector3f* wi, const Vector2f& Xi, float* pdf) const
	{
		// Cosine-sample the hemisphere, flipping the direction if necessary
		*wi = CosineSampleHemisphere(Xi);
		if (wo.z < 0)
		{
			wi->z *= -1;
		}
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	virtual float Pdf(const Vector3f& wo, const Vector3f& wi) const
	{
		return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * g_1DIVPI : 0.0f;
	}

	Type _Type = Type(0);
};

struct LambertianReflection : BxDF
{
	LambertianReflection(const Spectrum& R)
		: BxDF(BxDF::Type(BxDF::BSDF_Reflection | BxDF::BSDF_Diffuse))
		, R(R)
	{

	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		return R * g_1DIVPI;
	}

	Spectrum R;
};

struct BSDF
{
	int NumComponents(BxDF::Type Flags = BxDF::BSDF_All) const
	{
		int Num = 0;

		for (int i = 0; i < NumBxDF; ++i)
		{
			if (BxDFs[i]->MatchesFlags(Flags))
			{
				Num++;
			}
		}

		return Num;
	}

	void SetInteraction(const SurfaceInteraction& Interaction);

	Vector3f WorldToLocal(const Vector3f& v) const
	{
		return Vector3f(Dot(v, ss), Dot(v, ts), Dot(v, Ns));
	}

	Vector3f LocalToWorld(const Vector3f& v) const
	{
		return Vector3f(ss.x * v.x + ts.x * v.y + Ns.x * v.z,
			ss.y * v.x + ts.y * v.y + Ns.y * v.z,
			ss.z * v.x + ts.z * v.y + Ns.z * v.z);
	}

	void Add(std::shared_ptr<BxDF> pBxDF)
	{
		assert(NumBxDF < MaxBxDFs);
		BxDFs[NumBxDF++] = pBxDF;
	}

	Spectrum f(const Vector3f& woW, const Vector3f& wiW, BxDF::Type Flags = BxDF::BSDF_All) const;
	Spectrum Samplef(const Vector3f& woW, Vector3f* wiW, const Vector2f& Xi, float* pdf, BxDF::Type Flags = BxDF::BSDF_All) const;

	float Pdf(const Vector3f& woW, const Vector3f& wiW, BxDF::Type Flags = BxDF::BSDF_All) const;

	Vector3f Ng, Ns;
	Vector3f ss, ts;

	int NumBxDF = 0;
	static constexpr int MaxBxDFs = 8;
	std::shared_ptr<BxDF> BxDFs[MaxBxDFs];
};