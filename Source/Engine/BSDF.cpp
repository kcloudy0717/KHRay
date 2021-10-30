#include "BSDF.h"
#include "Scene.h"

void BSDF::SetBxDF(std::shared_ptr<BxDF> pBxDF)
{
	this->pBxDF = pBxDF;
}

void BSDF::SetInteraction(const SurfaceInteraction& Interaction)
{
	Ng			 = Interaction.GeometryFrame.n;
	ShadingFrame = Interaction.ShadingFrame;
}

Spectrum BSDF::f(const Vector3f& woW, const Vector3f& wiW) const
{
	Vector3f wo = WorldToLocal(woW), wi = WorldToLocal(wiW);
	if (wo.z == 0.0f)
	{
		return Spectrum(0.0f);
	}

	return pBxDF->f(wo, wi);
}

float BSDF::Pdf(const Vector3f& woW, const Vector3f& wiW, BxDFTypes Types /*= BxDFTypes::All*/) const
{
	Vector3f wo = WorldToLocal(woW), wi = WorldToLocal(wiW);
	if (wo.z == 0.0f)
	{
		return 0.0f;
	}

	return pBxDF->Pdf(wo, wi, Types);
}

std::optional<BSDFSample> BSDF::Samplef(const Vector3f& woW, const Vector2f& Xi, BxDFTypes Types /*= BxDFTypes::All*/)
	const
{
	Vector3f wo = WorldToLocal(woW);
	if (wo.z == 0.0f || !(pBxDF->Flags() & Types))
	{
		return {};
	}

	auto sample = pBxDF->Samplef(wo, Xi, Types);

	if (!sample || !sample->f || sample->pdf == 0.0f || sample->wi.z == 0.0f)
	{
		return {};
	}

	sample->wi = LocalToWorld(sample->wi);
	return sample;
}
