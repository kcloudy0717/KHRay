#include "Sampler.h"

void Sampler::StartPixel(int x, int y)
{
	CurrentPixelSample = 0;
}

bool Sampler::StartNextSample()
{
	return ++CurrentPixelSample < NumSamplesPerPixel;
}
