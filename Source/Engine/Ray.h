#pragma once
#include <embree/rtcore_ray.h>

class IMedium;

struct RayDesc
{
	RayDesc() = default;
	RayDesc(const Vector3f& Origin, float TMin, const Vector3f& Direction, float TMax, const IMedium* Medium = nullptr)
		: Origin(Origin)
		, TMin(TMin)
		, Direction(Direction)
		, TMax(TMax)
		, Medium(Medium)
	{
	}

	operator RTCRay() const
	{
		RTCRay RTCRay = {};
		RTCRay.org_x  = Origin.x;
		RTCRay.org_y  = Origin.y;
		RTCRay.org_z  = Origin.z;
		RTCRay.tnear  = TMin;

		RTCRay.dir_x = Direction.x;
		RTCRay.dir_y = Direction.y;
		RTCRay.dir_z = Direction.z;
		RTCRay.time	 = 0.0f;

		RTCRay.tfar	 = TMax;
		RTCRay.mask	 = -1;
		RTCRay.id	 = 0;
		RTCRay.flags = 0;

		return RTCRay;
	}

	operator RTCRayHit() const
	{
		RTCRayHit RTCRayHit = {};
		RTCRayHit.ray.org_x = Origin.x;
		RTCRayHit.ray.org_y = Origin.y;
		RTCRayHit.ray.org_z = Origin.z;
		RTCRayHit.ray.tnear = TMin;

		RTCRayHit.ray.dir_x = Direction.x;
		RTCRayHit.ray.dir_y = Direction.y;
		RTCRayHit.ray.dir_z = Direction.z;
		RTCRayHit.ray.time	= 0.0f;

		RTCRayHit.ray.tfar	= TMax;
		RTCRayHit.ray.mask	= -1;
		RTCRayHit.ray.id	= 0;
		RTCRayHit.ray.flags = 0;

		RTCRayHit.hit.geomID	= RTC_INVALID_GEOMETRY_ID;
		RTCRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
		return RTCRayHit;
	}

	Vector3f At(float T) const { return Origin + Direction * T; }

	Vector3f	   Origin;
	float		   TMin = 0.0f;
	Vector3f	   Direction;
	mutable float  TMax	  = INFINITY;
	const IMedium* Medium = nullptr;
};
