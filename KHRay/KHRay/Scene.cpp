#include "Scene.h"
#include "Device.h"

Ray Interaction::SpawnRay(const Vector3f& d) const
{
	return Ray(p, 0.01f, Normalize(d), INFINITY);
}

Ray Interaction::SpawnRayTo(const Interaction& Interaction) const
{
	Vector3f d = Interaction.p - p;
	float tmax = d.Length();
	d = Normalize(d);

	return Ray(p, 0.01f, d, tmax);
}

bool VisibilityTester::Unoccluded(const Scene& Scene) const
{
	return !Scene.Intersect(I0.SpawnRayTo(I1), nullptr);
}

Scene::Scene(const Device& Device)
	: TopLevelAccelerationStructure(Device)
{

}

bool Scene::Intersect(const Ray& Ray, SurfaceInteraction* pSurfaceInteraction) const
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

	if (pSurfaceInteraction)
	{
		using namespace DirectX;

		const auto& hit = RTCRayHit.hit;
		auto Instance = TopLevelAccelerationStructure[hit.geomID];
		auto GeometryDesc = (*Instance.pBLAS)[hit.instID[0]];

		XMMATRIX mMatrix = Instance.Transform.Matrix();

		unsigned int idx0 = GeometryDesc.pIndices[hit.primID * 3 + 0];
		unsigned int idx1 = GeometryDesc.pIndices[hit.primID * 3 + 1];
		unsigned int idx2 = GeometryDesc.pIndices[hit.primID * 3 + 2];

		// Fectch vertices and transform them into instance's Transform
		Vertex vtx0 = GeometryDesc.pVertices[idx0]; vtx0.TransformToWorld(mMatrix);
		Vertex vtx1 = GeometryDesc.pVertices[idx1]; vtx1.TransformToWorld(mMatrix);
		Vertex vtx2 = GeometryDesc.pVertices[idx2]; vtx2.TransformToWorld(mMatrix);

		auto p0 = vtx0.Position, p1 = vtx1.Position, p2 = vtx2.Position;
		// Compute 2 edges of the triangle
		auto e0 = p1 - p0;
		auto e1 = p2 - p0;
		auto n = Normalize(Cross(e0, e1));

		Vector3f barycentrics = { 1.f - hit.u - hit.v, hit.u, hit.v };
		Vertex interpolatedV = BarycentricInterpolation(vtx0, vtx1, vtx2, barycentrics);

		pSurfaceInteraction->p = Ray.At(RTCRayHit.ray.tfar);
		pSurfaceInteraction->wo = -Ray.Direction;
		pSurfaceInteraction->n = n;
		pSurfaceInteraction->uv = interpolatedV.TextureCoordinate;

		pSurfaceInteraction->Instance = Instance;

		// Compute geometry basis and shading basis
		pSurfaceInteraction->GeometryBasis = pSurfaceInteraction->ShadingBasis = OrthonormalBasis(n);

		if (GeometryDesc.HasNormals)
		{
			auto Ns = Normalize(interpolatedV.Normal);
			pSurfaceInteraction->ShadingBasis = OrthonormalBasis(Ns);
		}

		// Update BSDF's internal data
		pSurfaceInteraction->BSDF = pSurfaceInteraction->Instance.pBSDF->Clone();
		pSurfaceInteraction->BSDF.SetInteraction(*pSurfaceInteraction);
	}

	return true;
}