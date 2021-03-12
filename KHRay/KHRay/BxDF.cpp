#include "BxDF.h"
#include "Scene.h"

#include <algorithm>

float FrDielectric(float cosThetaI, float etaI, float etaT)
{
	cosThetaI = std::clamp<float>(cosThetaI, -1.0f, 1.0f);

	// Potentially swap indices of refraction
	bool entering = cosThetaI > 0.0f;
	if (!entering)
	{
		std::swap(etaI, etaT);
		cosThetaI = std::abs(cosThetaI);
	}

	// Compute cosThetaT using Snell's law
	float sinThetaI = std::sqrt(std::max(0.0f, 1.0f - cosThetaI * cosThetaI));
	float sinThetaT = etaI / etaT * sinThetaI;

	// Handle total internal reflection
	if (sinThetaT >= 1.0f)
	{
		return 1.0f;
	}

	float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sinThetaT * sinThetaT));

	float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
	float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
	return (Rparl * Rparl + Rperp * Rperp) / 2.0f;
}

Spectrum FrConductor(float cosThetaI, const Spectrum& etaI, const Spectrum& etaT, const Spectrum& k)
{
	cosThetaI = std::clamp<float>(cosThetaI, -1.0f, 1.0f);

	Spectrum eta = etaT / etaI;
	Spectrum etak = k / etaI;

	float cosThetaI2 = cosThetaI * cosThetaI;
	float sinThetaI2 = 1.0f - cosThetaI2;
	Spectrum eta2 = eta * eta;
	Spectrum etak2 = etak * etak;

	Spectrum t0 = eta2 - etak2 - sinThetaI2;
	Spectrum a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
	Spectrum t1 = a2plusb2 + cosThetaI2;
	Spectrum a = Sqrt(0.5f * (a2plusb2 + t0));
	Spectrum t2 = 2.0f * cosThetaI * a;
	Spectrum Rs = (t1 - t2) / (t1 + t2);

	Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
	Spectrum t4 = t2 * sinThetaI2;
	Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

	return 0.5 * (Rp + Rs);
}

void BSDF::SetInteraction(const SurfaceInteraction& Interaction)
{
	Ng = Interaction.GeometryBasis.n;
	Ns = Interaction.ShadingBasis.n;
	ss = Interaction.ShadingBasis.s;
	ts = Interaction.ShadingBasis.t;
}

Spectrum BSDF::f(const Vector3f& woW, const Vector3f& wiW, BxDF::Type Flags /*= BxDF::BSDF_All*/) const
{
	Vector3f wo = WorldToLocal(woW), wi = WorldToLocal(wiW);
	if (wo.z == 0.0f)
	{
		return Spectrum(0.0f);
	}

	bool reflect = Dot(wiW, Ng) * Dot(woW, Ng) > 0.0f;

	Spectrum f(0.0f);
	for (int i = 0; i < NumBxDF; ++i)
	{
		if (BxDFs[i]->MatchesFlags(Flags) &&
			((reflect && (BxDFs[i]->_Type & BxDF::BSDF_Reflection)) ||
				(!reflect && (BxDFs[i]->_Type & BxDF::BSDF_Transmission))))
		{
			f += BxDFs[i]->f(wo, wi);
		}
	}

	return f;
}

Spectrum BSDF::Samplef(const Vector3f& woW, Vector3f* wiW, const Vector2f& Xi, float* pdf, BxDF::Type Flags /*= BxDF::BSDF_All*/) const
{
	static constexpr float OneMinusEpsilon = 0x1.fffffep-1;

	// Choose which _BxDF_ to sample
	int matchingComps = NumComponents(Flags);
	if (matchingComps == 0)
	{
		*pdf = 0.0f;
		return Spectrum(0.0f);
	}

	int comp = std::min((int)std::floor(Xi[0] * matchingComps), matchingComps - 1);

	// Get _BxDF_ pointer for chosen component
	BxDF* bxdf = nullptr;
	int count = comp;
	for (int i = 0; i < NumBxDF; ++i)
	{
		if (BxDFs[i]->MatchesFlags(Flags) && count-- == 0)
		{
			bxdf = BxDFs[i].get();
			break;
		}
	}

	if (!bxdf)
	{
		return Spectrum(0.0f);
	}

	// Remap _BxDF_ sample _u_ to $[0,1)^2$
	Vector2f XiRemapped(std::min(Xi[0] * matchingComps - comp, OneMinusEpsilon), Xi[1]);

	// Sample chosen _BxDF_
	Vector3f wi, wo = WorldToLocal(woW);
	if (wo.z == 0)
	{
		return Spectrum(0.0f);
	}

	*pdf = 0.0f;
	Spectrum f = bxdf->Samplef(wo, &wi, XiRemapped, pdf);

	if (*pdf == 0.0f)
	{
		return Spectrum(0.0f);
	}
	*wiW = LocalToWorld(wi);

	// Compute overall PDF with all matching _BxDF_s
	bool isSpecular = bxdf->_Type & BxDF::BSDF_Specular;
	if (!isSpecular && matchingComps > 1)
	{
		for (int i = 0; i < NumBxDF; ++i)
		{
			if (BxDFs[i].get() != bxdf && BxDFs[i]->MatchesFlags(Flags))
			{
				*pdf += BxDFs[i]->Pdf(wo, wi);
			}
		}
	}

	if (matchingComps > 1)
	{
		*pdf /= float(matchingComps);
	}

	// Compute value of BSDF for sampled direction
	if (!isSpecular)
	{
		bool reflect = Dot(*wiW, Ng) * Dot(woW, Ng) > 0;
		f = Spectrum(0.0f);;
		for (int i = 0; i < NumBxDF; ++i)
		{
			if (BxDFs[i]->MatchesFlags(Flags) &&
				((reflect && (BxDFs[i]->_Type & BxDF::BSDF_Reflection)) ||
					(!reflect && (BxDFs[i]->_Type & BxDF::BSDF_Transmission))))
			{
				f += BxDFs[i]->f(wo, wi);
			}
		}
	}

	return f;
}

float BSDF::Pdf(const Vector3f& woW, const Vector3f& wiW, BxDF::Type Flags /*= BxDF::BSDF_All*/) const
{
	Vector3f wo = WorldToLocal(woW), wi = WorldToLocal(wiW);
	if (wo.z == 0.0f)
	{
		return 0.0f;
	}

	int matchingComps = 0;

	float Pdf = 0.0f;
	for (int i = 0; i < NumBxDF; ++i)
	{
		if (BxDFs[i]->MatchesFlags(Flags))
		{
			++matchingComps;
			Pdf += BxDFs[i]->Pdf(wo, wi);
		}
	}

	return matchingComps > 0 ? Pdf / matchingComps : 0.0f;
}