#pragma once
#include "Integrator.h"

class AOIntegrator : public Integrator
{
public:
	AOIntegrator(int NumSamples)
		: NumSamples(NumSamples)
	{

	}

	Spectrum Li(Ray ray, const Scene& scene, Sampler& sampler) override;
private:
	int NumSamples;
};

std::unique_ptr<AOIntegrator> CreateAOIntegrator(int NumSamples);