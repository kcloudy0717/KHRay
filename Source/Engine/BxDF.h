#pragma once
#include <optional>
#include "Math/Math.h"
#include "Spectrum.h"
#include "Sampling.h"

struct SurfaceInteraction;

float	 FrDielectric(float cosThetaI, float etaI, float etaT);
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
inline float Sin2Theta(const Vector3f& w)
{
	return std::max(0.0f, 1.0f - Cos2Theta(w));
}
inline float SinTheta(const Vector3f& w)
{
	return std::sqrt(Sin2Theta(w));
}
inline float TanTheta(const Vector3f& w)
{
	return SinTheta(w) / CosTheta(w);
}
inline float Tan2Theta(const Vector3f& w)
{
	return Sin2Theta(w) / Cos2Theta(w);
}
inline float CosPhi(const Vector3f& w)
{
	float sinTheta = SinTheta(w);
	return (sinTheta == 0.0f) ? 1.0f : std::clamp(w.x / sinTheta, -1.0f, 1.0f);
}
inline float SinPhi(const Vector3f& w)
{
	float sinTheta = SinTheta(w);
	return (sinTheta == 0.0f) ? 0.0f : std::clamp(w.y / sinTheta, -1.0f, 1.0f);
}
inline float Cos2Phi(const Vector3f& w)
{
	return CosPhi(w) * CosPhi(w);
}
inline float Sin2Phi(const Vector3f& w)
{
	return SinPhi(w) * SinPhi(w);
}

inline bool SameHemisphere(const Vector3f& v0, const Vector3f& v1)
{
	return v0.z * v1.z > 0;
}

inline Vector3f Reflect(const Vector3f& wo, const Vector3f& n)
{
	return -wo + 2.0f * dot(wo, n) * n;
}

inline bool Refract(const Vector3f& wi, const Vector3f& n, float eta, Vector3f* wt)
{
	// Compute $\cos \theta_\roman{t}$ using Snell's law
	float cosThetaI	 = dot(n, wi);
	float sin2ThetaI = std::max(float(0), float(1.0f - cosThetaI * cosThetaI));
	float sin2ThetaT = eta * eta * sin2ThetaI;

	// Handle total internal reflection for transmission
	if (sin2ThetaT >= 1)
		return false;
	float cosThetaT = std::sqrt(1 - sin2ThetaT);
	*wt				= eta * -wi + (eta * cosThetaI - cosThetaT) * n;
	return true;
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

	Spectrum Evaluate(float cosThetaI) const override { return FrDielectric(cosThetaI, etaI, etaT); }

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

	Spectrum Evaluate(float cosThetaI) const override { return FrConductor(cosThetaI, etaI, etaT, k); }

	Spectrum etaI, etaT, k;
};

class MicrofacetDistribution
{
public:
	virtual ~MicrofacetDistribution() = default;

	virtual float D(const Vector3f& wh) const	  = 0;
	virtual float Lambda(const Vector3f& w) const = 0;

	float Pdf(const Vector3f& wo, const Vector3f& wh) const;

	virtual Vector3f Sample_wh(const Vector3f& wo, const Vector2f& Xi) const = 0;

	float G1(const Vector3f& w) const { return 1.0f / (1.0f + Lambda(w)); }

	virtual float G(const Vector3f& wo, const Vector3f& wi) const { return 1.0f / (1.0f + Lambda(wo) + Lambda(wi)); }

protected:
	MicrofacetDistribution(bool sampleVisibleArea)
		: sampleVisibleArea(sampleVisibleArea)
	{
	}

	const bool sampleVisibleArea;
};

class TrowbridgeReitzDistribution : public MicrofacetDistribution
{
public:
	TrowbridgeReitzDistribution(float alphax, float alphay, bool samplevis = true)
		: MicrofacetDistribution(samplevis)
		, alphax(std::max(float(0.001f), alphax))
		, alphay(std::max(float(0.001f), alphay))
	{
	}

	static inline float RoughnessToAlpha(float roughness)
	{
		roughness = std::max(roughness, (float)1e-3);
		float x	  = std::log(roughness);
		return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
	}

	float D(const Vector3f& wh) const override;

	Vector3f Sample_wh(const Vector3f& wo, const Vector2f& Xi) const override;

private:
	float Lambda(const Vector3f& w) const override;

private:
	const float alphax, alphay;
};

enum class BxDFTypes
{
	Unknown		 = 0,
	Reflection	 = 1 << 0,
	Transmission = 1 << 1,
	All			 = Reflection | Transmission
};

inline BxDFTypes operator|(BxDFTypes a, BxDFTypes b)
{
	return BxDFTypes((int)a | (int)b);
}

inline int operator&(BxDFTypes a, BxDFTypes b)
{
	return ((int)a & (int)b);
}

inline BxDFTypes& operator|=(BxDFTypes& a, BxDFTypes b)
{
	(int&)a |= int(b);
	return a;
}

enum BxDFFlags
{
	Unknown		 = 0,
	Reflection	 = 1 << 0,
	Transmission = 1 << 1,

	Diffuse	 = 1 << 2,
	Glossy	 = 1 << 3,
	Specular = 1 << 4,
	// Composite flags definitions
	DiffuseReflection	 = Diffuse | Reflection,
	DiffuseTransmission	 = Diffuse | Transmission,
	GlossyReflection	 = Glossy | Reflection,
	GlossyTransmission	 = Glossy | Transmission,
	SpecularReflection	 = Specular | Reflection,
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

	Spectrum  f;
	Vector3f  wi;
	float	  pdf = 0.0f;
	BxDFFlags flags;
};

// Interface for BRDF and BTDF
struct BxDF
{
	BxDF()			= default;
	virtual ~BxDF() = default;

	bool MatchesFlags(BxDFFlags flags) const { return (Flags() & flags) == Flags(); }

	virtual Spectrum f(const Vector3f& wo, const Vector3f& wi) const = 0;

	virtual float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const = 0;

	virtual std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All)
		const = 0;

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

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All)
		const override
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

		return BSDFSample(f(wo, wi), wi, Pdf(wo, wi, Types), Flags());
	}

	BxDFFlags Flags() const override { return BxDFFlags::DiffuseReflection; }

	Spectrum R;
};

struct Mirror : BxDF
{
	Mirror(const Spectrum& R)
		: R(R)
	{
	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override { return Spectrum(0.0f); }

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override { return 0.0f; }

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All)
		const override
	{
		Vector3f wi	 = Vector3f(-wo.x, -wo.y, wo.z);
		float	 pdf = 1.0f;

		return BSDFSample(R / AbsCosTheta(wi), wi, pdf, Flags());
	}

	BxDFFlags Flags() const override { return BxDFFlags::SpecularReflection; }

	Spectrum R;
};

struct MicrofacetReflection : BxDF
{
	MicrofacetReflection(const Spectrum& R, MicrofacetDistribution* distribution, Fresnel* fresnel)
		: R(R)
		, distribution(distribution)
		, fresnel(fresnel)
	{
	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		float	 cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
		Vector3f wh = wi + wo;
		//<< Handle degenerate cases for microfacet reflection >>
		if (cosThetaI == 0 || cosThetaO == 0)
			return Spectrum(0.);
		if (wh.x == 0.0f && wh.y == 0.0f && wh.z == 0.0f)
			return Spectrum(0.);
		wh		   = normalize(wh);
		Spectrum F = fresnel->Evaluate(dot(wi, wh));
		return R * distribution->D(wh) * F * distribution->G(wo, wi) / (4.0f * cosThetaI * cosThetaO);
	}

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override
	{
		if (!SameHemisphere(wo, wi))
		{
			return 0.0f;
		}
		Vector3f wh = normalize(wo + wi);
		return distribution->Pdf(wo, wh) / (4.0f * dot(wo, wh));
	}

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All)
		const override
	{
		// Sample microfacet orientation $\wh$ and reflected direction $\wi$
		if (wo.z == 0)
		{
			return {};
		}

		Vector3f wh = distribution->Sample_wh(wo, Xi);
		if (dot(wo, wh) < 0) // Should be rare
		{
			return {};
		}

		Vector3f wi = Reflect(wo, wh);
		if (!SameHemisphere(wo, wi))
		{
			return {};
		}

		// Compute PDF of _wi_ for microfacet reflection
		float pdf = distribution->Pdf(wo, wh) / (4.0f * dot(wo, wh));

		return BSDFSample(f(wo, wi), wi, pdf, Flags());
	}

	BxDFFlags Flags() const override { return BxDFFlags::GlossyReflection; }

	Spectrum				R;
	MicrofacetDistribution* distribution;
	Fresnel*				fresnel;
};

struct MicrofacetTransmission : BxDF
{
	MicrofacetTransmission(const Spectrum& T, float etaA, float etaB, MicrofacetDistribution* distribution)
		: T(T)
		, etaA(etaA)
		, etaB(etaB)
		, distribution(distribution)
		, fresnel(etaA, etaB)
	{
	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		if (SameHemisphere(wo, wi))
			return 0; // transmission only

		float cosThetaO = CosTheta(wo);
		float cosThetaI = CosTheta(wi);
		if (cosThetaI == 0 || cosThetaO == 0)
			return Spectrum(0);

		// Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
		float	 eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
		Vector3f wh	 = normalize(wo + wi * eta);
		if (wh.z < 0)
			wh = -wh;

		// Same side?
		if (dot(wo, wh) * dot(wi, wh) > 0)
			return Spectrum(0);

		Spectrum F = fresnel.Evaluate(dot(wo, wh));

		float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
		float factor	= 1.0f / eta;

		return (Spectrum(1.0f) - F) * T *
			   std::abs(
				   distribution->D(wh) * distribution->G(wo, wi) * eta * eta * absdot(wi, wh) * absdot(wo, wh) *
				   factor * factor / (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
	}

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override
	{
		if (!SameHemisphere(wo, wi))
		{
			return 0.0f;
		}

		// Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
		float	 eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
		Vector3f wh	 = normalize(wo + wi * eta);

		if (dot(wo, wh) * dot(wi, wh) > 0)
			return 0;

		// Compute change of variables _dwh\_dwi_ for microfacet transmission
		float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
		float dwh_dwi	= std::abs((eta * eta * dot(wi, wh)) / (sqrtDenom * sqrtDenom));
		return distribution->Pdf(wo, wh) * dwh_dwi;
	}

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All)
		const override
	{
		if (wo.z == 0)
			return {};
		Vector3f wh = distribution->Sample_wh(wo, Xi);
		if (dot(wo, wh) < 0)
			return {}; // Should be rare

		float	 eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
		Vector3f wi;
		if (!Refract(wo, wh, eta, &wi))
			return {};

		return BSDFSample(f(wo, wi), wi, Pdf(wo, wi), Flags());
	}

	BxDFFlags Flags() const override { return BxDFFlags::GlossyTransmission; }

	Spectrum				T;
	float					etaA, etaB;
	MicrofacetDistribution* distribution;
	FresnelDielectric		fresnel;
};
