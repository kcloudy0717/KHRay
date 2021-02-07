#include "BxDF.h"
#include "Scene.h"

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