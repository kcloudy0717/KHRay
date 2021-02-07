#pragma once
#include <embree/rtcore_ray.h>
#include "Math.h"

struct Ray
{
	Ray() = default;
	Ray(const Vector3f& Origin,
		float TMin,
		const Vector3f& Direction,
		float TMax,
		float Time = 0.0f)
		: Origin(Origin)
		, TMin(TMin)
		, Direction(Direction)
		, TMax(TMax)
		, Time(Time)
	{

	}

	operator RTCRayHit() const
	{
		RTCRayHit RTCRayHit = {};
		RTCRayHit.ray.org_x = Origin.x;
		RTCRayHit.ray.org_y = Origin.y;
		RTCRayHit.ray.org_z = Origin.z;
		RTCRayHit.ray.dir_x = Direction.x;
		RTCRayHit.ray.dir_y = Direction.y;
		RTCRayHit.ray.dir_z = Direction.z;
		RTCRayHit.ray.tnear = TMin;
		RTCRayHit.ray.tfar = TMax;
		RTCRayHit.ray.mask = -1;
		RTCRayHit.ray.flags = 0;
		RTCRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		RTCRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
		return RTCRayHit;
	}

	Vector3f At(float T) const
	{
		return Origin + Direction * T;
	}

	Vector3f Origin;
	float TMin = 0.0f;
	Vector3f Direction;
	float TMax = INFINITY;
	float Time = 0.0f;
};

// https://github.com/mmp/pbrt-v3/blob/master/src/core/pbrt.h
inline Vector3f OffsetRayOrigin(const Vector3f& p, const Vector3f& pError,
	const Vector3f& n, const Vector3f& w)
{
	float d = Dot(Abs(n), pError);
	Vector3f offset = d * Vector3f(n);
	if (Dot(w, n) < 0)
	{
		offset = -offset;
	}

	Vector3f po = p + offset;

	for (int i = 0; i < 3; ++i)
	{
		if (offset[i] > 0)
		{
			po[i] = NextFloatUp(po[i]);
		}
		else if (offset[i] < 0)
		{
			po[i] = NextFloatDown(po[i]);
		}
	}

	return po;
}