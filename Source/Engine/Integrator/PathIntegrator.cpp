#include "PathIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum PathIntegrator::Li(RayDesc ray, const Scene& scene, Sampler& sampler)
{
	Spectrum L(0), beta(1);
	bool specularBounce = false;

	for (int bounces = 0; ; ++bounces)
	{
		std::optional<SurfaceInteraction> si = scene.Intersect(ray);

		if (!si || bounces >= MaxDepth)
		{
			break;
		}

		// Sample illumination from lights to find path contribution.
		// (But skip this for perfectly specular BSDFs.)
		if (si->BSDF.IsNonSpecular())
		{
			L += beta * UniformSampleOneLight(*si, scene, sampler, false);
		}

		// Sample BSDF to get new path direction
		Vector3f wo = -ray.Direction;
		std::optional<BSDFSample> bsdfSample = si->BSDF.Samplef(wo, sampler.Get2D());

		if (!bsdfSample)
		{
			break;
		}

		beta *= bsdfSample->f * absdot(bsdfSample->wi, si->ShadingFrame.n) / bsdfSample->pdf;

		ray = si->SpawnRay(bsdfSample->wi);

		// Possibly terminate the path with Russian roulette.
		Spectrum rrBeta = beta;
		float rrMaxComponentValue = rrBeta.MaxComponentValue();
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

std::unique_ptr<PathIntegrator> CreatePathIntegrator(int MaxDepth)
{
	return std::make_unique<PathIntegrator>(MaxDepth);
}