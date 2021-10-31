#pragma once
#include "BxDF.h"
#include "Math/Math.h"

class BSDF
{
public:
	operator bool() const noexcept { return static_cast<bool>(pBxDF); }

	BSDF Clone() const { return *this; }

	void SetBxDF(std::shared_ptr<BxDF> pBxDF);

	void SetInteraction(const SurfaceInteraction& Interaction);

	Vector3f WorldToLocal(const Vector3f& v) const { return ShadingFrame.ToLocal(v); }

	Vector3f LocalToWorld(const Vector3f& v) const { return ShadingFrame.ToWorld(v); }

	bool IsNonSpecular() const { return (pBxDF->Flags() & (BxDFFlags::Diffuse | BxDFFlags::Glossy)); }
	bool IsDiffuse() const { return (pBxDF->Flags() & BxDFFlags::Diffuse); }
	bool IsGlossy() const { return (pBxDF->Flags() & BxDFFlags::Glossy); }
	bool IsSpecular() const { return (pBxDF->Flags() & BxDFFlags::Specular); }
	bool HasReflection() const { return (pBxDF->Flags() & BxDFFlags::Reflection); }
	bool HasTransmission() const { return (pBxDF->Flags() & BxDFFlags::Transmission); }

	// Evaluate the BSDF for a pair of directions
	Spectrum f(const Vector3f& woW, const Vector3f& wiW) const;

	// Compute the pdf of sampling
	float Pdf(const Vector3f& woW, const Vector3f& wiW, BxDFTypes Types = BxDFTypes::All) const;

	// Samples the BSDF
	std::optional<BSDFSample> Samplef(const Vector3f& woW, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const;

private:
	Vector3f			  Ng;
	Frame				  ShadingFrame;
	std::shared_ptr<BxDF> pBxDF;
};
