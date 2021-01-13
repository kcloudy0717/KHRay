#pragma once
#include <DirectXMath.h>
#include <embree/rtcore_ray.h>

struct Ray
{
	Ray() = default;
	Ray(const DirectX::XMFLOAT3& Origin, const DirectX::XMFLOAT3& Direction, float Time);

	operator RTCRayHit() const
	{
		RTCRayHit RTCRayHit = {};
		RTCRayHit.ray.org_x = Origin.x;
		RTCRayHit.ray.org_y = Origin.y;
		RTCRayHit.ray.org_z = Origin.z;
		RTCRayHit.ray.dir_x = Direction.x;
		RTCRayHit.ray.dir_y = Direction.y;
		RTCRayHit.ray.dir_z = Direction.z;
		RTCRayHit.ray.tnear = 0;
		RTCRayHit.ray.tfar = INFINITY;
		RTCRayHit.ray.mask = -1;
		RTCRayHit.ray.flags = 0;
		RTCRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		RTCRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
		return RTCRayHit;
	}

	DirectX::XMVECTOR XM_CALLCONV At(float T);

	DirectX::XMFLOAT3 Origin;
	DirectX::XMFLOAT3 Direction;
	float Time;
};