#pragma once
#include <memory>
#include "../Math.h"
#include "../Sampling.h"

class Sampler
{
public:
	Sampler(int NumSamplesPerPixel)
		: CurrentPixelSample(0)
		, NumSamplesPerPixel(NumSamplesPerPixel)
	{

	}

	virtual ~Sampler() = default;

	int GetNumSamplesPerPixel() const
	{
		return NumSamplesPerPixel;
	}

	// Create an exact clone of the current Sampler instance.
	virtual std::unique_ptr<Sampler> Clone() const = 0;

	/*
	* Prepares the sampler to render a new film tile
	* This function is called when the sampler begins rendering a new film tile
	* This can be used to determinstically initialize the sampler so that repeated program runs
	* always create the same image.
	*/
	virtual void StartPixel(int x, int y);
	virtual bool StartNextSample();

	virtual float Get1D() = 0;
	virtual Vector2f Get2D() = 0;
protected:
	int CurrentPixelSample;
	int NumSamplesPerPixel;
};