#pragma once
#include "../Ray.h"
#include "../Spectrum.h"

struct Light
{
	virtual ~Light() = default;

	virtual Spectrum Le(const Ray& Ray);
};