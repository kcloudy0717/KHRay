#pragma once
#include "Integrator.h"

class PathIntegrator : public Integrator
{
public:
	PathIntegrator(int MaxDepth, float rrThreshold = 1.0f)
		: MaxDepth(MaxDepth)
		, rrThreshold(rrThreshold)
	{

	}

	Spectrum Li(Ray ray, const Scene& scene, Sampler& sampler) override;
private:
	int MaxDepth;
	float rrThreshold;
};

std::unique_ptr<PathIntegrator> CreatePathIntegrator(int MaxDepth);