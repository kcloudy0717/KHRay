#pragma once
#include <memory>
#include <optional>
#include "Math.h"
#include "Spectrum.h"
#include "Sampling.h"

struct SurfaceInteraction;

float FrDielectric(float cosThetaI, float etaI, float etaT);
Spectrum FrConductor(float cosThetaI, const Spectrum& etaI, const Spectrum& etaT, const Spectrum& k);

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

inline Vector3f Reflect(const Vector3f& wo, const Vector3f& n)
{
	return -wo + 2.0f * Dot(wo, n) * n;
}

struct Fresnel
{
	virtual ~Fresnel() = default;

	virtual Spectrum Evaluate(float cosThetaI) const
	{
		// Default implementation returns 100% reflection for all incoming directions.
		// Although this is physically implausible, it is a convenient capability to have available.
		return Spectrum(1.0f);
	}
};

struct FresnelDielectric : Fresnel
{
	FresnelDielectric(float etaI, float etaT)
		: etaI(etaI)
		, etaT(etaT)
	{

	}

	Spectrum Evaluate(float cosThetaI) const override
	{
		return FrDielectric(cosThetaI, etaI, etaT);
	}

	float etaI, etaT;
};

struct FresnelConductor : Fresnel
{
	FresnelConductor(const Spectrum& etaI, const Spectrum& etaT, const Spectrum& k)
		: etaI(etaI)
		, etaT(etaT)
		, k(k)
	{

	}

	Spectrum Evaluate(float cosThetaI) const override
	{
		return FrConductor(cosThetaI, etaI, etaT, k);
	}

	Spectrum etaI, etaT, k;
};

enum class BxDFTypes
{
	Unknown = 0,
	Reflection = 1 << 0,
	Transmission = 1 << 1,
	All = Reflection | Transmission
};

inline BxDFTypes operator|(BxDFTypes a, BxDFTypes b) {
	return BxDFTypes((int)a | (int)b);
}

inline int operator&(BxDFTypes a, BxDFTypes b) {
	return ((int)a & (int)b);
}

inline BxDFTypes& operator|=(BxDFTypes& a, BxDFTypes b) {
	(int&)a |= int(b);
	return a;
}

enum BxDFFlags
{
	Unknown = 0,
	Reflection = 1 << 0,
	Transmission = 1 << 1,

	Diffuse = 1 << 2,
	Glossy = 1 << 3,
	Specular = 1 << 4,
	// Composite flags definitions
	DiffuseReflection = Diffuse | Reflection,
	DiffuseTransmission = Diffuse | Transmission,
	GlossyReflection = Glossy | Reflection,
	GlossyTransmission = Glossy | Transmission,
	SpecularReflection = Specular | Reflection,
	SpecularTransmission = Specular | Transmission,

	All = Diffuse | Glossy | Specular | Reflection | Transmission
};

inline BxDFFlags operator|(BxDFFlags a, BxDFFlags b)
{
	return BxDFFlags((int)a | (int)b);
}

inline int operator&(BxDFFlags a, BxDFFlags b)
{
	return ((int)a & (int)b);
}

inline int operator&(BxDFFlags a, BxDFTypes b)
{
	return ((int)a & (int)b);
}

inline BxDFFlags& operator|=(BxDFFlags& a, BxDFFlags b)
{
	(int&)a |= int(b);
	return a;
}

// BxDFFlags Inline Functions
inline bool IsReflective(BxDFFlags f)
{
	return f & BxDFFlags::Reflection;
}
inline bool IsTransmissive(BxDFFlags f)
{
	return f & BxDFFlags::Transmission;
}
inline bool IsDiffuse(BxDFFlags f)
{
	return f & BxDFFlags::Diffuse;
}
inline bool IsGlossy(BxDFFlags f)
{
	return f & BxDFFlags::Glossy;
}
inline bool IsSpecular(BxDFFlags f)
{
	return f & BxDFFlags::Specular;
}

struct BSDFSample
{
	BSDFSample() = default;
	BSDFSample(Spectrum f, Vector3f wi, float pdf, BxDFFlags flags)
		: f(f)
		, wi(wi)
		, pdf(pdf)
		, flags(flags)
	{

	}

	Spectrum f;
	Vector3f wi;
	float pdf = 0.0f;
	BxDFFlags flags;
};

// Interface for BRDF and BTDF
struct BxDF
{
	BxDF() = default;
	virtual ~BxDF() = default;

	bool MatchesFlags(BxDFFlags flags) const { return (Flags() & flags) == Flags(); }

	virtual Spectrum f(const Vector3f& wo, const Vector3f& wi) const = 0;

	virtual float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const = 0;

	virtual std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const = 0;

	virtual BxDFFlags Flags() const = 0;
};

struct LambertianReflection : BxDF
{
	LambertianReflection(const Spectrum& R)
		: R(R)
	{

	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		if (!SameHemisphere(wo, wi))
		{
			return Spectrum(0.0f);
		}

		return R * g_1DIVPI;
	}

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override
	{
		if (!(Types & BxDFTypes::Reflection) || !SameHemisphere(wo, wi))
		{
			return 0.0f;
		}

		return CosineHemispherePdf(AbsCosTheta(wi));
	}

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const override
	{
		if (!(Types & BxDFTypes::Reflection))
		{
			return {};
		}

		// Sample cosine-weighted hemisphere to compute _wi_ and _pdf_
		Vector3f wi = SampleCosineHemisphere(Xi);
		if (wo.z < 0.0f)
		{
			wi.z *= -1.0f;
		}

		return BSDFSample(f(wo, wi), wi, Pdf(wo, wi, Types), BxDFFlags::DiffuseReflection);
	}

	BxDFFlags Flags() const override
	{
		return BxDFFlags::DiffuseReflection;
	}

	Spectrum R;
};

struct Mirror : BxDF
{
	Mirror(const Spectrum& R)
		: R(R)
	{

	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		return Spectrum(0.0f);
	}

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override
	{
		return 0.0f;
	}

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const override
	{
		Vector3f wi = Vector3f(-wo.x, -wo.y, wo.z);
		float pdf = 1.0f;

		return BSDFSample(R / AbsCosTheta(wi), wi, pdf, BxDFFlags::DiffuseReflection);
	}

	BxDFFlags Flags() const override
	{
		return BxDFFlags::SpecularReflection;
	}

	Spectrum R;
};