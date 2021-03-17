#include "Disney.h"

// https://schuttejoe.github.io/post/disneybsdf/
Spectrum CalculateTint(Spectrum Albedo)
{
	float luminance = Albedo.y();
	return luminance > 0.0f ? Albedo / luminance : Spectrum(1.0f);
}

inline float GTR1(float cosTheta, float alpha)
{
	float alpha2 = alpha * alpha;
	return (alpha2 - 1) / (g_PI * std::log(alpha2) * (1 + (alpha2 - 1) * cosTheta * cosTheta));
}

// Smith masking/shadowing term.
inline float smithG_GGX(float cosTheta, float alpha)
{
	float alpha2 = alpha * alpha;
	float cosTheta2 = cosTheta * cosTheta;
	return 1.0f / (cosTheta + sqrt(alpha2 + cosTheta2 - alpha2 * cosTheta2));
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
//
// The Schlick Fresnel approximation is:
//
// R = R(0) + (1 - R(0)) (1 - cos theta)^5,
//
// where R(0) is the reflectance at normal indicence.
inline float SchlickWeight(float cosTheta)
{
	float m = std::clamp<float>(1.0f - cosTheta, 0.0f, 1.0f);
	return m * m * m * m * m;
}

inline float FrSchlick(float R0, float cosTheta)
{
	return std::lerp(R0, 1.0f, SchlickWeight(cosTheta));
}

//struct DisneySheen : BxDF
//{
//	DisneySheen(const Spectrum& R, float Sheen, float SheenTint)
//		: BxDF(BxDF::Type(BxDF::BSDF_Reflection | BxDF::BSDF_Diffuse))
//		, R(R)
//		, Sheen(Sheen)
//		, SheenTint(SheenTint)
//	{
//
//	}
//
//	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
//	{
//		if (Sheen <= 0.0f)
//		{
//			return Spectrum(0.0f);
//		}
//
//		Vector3f wh = Normalize(wi + wo);
//		float cosThetaD = Dot(wi, wh);
//
//		Spectrum tint = CalculateTint(R);
//		return Sheen * Lerp(Spectrum(1.0f), tint, SheenTint) * SchlickWeight(cosThetaD);
//	}
//
//	Spectrum R;
//	float Sheen;
//	float SheenTint;
//};
//
//struct DisneyClearcoat : BxDF
//{
//	DisneyClearcoat(float Clearcoat, float ClearcoatGloss)
//		: BxDF(BxDF::Type(BxDF::BSDF_Reflection | BxDF::BSDF_Glossy))
//		, Clearcoat(Clearcoat)
//		, ClearcoatGloss(ClearcoatGloss)
//	{
//
//	}
//
//	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
//	{
//		Vector3f wh = Normalize(wi + wo);
//
//		// Clearcoat has ior = 1.5 hardcoded -> F0 = 0.04. It then uses the
//		// GTR1 distribution, which has even fatter tails than Trowbridge-Reitz
//		// (which is GTR2).
//		float Dr = GTR1(AbsCosTheta(wh), ClearcoatGloss);
//		float Fr = FrSchlick(0.04f, Dot(wo, wh));
//		// The geometric term always based on alpha = 0.25.
//		float Gr = smithG_GGX(AbsCosTheta(wo), 0.25f) * smithG_GGX(AbsCosTheta(wi), 0.25f);
//
//		return Clearcoat * Gr * Fr * Dr / 4.0f;
//	}
//
//	float Clearcoat;
//	float ClearcoatGloss;
//};
//
//struct DisneyDiffuse : BxDF
//{
//
//};
//
//void Disney::ComputeScatteringFunctions(SurfaceInteraction* pSurfaceInteraction)
//{
//	Spectrum c = baseColor;
//	float metallicWeight = metallic;
//
//	// Clearcoat
//	if (clearcoat > 0.0f)
//	{
//		pSurfaceInteraction->BSDF.Add(std::make_shared<DisneyClearcoat>(clearcoat, std::lerp(0.1f, 0.001f, clearcoatGloss)));
//	}
//}