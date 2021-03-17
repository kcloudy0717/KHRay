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

Spectrum Disney::f(const Vector3f& wo, const Vector3f& wi) const
{
	Vector3f wh = Normalize(wi + wo);
	float cosThetaD = Dot(wi, wh);

	// Diffuse
	float Fo = SchlickWeight(AbsCosTheta(wo));
	float Fi = SchlickWeight(AbsCosTheta(wi));

	Spectrum f_lambert;
	Spectrum f_retro;

	float Fo = SchlickWeight(AbsCosTheta(wo));
	float Fi = SchlickWeight(AbsCosTheta(wi));
	float Rr = 2.0f * roughness * cosThetaD * cosThetaD;

	f_lambert = baseColor / g_PI;
	f_retro = baseColor / g_PI * Rr * (Fo + Fi + Fo * Fi * (Rr - 1.0f));
	Spectrum fd = f_lambert * (1.0f - 0.5f * Fo) * (1 - 0.5f * Fi) + f_retro;

	return fd;
}

float Disney::Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types) const
{
	return 0.0f;
}

std::optional<BSDFSample> Disney::Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types) const
{

}