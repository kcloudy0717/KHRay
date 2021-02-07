#include "NormalIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum NormalIntegrator::Li(Ray ray, const Scene& scene, Sampler& sampler)
{
	SurfaceInteraction interaction;
	if (!scene.Intersect(ray, &interaction))
	{
		return Spectrum(0.0f);
	}

	Vector3f n;
	switch (ViewType)
	{
	case Geometric:
		n = interaction.GeometryBasis.n;
		break;
	case Shading:
		n = interaction.ShadingBasis.n;
		break;
	}

	n = Abs(n);
	return { n.x, n.y, n.z };
}

std::unique_ptr<NormalIntegrator> CreateNormalIntegrator(NormalView ViewType)
{
	return std::unique_ptr<NormalIntegrator>(new NormalIntegrator(ViewType));
}