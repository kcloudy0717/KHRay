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

float MicrofacetDistribution::Pdf(const Vector3f& wo, const Vector3f& wh) const
{
	if (sampleVisibleArea)
	{
		return D(wh) * G1(wo) * AbsDot(wo, wh) / AbsCosTheta(wo);
	}

	return D(wh) * AbsCosTheta(wh);
}

float TrowbridgeReitzDistribution::D(const Vector3f& wh) const
{
	// http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html#MicrofacetDistributionFunctions
	float tan2Theta = Tan2Theta(wh);
	if (std::isinf(tan2Theta))
	{
		return 0.0f;
	}

	float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
	float e = (Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay)) * tan2Theta;
	return 1.0f / (g_PI * alphax * alphay * cos4Theta * (1.0f + e) * (1.0f + e));
}

// https://github.com/mmp/pbrt-v3/blob/master/src/core/microfacet.cpp#L238
Vector2f TrowbridgeReitzSample11(float cosTheta, float U1, float U2)
{
	Vector2f slope;

	// special case (normal incidence)
	if (cosTheta > .9999) {
		float r = sqrt(U1 / (1 - U1));
		float phi = g_2PI * U2;
		slope.x = r * cos(phi);
		slope.y = r * sin(phi);
		return slope;
	}

	float sinTheta = std::sqrt(std::max((float)0, (float)1 - cosTheta * cosTheta));
	float tanTheta = sinTheta / cosTheta;
	float a = 1 / tanTheta;
	float G1 = 2 / (1 + std::sqrt(1.f + 1.f / (a * a)));

	// sample slope_x
	float A = 2 * U1 / G1 - 1;
	float tmp = 1.f / (A * A - 1.f);
	if (tmp > 1e10) tmp = 1e10;
	float B = tanTheta;
	float D = std::sqrt(
		std::max(float(B * B * tmp * tmp - (A * A - B * B) * tmp), float(0)));
	float slope_x_1 = B * tmp - D;
	float slope_x_2 = B * tmp + D;
	slope.x = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

	// sample slope_y
	float S;
	if (U2 > 0.5f)
	{
		S = 1.f;
		U2 = 2.f * (U2 - .5f);
	}
	else
	{
		S = -1.f;
		U2 = 2.f * (.5f - U2);
	}
	float z =
		(U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
		(U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
	slope.y = S * z * std::sqrt(1.f + slope.x * slope.x);

	return slope;
}

// https://github.com/mmp/pbrt-v3/blob/master/src/core/microfacet.cpp#L284
Vector3f TrowbridgeReitzSample(const Vector3f& wi, float alpha_x, float alpha_y, float U1, float U2)
{
	// 1. stretch wi
	Vector3f wiStretched = Normalize(Vector3f(alpha_x * wi.x, alpha_y * wi.y, wi.z));

	// 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
	Vector2f slope = TrowbridgeReitzSample11(CosTheta(wiStretched), U1, U2);

	// 3. rotate
	float tmp = CosPhi(wiStretched) * slope.x - SinPhi(wiStretched) * slope.y;
	slope.y = SinPhi(wiStretched) * slope.x + CosPhi(wiStretched) * slope.y;
	slope.x = tmp;

	// 4. unstretch
	slope.x = alpha_x * slope.x;
	slope.y = alpha_y * slope.y;

	// 5. compute normal
	return Normalize(Vector3f(-slope.x, -slope.y, 1.));
}

// https://github.com/mmp/pbrt-v3/blob/master/src/core/microfacet.cpp#L307
Vector3f TrowbridgeReitzDistribution::Sample_wh(const Vector3f& wo, const Vector2f& Xi) const
{
	Vector3f wh;
	if (!sampleVisibleArea)
	{
		float cosTheta = 0, phi = (2 * g_PI) * Xi[1];
		if (alphax == alphay)
		{
			float tanTheta2 = alphax * alphax * Xi[0] / (1.0f - Xi[0]);
			cosTheta = 1 / std::sqrt(1 + tanTheta2);
		}
		else
		{
			phi = std::atan(alphay / alphax * std::tan(2 * g_PI * Xi[1] + .5f * g_PI));
			if (Xi[1] > .5f) phi += g_PI;
			float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
			const float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
			const float alpha2 =
				1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
			float tanTheta2 = alpha2 * Xi[0] / (1 - Xi[0]);
			cosTheta = 1 / std::sqrt(1 + tanTheta2);
		}
		float sinTheta = std::sqrt(std::max((float)0., (float)1. - cosTheta * cosTheta));
		wh = SphericalDirection(sinTheta, cosTheta, phi);
		if (!SameHemisphere(wo, wh)) wh = -wh;
	}
	else
	{
		bool flip = wo.z < 0;
		wh = TrowbridgeReitzSample(flip ? -wo : wo, alphax, alphay, Xi[0], Xi[1]);
		if (flip) wh = -wh;
	}
	return wh;
}

float TrowbridgeReitzDistribution::Lambda(const Vector3f& w) const
{
	// http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html#MaskingandShadowing
	float absTanTheta = std::abs(TanTheta(w));
	if (std::isinf(absTanTheta))
	{
		return 0.0f;
	}
	// Compute _alpha_ for direction _w_
	float alpha = std::sqrt(Cos2Phi(w) * alphax * alphax + Sin2Phi(w) * alphay * alphay);
	float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
	return (-1.0f + std::sqrt(1.0f + alpha2Tan2Theta)) / 2.0f;
}