#include "Medium.h"

float HenyeyGreenstein::p(Vector3f wo, Vector3f wi) const noexcept
{
	return PhaseHG(Dot(wo, wi), g);
}

float HenyeyGreenstein::Sample_p(Vector3f wo, Vector3f* wi, Vector2f Xi) const noexcept
{
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
	CoordinateSystem(wo, &v1, &v2);
	*wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, wo);
	return PhaseHG(cosTheta, g);
}

Spectrum HomogeneousMedium::Tr(Ray ray, Sampler& sampler) const noexcept
{
	return Exp(-sigma_t * std::min(ray.TMax * ray.Direction.Length(), std::numeric_limits<float>::max()));
}

Spectrum HomogeneousMedium::Sample(Ray ray, Sampler& sampler, MediumInteraction* mi) const noexcept
{
	return 0.0f;
}
