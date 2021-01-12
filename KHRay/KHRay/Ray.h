#pragma once

#include "Vector3.h"

struct Ray
{
	Ray(Vector3f Origin, Vector3f Direction, float TMax = INFINITY)
		: Origin(Origin), Direction(Direction), TMax(TMax)
	{

	}

	Vector3f At(float T)
	{
		return Origin + Direction * T;
	}

	Vector3f Origin;
	Vector3f Direction;
	float TMax;
};