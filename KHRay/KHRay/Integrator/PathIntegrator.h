#pragma once
#include "Integrator.h"

class PathIntegrator : public Integrator
{
public:
	PathIntegrator(int MaxDepth)
		: MaxDepth(MaxDepth)
	{

	}

	Spectrum Li(const Ray& Ray, const Scene& Scene, Sampler& Sampler) override;
private:
	int MaxDepth;
};

std::unique_ptr<PathIntegrator> CreatePathIntegrator(int MaxDepth);