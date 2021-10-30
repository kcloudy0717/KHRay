#include "NormalIntegrator.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

Spectrum NormalIntegrator::Li(Ray ray, const Scene& scene, Sampler& sampler)
{
	std::optional<SurfaceInteraction> si = scene.Intersect(ray);
	if (!si)
	{
		return Spectrum(0.0f);
	}

	Vector3f n;
	switch (ViewType)
	{
	case Geometric:
		n = si->GeometryFrame.n;
		break;
	case Shading:
		n = si->ShadingFrame.n;
		break;
	}

	n = Abs(n);
	return { n.x, n.y, n.z };
}

std::unique_ptr<NormalIntegrator> CreateNormalIntegrator(NormalView ViewType)
{
	return std::make_unique<NormalIntegrator>(ViewType);
}