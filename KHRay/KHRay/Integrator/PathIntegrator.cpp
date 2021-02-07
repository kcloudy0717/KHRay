#include "PathIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum PathIntegrator::Li(Ray ray, const Scene& scene, Sampler& sampler)
{
	Spectrum L(0), beta(1);
	bool specularBounce = false;

	for (int bounces = 0; ; ++bounces)
	{
		SurfaceInteraction interaction;
		bool foundIntersection = scene.Intersect(ray, &interaction);

		if (!foundIntersection || bounces >= MaxDepth)
		{
			break;
		}

		// Sample illumination from lights to find path contribution.
		// (But skip this for perfectly specular BSDFs.)
		if (interaction.BSDF->NumComponents(BxDF::Type(BxDF::BSDF_All & ~BxDF::BSDF_Specular)) > 0)
		{
			L += UniformSampleOneLight(interaction, scene, sampler);
		}

		// Sample BSDF to get new path direction
		Vector3f wo = -ray.Direction, wi;
		float pdf;
		Spectrum f = interaction.BSDF->Samplef(wo, &wi, sampler.Get2D(), &pdf);

		if (f.IsBlack() || pdf == 0.0f)
		{
			break;
		}

		beta *= f * AbsDot(wi, interaction.ShadingBasis.n) / pdf;

		ray = interaction.SpawnRay(wi);

		// Possibly terminate the path with Russian roulette.
		// Factor out radiance scaling due to refraction in rrBeta.
		Spectrum rrBeta = beta;
		if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3)
		{
			float q = std::max((float).05, 1.0f - rrBeta.MaxComponentValue());
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
	return std::unique_ptr<PathIntegrator>(new PathIntegrator(MaxDepth));
}