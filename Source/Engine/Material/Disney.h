#pragma once
#include "../BxDF.h"

struct Disney : public BxDF
{
	Disney() = default;

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override;

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const override;

	BxDFFlags Flags() const override
	{
		return BxDFFlags::Reflection | BxDFFlags::Diffuse | BxDFFlags::Glossy;
	}

	Spectrum baseColor = Spectrum(1.0f);
	float metallic = 0.0f;
	float subsurface = 0.0f;
	float specular = 0.5f;
	float roughness = 0.5f;
	float specularTint = 0.0f;
	float anisotropic = 0.0f;
	float sheen = 0.0f;
	float sheenTint = 0.5f;
	float clearcoat = 0.0f;
	float clearcoatGloss = 1.0f;
};