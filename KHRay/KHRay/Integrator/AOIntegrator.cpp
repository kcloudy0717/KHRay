#include "AOIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"
#include "../Sampling.h"

Spectrum AOIntegrator::Li(const Ray& ray, const Scene& scene, Sampler& sampler)
{
	Spectrum L(0);

	Intersection intersection = {};
	if (scene.Intersect(ray, &intersection))
	{
		// Compute coordinate frame based on true geometry, not shading
		// geometry.
		auto n = Faceforward(intersection.geoFrame.n, -ray.Direction);
		Vector3f s, t;
		CoordinateSystem(n, &s, &t);

		for (int i = 0; i < NumSamples; ++i)
		{
			auto wi = CosineSampleHemisphere(sampler.Get2D());
			auto pdf = CosineHemispherePdf(std::abs(wi.z));

			// Transform wi from local frame to world space.
			wi = Vector3f(s.x * wi.x + t.x * wi.y + n.x * wi.z,
				s.y * wi.x + t.y * wi.y + n.y * wi.z,
				s.z * wi.x + t.z * wi.y + n.z * wi.z);

			Ray occlusionRay = {};
			occlusionRay.Origin = intersection.p;
			occlusionRay.TMin = 0.001f;
			occlusionRay.Direction = wi;
			occlusionRay.TMax = INFINITY;

			if (!scene.Intersect(occlusionRay, nullptr))
			{
				L += Dot(wi, n) / pdf;
			}
		}
	}

	return L /= float(NumSamples);
}

std::unique_ptr<AOIntegrator> CreateAOIntegrator(int NumSamples)
{
	return std::unique_ptr<AOIntegrator>(new AOIntegrator(NumSamples));
}