#pragma once
#include "Integrator.h"

enum class SamplingStrategy
{
	Uniform,
	Cosine
};

class AOIntegrator : public Integrator
{
public:
	AOIntegrator(int NumSamples, SamplingStrategy Strategy = SamplingStrategy::Cosine)
		: NumSamples(NumSamples)
		, Strategy(Strategy)
	{

	}

	Spectrum Li(RayDesc ray, const Scene& scene, Sampler& sampler) override;
private:
	int NumSamples;
	SamplingStrategy Strategy;
};

std::unique_ptr<AOIntegrator> CreateAOIntegrator(int NumSamples, SamplingStrategy Strategy = SamplingStrategy::Cosine);