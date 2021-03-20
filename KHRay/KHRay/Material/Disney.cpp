#include "Disney.h"

/*

Implementation of the Disney BSDF with Subsurface Scattering, as described in:
http://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf.

That model is based on the Disney BRDF, described in:
https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf

Many thanks for Brent Burley and Karl Li for answering many questions about
the details of the implementation.

The initial implementation of the BRDF was adapted from
https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf, which is
licensed under a slightly-modified Apache 2.0 license.

*/

// Sampling functions for disney brdf
// Refer to B in paper for sampling functions
Vector3f SampleGTR1(const Vector2f& Xi, float alpha)
{
	float phi = 2.0f * g_PI * Xi[0];
	float theta = 0.0f;
	if (alpha < 1.0f)
	{
		theta = acos(sqrt((1 - pow(pow(alpha, 2), 1 - Xi[1])) / (1 - pow(alpha, 2))));
	}
	return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

Vector3f SampleGTR2(const Vector2f& Xi, float alpha)
{
	float phi = 2.0f * g_PI * Xi[0];
	float theta = acos(sqrt((1 - Xi[1]) / (1 + (pow(alpha, 2) - 1) * Xi[1])));
	return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

inline float sqr(float x)
{
	return x * x;
}

// https://schuttejoe.github.io/post/disneybsdf/
inline float D_GTR1(float cosTheta, float alpha)
{
	// Eq 4
	float a2 = alpha * alpha;
	return (a2 - 1.0f) / (g_PI * std::log(a2) * (1.0f + (a2 - 1.0f) * cosTheta * cosTheta));
}

inline float D_GTR2(float cosTheta, float alpha)
{
	// Eq 8
	float a2 = alpha * alpha;
	float t = 1.0f + (a2 - 1.0f) * cosTheta * cosTheta;
	return a2 / (g_PI * t * t);
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

inline Spectrum FrSchlick(const Spectrum& R0, float cosTheta)
{
	return Lerp(R0, Spectrum(1.0), SchlickWeight(cosTheta));
}

// For a dielectric, R(0) = (eta - 1)^2 / (eta + 1)^2, assuming we're
// coming from air..
inline float SchlickR0FromEta(float eta)
{
	return sqr(eta - 1) / sqr(eta + 1);
}

struct DisneySheen : BxDF
{
public:
	DisneySheen(const Spectrum& R)
		: R(R)
	{

	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		Vector3f wh = wi + wo;
		if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Spectrum(0.);
		wh = Normalize(wh);
		float cosThetaD = Dot(wi, wh);

		return R * SchlickWeight(cosThetaD);
	}

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override
	{
		return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * g_1DIVPI : 0;
	}

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const override
	{
		// Cosine-sample the hemisphere, flipping the direction if necessary
		Vector3f wi = SampleCosineHemisphere(Xi);
		if (wo.z < 0) wi.z *= -1;
		return BSDFSample(f(wo, wi), wi, Pdf(wo, wi), Flags());
	}

	BxDFFlags Flags() const override
	{
		return BxDFFlags::DiffuseReflection;
	}

	Spectrum R;
};

struct DisneyClearcoat : BxDF
{
public:
	DisneyClearcoat(float weight, float gloss)
		: weight(weight)
		, gloss(gloss)
	{

	}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	{
		Vector3f wh = wi + wo;
		if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Spectrum(0.);
		wh = Normalize(wh);

		// Clearcoat has ior = 1.5 hardcoded -> F0 = 0.04. It then uses the
		// GTR1 distribution, which has even fatter tails than Trowbridge-Reitz
		// (which is GTR2).
		float Dr = D_GTR1(AbsCosTheta(wh), gloss);
		float Fr = FrSchlick(0.04f, Dot(wo, wh));
		// The geometric term always based on alpha = 0.25.
		float Gr = smithG_GGX(AbsCosTheta(wo), 0.25f) * smithG_GGX(AbsCosTheta(wi), 0.25f);

		return weight * Gr * Fr * Dr / 4.0f;
	}

	float Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types = BxDFTypes::All) const override
	{
		if (!SameHemisphere(wo, wi))
		{
			return 0.0f;
		}

		Vector3f wh = Normalize(wi + wo);

		// The sampling routine samples wh exactly from the GTR1 distribution.
		// Thus, the final value of the PDF is just the value of the
		// distribution for wh converted to a mesure with respect to the
		// surface normal.
		float Dr = D_GTR1(AbsCosTheta(wh), gloss);
		return Dr * AbsCosTheta(wh) / (4 * Dot(wo, wh));
	}

	std::optional<BSDFSample> Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types = BxDFTypes::All) const override
	{
		// Sample microfacet orientation $\wh$ and reflected direction $\wi$
		if (wo.z == 0) { return {}; }

		float alpha2 = gloss * gloss;
		float cosTheta = std::sqrt(std::max(float(0), (1 - std::pow(alpha2, 1 - Xi[0])) / (1 - alpha2)));
		float sinTheta = std::sqrt(std::max(float(0), 1 - cosTheta * cosTheta));
		float phi = 2.0f * g_PI * Xi[1];
		Vector3f wh = SphericalDirection(sinTheta, cosTheta, phi);
		if (!SameHemisphere(wo, wh)) wh = -wh;

		Vector3f wi = Reflect(wo, wh);
		if (!SameHemisphere(wo, wi)) return {};

		float pdf = Pdf(wo, wi);

		return BSDFSample(f(wo, wi), wi, pdf, Flags());
	}

	BxDFFlags Flags() const override
	{
		return BxDFFlags::GlossyReflection;
	}

	float weight, gloss;
};

class DisneyMicrofacetDistribution : public TrowbridgeReitzDistribution
{
public:
	DisneyMicrofacetDistribution(float alphax, float alphay)
		: TrowbridgeReitzDistribution(alphax, alphay)
	{

	}

	float G(const Vector3f& wo, const Vector3f& wi) const override
	{
		// Disney uses the separable masking-shadowing model.
		return G1(wo) * G1(wi);
	}
};

struct DisneyFresnel : Fresnel
{
	DisneyFresnel(const Spectrum& R0, float metallic, float eta)
		: R0(R0)
		, metallic(metallic)
		, eta(eta)
	{

	}

	Spectrum Evaluate(float cosThetaI) const override
	{
		return Lerp(Spectrum(FrDielectric(cosThetaI, 1, eta)), FrSchlick(R0, cosThetaI), metallic);
	}

	Spectrum R0;
	float metallic;
	float eta;
};

Spectrum Disney::f(const Vector3f& wo, const Vector3f& wi) const
{
	Vector3f wh = Normalize(wi + wo);
	float cosThetaD = Dot(wi, wh);

	float luminance = baseColor.y();
	Spectrum Ctint = luminance > 0.0f ? baseColor / luminance : Spectrum(1.0f);
	Spectrum Cspec0 = Lerp(specular * 0.08f * Lerp(Spectrum(1.0f), Ctint, specularTint), baseColor, metallic);
	Spectrum Csheen = Lerp(Spectrum(1.0f), Ctint, sheenTint);

	// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
	// and mix in diffuse retro-reflection based on roughness
	float Fo = SchlickWeight(AbsCosTheta(wo));
	float Fi = SchlickWeight(AbsCosTheta(wi));
	float Fd90 = 0.5f + 2.0f * cosThetaD * cosThetaD * roughness;
	float Fd = std::lerp(1.0f, Fd90, Fo) * std::lerp(1.0f, Fd90, Fi);

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	// Fss90 used to "flatten" retroreflection based on roughness
	float Fss90 = cosThetaD * cosThetaD * roughness;
	float Fss = std::lerp(1.0f, Fss90, Fo) * std::lerp(1.0f, Fss90, Fi);
	// 1.25 scale is used to (roughly) preserve albedo
	float ss = 1.25f * (Fss * (1.0f / (AbsCosTheta(wo) + AbsCosTheta(wi)) - 0.5f) + 0.5f);


	// Sheen
	DisneySheen DisneySheen(sheen * Csheen);

	/*
	// Specular is Trowbridge-Reitz with a modified Fresnel function.
	float aspect = std::sqrt(1.0f - 0.9f * anisotropic);
	float ax = std::max(0.001f, roughness * roughness / aspect);
	float ay = std::max(0.001f, roughness * roughness * aspect);
	DisneyMicrofacetDistribution distribution(ax, ay);

	float specTint = specularTint;
	Spectrum R0 = Lerp(SchlickR0FromEta(eta) * Lerp(Spectrum(1.), Ctint, specTint), baseColor, metallic);

	DisneyFresnel fresnel(R0, metallic, eta);

	MicrofacetReflection reflection(Spectrum(1.0f), &distribution, &fresnel);
	*/

	// specular
	//float aspect = sqrt(1-mat.anisotropic*.9);
	//float ax = Max(.001f, sqr(mat.roughness)/aspect);
	//float ay = Max(.001f, sqr(mat.roughness)*aspect);
	//float Ds = GTR2_aniso(NDotH, Dot(H, X), Dot(H, Y), ax, ay);

	float a = std::max(0.001f, roughness);
	float Ds = D_GTR2(AbsCosTheta(wh), a);
	float FH = SchlickWeight(cosThetaD);
	Spectrum Fs = Lerp(Cspec0, Spectrum(1.0f), FH);
	float roughg = sqr(roughness * 0.5f + 0.5f);
	float Gs = smithG_GGX(AbsCosTheta(wo), roughg) * smithG_GGX(AbsCosTheta(wi), roughg);

	// Clearcoat
	DisneyClearcoat DisneyClearcoat(clearcoat, std::lerp(0.1f, 0.001f, clearcoatGloss));

	/*
	// BTDF
	if (specTrans > 0.0f)
	{
		Spectrum T = specTrans * Sqrt(baseColor);
		float rscaled = thin ? (0.65f * eta - 0.35f) * roughness : roughness;
		float tax = std::max(0.001f, rscaled * rscaled / aspect);
		float tay = std::max(0.001f, rscaled * rscaled * aspect);
		DisneyMicrofacetDistribution scaledDistribution(tax, tay);

		MicrofacetTransmission transmission(T, 1.0f, eta, &scaledDistribution);
	}
	*/

	return ((1.0f / g_PI) * std::lerp(Fd, ss, subsurface) * baseColor + DisneySheen.f(wo, wi)) * (1.0f - metallic)
		+ Gs * Fs * Ds
		+ DisneyClearcoat.f(wo, wi);
}

float Disney::Pdf(const Vector3f& wo, const Vector3f& wi, BxDFTypes Types) const
{
	Vector3f wh = Normalize(wo + wi);
	float cosTheta = AbsCosTheta(wh);

	float specularAlpha = std::max(0.001f, roughness);
	float clearcoatAlpha = std::lerp(0.1f, 0.001f, clearcoatGloss);

	float diffuseRatio = 0.5f * (1.0f - metallic);
	float specularRatio = 1.0f - diffuseRatio;

	float pdfGTR2 = D_GTR2(cosTheta, specularAlpha) * cosTheta;
	float pdfGTR1 = D_GTR1(cosTheta, clearcoatAlpha) * cosTheta;

	// calculate diffuse and specular pdfs and mix ratio
	float ratio = 1.0f / (1.0f + clearcoat);
	float pdfSpec = std::lerp(pdfGTR1, pdfGTR2, ratio) / (4.0 * abs(Dot(wi, wh)));
	float pdfDiff = AbsCosTheta(wi) * g_1DIVPI;

	// weight pdfs according to ratios
	return diffuseRatio * pdfDiff + specularRatio * pdfSpec;
}

std::optional<BSDFSample> Disney::Samplef(const Vector3f& wo, const Vector2f& Xi, BxDFTypes Types) const
{
	// http://simon-kallweit.me/rendercompo2015/report/#disneybrdf, refer to this link
	// for how to importance sample the disney brdf

	Vector3f wi;

	float diffuse_ratio = 0.5f * (1.0f - metallic);

	// Sample diffuse
	if (Xi[0] < diffuse_ratio)
	{
		Vector2f _Xi = Vector2f(Xi[0] / diffuse_ratio, Xi[1]);

		wi = SampleCosineHemisphere(_Xi);

		return BSDFSample(f(wo, wi), wi, Pdf(wo, wi), Flags());
	}
	// Sample specular
	else
	{
		Vector2f _Xi = Vector2f((Xi[0] - diffuse_ratio) / (1.0f - diffuse_ratio), Xi[1]);

		float gtr2_ratio = 1.0f / (1.0f + clearcoat);

		if (_Xi[0] < gtr2_ratio)
		{
			_Xi[0] /= gtr2_ratio;

			float alpha = std::max(0.01f, pow(roughness, 2.0f));
			Vector3f wh = SampleGTR2(_Xi, alpha);

			wi = Normalize(Reflect(wo, wh));
		}
		else
		{
			_Xi[0] = (_Xi[0] - gtr2_ratio) / (1 - gtr2_ratio);

			float alpha = std::lerp(0.1f, 0.001f, clearcoatGloss);
			Vector3f wh = SampleGTR1(_Xi, alpha);

			wi = Normalize(Reflect(wo, wh));
		}

		return BSDFSample(f(wo, wi), wi, Pdf(wo, wi), Flags());
	}
}