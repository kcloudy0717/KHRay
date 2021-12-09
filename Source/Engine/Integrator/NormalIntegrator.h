#pragma once
#include "Integrator.h"

enum NormalView
{
	Geometric,
	Shading
};

class NormalIntegrator : public Integrator
{
public:
	NormalIntegrator(NormalView ViewType)
		: ViewType(ViewType)
	{

	}

	Spectrum Li(RayDesc ray, const Scene& scene, Sampler& sampler) override;
private:
	NormalView ViewType;
};

std::unique_ptr<NormalIntegrator> CreateNormalIntegrator(NormalView ViewType);