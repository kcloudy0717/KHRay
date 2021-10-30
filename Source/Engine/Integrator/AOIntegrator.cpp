#include "AOIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum AOIntegrator::Li(Ray ray, const Scene& scene, Sampler& sampler)
{
	Spectrum L(0);

	std::optional<SurfaceInteraction> si = scene.Intersect(ray);
	if (si)
	{
		// Compute coordinate frame based on true geometry, not shading
		// geometry.
		auto n = Faceforward(si->GeometryFrame.n, -ray.Direction);
		Vector3f s, t;
		CoordinateSystem(n, &s, &t);

		for (int i = 0; i < NumSamples; ++i)
		{
			Vector3f wi;
			float pdf = 0.0f;;

			switch (Strategy)
			{
			case SamplingStrategy::Uniform:
				wi = SampleUniformHemisphere(sampler.Get2D());
				pdf = UniformHemispherePdf();
				break;
			case SamplingStrategy::Cosine:
				wi = SampleCosineHemisphere(sampler.Get2D());
				pdf = CosineHemispherePdf(std::abs(wi.z));
				break;
			}

			// Transform wi from local frame to world space.
			wi = Vector3f(s.x * wi.x + t.x * wi.y + n.x * wi.z,
				s.y * wi.x + t.y * wi.y + n.y * wi.z,
				s.z * wi.x + t.z * wi.y + n.z * wi.z);

			Ray occlusionRay = {};
			occlusionRay.Origin = si->p;
			occlusionRay.TMin = 0.0001f;
			occlusionRay.Direction = wi;
			occlusionRay.TMax = INFINITY;

			if (!scene.Occluded(occlusionRay))
			{
				L += Dot(wi, n) / pdf;
			}
		}
	}

	return L /= float(NumSamples);
}

std::unique_ptr<AOIntegrator> CreateAOIntegrator(int NumSamples, SamplingStrategy Strategy /*= SamplingStrategy::Cosine*/)
{
	return std::make_unique<AOIntegrator>(NumSamples, Strategy);
}