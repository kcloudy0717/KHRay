#include "Random.h"

std::unique_ptr<Sampler> Random::Clone() const
{
	auto clone = std::make_unique<Random>(NumSamplesPerPixel);
	clone->RNG = this->RNG;
	return clone;
}

void Random::StartPixel(int x, int y)
{
	RNG.seed(x, y);
}

float Random::Get1D()
{
	return RNG.nextFloat();
}

Vector2f Random::Get2D()
{
	return { RNG.nextFloat(), RNG.nextFloat() };
}