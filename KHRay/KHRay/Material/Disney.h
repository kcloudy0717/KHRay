#pragma once
#include "../BxDF.h"

class Disney : public BxDF
{
public:
	Spectrum baseColor;
	float metallic = 0.0f;
	float subsurface = 0.0f;
	float specular = 0.5f;
	float roughness = .5f;
	float specularTint = 0.0f;
	float anisotropic = 0.0f;
	float sheen = 0.0f;
	float sheenTint = 0.5f;
	float clearcoat = 0.0f;
	float clearcoatGloss = 1.0f;
};