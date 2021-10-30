#pragma once
#include "Sampler.h"

class Sobol : public Sampler
{
public:
	Sobol(int SamplesPerPixel, int ResolutionX, int ResolutionY)
		: Sampler(RoundUpPow2(SamplesPerPixel))
	{
		resolution = RoundUpPow2(std::max(ResolutionX, ResolutionY));
		log2Resolution = Log2Int(resolution);

		assert(1 << log2Resolution == resolution);

		sobolIndex = 0;
		dimension = 0;
		scramble = 0;
	}

	std::unique_ptr<Sampler> Clone() const override;

	void StartPixel(int x, int y) override;
	bool StartNextSample() override;

	float Get1D() override;
	Vector2f Get2D() override;
private:
	int64_t GetIndexForSample(int64_t Sample, int x, int y);
private:
	int x, y;

	int resolution;
	int log2Resolution;
	unsigned long long sobolIndex;
	unsigned dimension;
	unsigned scramble;
};