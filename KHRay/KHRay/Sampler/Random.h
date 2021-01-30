#pragma once
#include "Sampler.h"
#include <pcg32/pcg32.h>

class Random : public Sampler
{
public:
	Random(size_t SamplesPerPixel)
		: Sampler(SamplesPerPixel)
	{

	}

	std::unique_ptr<Sampler> Clone() const override;

	void StartPixel(int x, int y) override;

	float Get1D() override;
	DirectX::XMFLOAT2 Get2D() override;
private:
	pcg32 RNG;
};