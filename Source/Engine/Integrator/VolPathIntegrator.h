#pragma once
#include "Integrator.h"

class VolPathIntegrator : public Integrator
{
public:
	VolPathIntegrator(int MaxDepth, float rrThreshold = 1.0f)
		: MaxDepth(MaxDepth)
		, rrThreshold(rrThreshold)
	{
	}

	Spectrum Li(RayDesc ray, const Scene& scene, Sampler& sampler) override;

private:
	int	  MaxDepth;
	float rrThreshold;
};

std::unique_ptr<VolPathIntegrator> CreateVolPathIntegrator(int MaxDepth);
