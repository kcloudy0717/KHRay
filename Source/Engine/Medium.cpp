#include "Medium.h"
#include "Interaction.h"

float HenyeyGreenstein::p(Vector3f wo, Vector3f wi) const noexcept
{
	return PhaseHG(dot(wo, wi), g);
}

float HenyeyGreenstein::Sample_p(Vector3f wo, Vector3f* wi, Vector2f Xi) const noexcept
{
	// Section 15.2.3 Sampling Phase Functions
	// (https://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Volume_Scattering#SamplingPhaseFunctions)

	// Compute $\cos \theta$ for Henyey--Greenstein sample
	float cosTheta;
	if (std::abs(g) < 1e-3)
	{
		cosTheta = 1 - 2 * Xi[0];
	}
	else
	{
		float sqrTerm = (1 - g * g) / (1 + g - 2 * g * Xi[0]);
		cosTheta	  = -(1 + g * g - sqrTerm * sqrTerm) / (2 * g);
	}

	// Compute direction _wi_ for Henyey--Greenstein sample
	float	 sinTheta = std::sqrt(std::max(0.0f, 1 - cosTheta * cosTheta));
	float	 phi	  = 2 * g_PI * Xi[1];
	Vector3f v1, v2;
	coordinatesystem(wo, &v1, &v2);
	*wi = sphericaldirection(sinTheta, cosTheta, phi, v1, v2, wo);
	return PhaseHG(cosTheta, g);
}

Spectrum HomogeneousMedium::Tr(RayDesc ray, Sampler& sampler) const noexcept
{
	return Exp(-sigma_t * std::min(ray.TMax * ray.Direction.Length(), std::numeric_limits<float>::max()));
}

Spectrum HomogeneousMedium::Sample(RayDesc ray, Sampler& sampler, MediumInteraction* mi) const noexcept
{
	//<<Sample a channel and distance along the ray>>=
	int	  channel		= std::min((int)(sampler.Get1D() * Spectrum::NumCoefficients), Spectrum::NumCoefficients - 1);
	float dist			= -std::log(1 - sampler.Get1D()) / sigma_t[channel];
	float t				= std::min(dist * ray.Direction.Length(), ray.TMax);
	bool  sampledMedium = t < ray.TMax;
	if (sampledMedium)
	{
		*mi = MediumInteraction(ray.At(t), -ray.Direction, this, std::make_unique<HenyeyGreenstein>(g));
	}

	//<<Compute the transmittance and sampling density>>=
	Spectrum Tr = Exp(-sigma_t * std::min(t, std::numeric_limits<float>::max()) * ray.Direction.Length());

	//<<Return weighting factor for scattering from homogeneous medium>>=
	Spectrum density = sampledMedium ? (sigma_t * Tr) : Tr;
	float	 pdf	 = 0.0f;
	for (int i = 0; i < Spectrum::NumCoefficients; ++i)
		pdf += density[i];
	pdf *= 1.0f / (float)Spectrum::NumCoefficients;
	return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
}
