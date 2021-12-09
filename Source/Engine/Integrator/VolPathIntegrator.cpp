#include "VolPathIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum VolPathIntegrator::Li(RayDesc ray, const Scene& scene, Sampler& sampler)
{
	Spectrum L(0), beta(1);
	bool	 specularBounce = false;

	for (int bounces = 0;; ++bounces)
	{
		std::optional<SurfaceInteraction> si = scene.Intersect(ray);

		// Sample the participating medium, if present
		MediumInteraction mi;
		if (ray.Medium)
		{
			beta *= ray.Medium->Sample(ray, sampler, &mi);
		}
		if (beta.IsBlack())
		{
			break;
		}

		// Handle an interaction with a medium or a surface
		if (mi.IsValid())
		{
			if (bounces >= MaxDepth)
			{
				break;
			}

			L += beta * UniformSampleOneLight(mi, scene, sampler, true);

			Vector3f wo = -ray.Direction, wi;
			mi.phase->Sample_p(wo, &wi, sampler.Get2D());
			ray = mi.SpawnRay(wi);
		}
		else
		{
			// Handle scattering at point on surface for volumetric path tracer
			if (!si || bounces >= MaxDepth)
			{
				break;
			}

			// Sample illumination from lights to find path contribution.
			L += beta * UniformSampleOneLight(*si, scene, sampler, true);

			// Sample BSDF to get new path direction
			Vector3f				  wo		 = -ray.Direction;
			std::optional<BSDFSample> bsdfSample = si->BSDF.Samplef(wo, sampler.Get2D());

			if (!bsdfSample)
			{
				break;
			}

			beta *= bsdfSample->f * absdot(bsdfSample->wi, si->ShadingFrame.n) / bsdfSample->pdf;

			ray = si->SpawnRay(bsdfSample->wi);
		}

		// Possibly terminate the path with Russian roulette.
		Spectrum rrBeta				 = beta;
		float	 rrMaxComponentValue = rrBeta.MaxComponentValue();
		if (rrMaxComponentValue < rrThreshold && bounces > 3)
		{
			float q = std::max(0.05f, 1.0f - rrMaxComponentValue);
			if (sampler.Get1D() < q)
			{
				break;
			}
			beta /= 1.0f - q;
		}
	}

	return L;
}

std::unique_ptr<VolPathIntegrator> CreateVolPathIntegrator(int MaxDepth)
{
	return std::make_unique<VolPathIntegrator>(MaxDepth);
}
