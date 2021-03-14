#include "Random.h"

std::unique_ptr<Sampler> Random::Clone() const
{
	return std::make_unique<Random>(*this);
}

void Random::StartPixel(int x, int y)
{
	RNG.seed(x, y);
	return Sampler::StartPixel(x, y);
}

bool Random::StartNextSample()
{
	return Sampler::StartNextSample();
}

float Random::Get1D()
{
	return RNG.nextFloat();
}

Vector2f Random::Get2D()
{
	return { RNG.nextFloat(), RNG.nextFloat() };
}