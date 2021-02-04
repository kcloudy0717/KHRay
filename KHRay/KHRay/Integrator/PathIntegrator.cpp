#include "PathIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum PathIntegrator::Li(const Ray& Ray, const Scene& Scene, Sampler& Sampler)
{
	Spectrum L(0), beta(1);

	for (int bounce = 0; ; ++bounce)
	{
		Intersection intersection = {};
		bool foundIntersection = Scene.Intersect(Ray, &intersection);

		if (foundIntersection)
		{
			L += beta * Spectrum(intersection.shFrame.n.x, intersection.shFrame.n.y, intersection.shFrame.n.z);
		}
		else
		{
			float t = 0.5f * (Ray.Direction.y + 1.0f);
			L += beta * (1.0f - t) * Spectrum(1.0f, 1.0f, 1.0f) + t * Spectrum(0.5f, 0.7f, 1.0f);
		}

		if (!foundIntersection || bounce >= MaxDepth)
		{
			break;
		}
	}

	return L;
}

std::unique_ptr<PathIntegrator> CreatePathIntegrator(int MaxDepth)
{
	return std::unique_ptr<PathIntegrator>(new PathIntegrator(MaxDepth));
}