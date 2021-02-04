#include "Scene.h"
#include "Device.h"

using namespace DirectX;

Scene::Scene(const Device& Device)
	: TopLevelAccelerationStructure(Device)
{

}

bool Scene::Intersect(const Ray& Ray, Intersection* pIntersection) const
{
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

	if (pIntersection)
	{
		const auto& hit = RTCRayHit.hit;
		auto Instance = TopLevelAccelerationStructure[hit.geomID];
		auto GeometryDesc = (*Instance.pBLAS)[hit.instID[0]];

		XMMATRIX mMatrix = Instance.Transform.Matrix();

		unsigned int idx0 = GeometryDesc.pIndices[hit.primID * 3 + 0];
		unsigned int idx1 = GeometryDesc.pIndices[hit.primID * 3 + 1];
		unsigned int idx2 = GeometryDesc.pIndices[hit.primID * 3 + 2];

		Vertex vtx0 = GeometryDesc.pVertices[idx0]; vtx0.TransformToWorld(mMatrix);
		Vertex vtx1 = GeometryDesc.pVertices[idx1]; vtx1.TransformToWorld(mMatrix);
		Vertex vtx2 = GeometryDesc.pVertices[idx2]; vtx2.TransformToWorld(mMatrix);

		Vector3f barycentrics = { 1.f - hit.u - hit.v, hit.u, hit.v };
		Vertex Vertex = BarycentricInterpolation(vtx0, vtx1, vtx2, barycentrics);

		pIntersection->Instance = Instance;
		pIntersection->p = Ray.At(RTCRayHit.ray.tfar);
		pIntersection->uv = Vertex.TextureCoordinate;

		auto p0 = vtx0.Position, p1 = vtx1.Position, p2 = vtx2.Position;
		auto e0 = p1 - p0;
		auto e1 = p2 - p0;
		auto Ng = Normalize(Cross(e0, e1));

		pIntersection->geoFrame = ONB(Ng);
		if (GeometryDesc.HasNormals)
		{
			auto Ns = Normalize(Vertex.Normal);
			pIntersection->shFrame = ONB(Ns);
		}	
		else
		{
			pIntersection->shFrame = pIntersection->geoFrame;
		}
	}

	return true;
}