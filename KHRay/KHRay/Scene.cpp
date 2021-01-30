#include "Scene.h"
#include "Device.h"

using namespace DirectX;

Scene::Scene(const Device& Device)
	: TopLevelAccelerationStructure(Device)
{
	
}

bool Scene::Intersect(const Ray& Ray, Intersection* pIntersection) const
{
	assert(pIntersection);

	/*
	* The intersect context can be used to set intersection
	* filters or flags, and it also contains the instance ID stack
	* used in multi-level instancing.
	*/
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	/*
	* The ray hit structure holds both the ray and the hit.
	* The user must initialize it properly -- see API documentation
	* for rtcIntersect1() for details.
	*/
	RTCRayHit RTCRayHit = Ray;

	/*
	* There are multiple variants of rtcIntersect. This one
	* intersects a single ray with the scene.
	*/
	rtcIntersect1(TopLevelAccelerationStructure, &context, &RTCRayHit);

	if (RTCRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		return false;
	}

	const auto& hit = RTCRayHit.hit;
	auto Instance = TopLevelAccelerationStructure[hit.geomID];
	auto GeometryDesc = (*Instance.pBLAS)[hit.instID[0]];

	const unsigned int idx0 = GeometryDesc.pIndices[hit.primID * 3 + 0];
	const unsigned int idx1 = GeometryDesc.pIndices[hit.primID * 3 + 1];
	const unsigned int idx2 = GeometryDesc.pIndices[hit.primID * 3 + 2];

	const Vertex& vtx0 = GeometryDesc.pVertices[idx0];
	const Vertex& vtx1 = GeometryDesc.pVertices[idx1];
	const Vertex& vtx2 = GeometryDesc.pVertices[idx2];

	XMFLOAT3 barycentrics = { 1.f - hit.u - hit.v, hit.u, hit.v };
	Vertex Vertex = BarycentricInterpolation(vtx0, vtx1, vtx2, barycentrics);

	pIntersection->Instance = Instance;
	pIntersection->Position = Vertex.Position;
	pIntersection->Normal = Vertex.Normal;
	pIntersection->TextureCoordinate = Vertex.TextureCoordinate;

	return true;
}