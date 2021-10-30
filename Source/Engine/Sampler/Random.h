#pragma once
#include "Sampler.h"
#include <pcg32/pcg32.h>

class Random : public Sampler
{
public:
	Random(int SamplesPerPixel)
		: Sampler(SamplesPerPixel)
	{

	}

	std::unique_ptr<Sampler> Clone() const override;

	void StartPixel(int x, int y) override;
	bool StartNextSample() override;

	float Get1D() override;
	Vector2f Get2D() override;
private:
	pcg32 RNG;
};