#include "Scene.h"

bool VisibilityTester::Unoccluded(const Scene& Scene) const
{
	RayDesc shadowRay = I0.SpawnRayTo(I1);

	return !Scene.Occluded(shadowRay);
}

Spectrum VisibilityTester::Tr(const Scene& Scene, Sampler& sampler) const
{
	RayDesc		 ray = I0.SpawnRayTo(I1);
	Spectrum Tr(1.f);
	while (true)
	{
		std::optional<SurfaceInteraction> isect = Scene.Intersect(ray);
		// Handle opaque surface along ray's path
		if (isect && isect->BSDF)
		{
			return Spectrum(0.0f);
		}

		// Update transmittance for current ray segment
		if (ray.Medium)
		{
			Tr *= ray.Medium->Tr(ray, sampler);
		}

		// Generate next ray segment or return final transmittance
		if (!isect)
			break;
		ray = isect->SpawnRayTo(I1);
	}
	return Tr;
}

Scene::Scene(const RTXDevice& Device)
	: TopLevelAccelerationStructure(Device)
{
	rtcSetSceneBuildQuality(TopLevelAccelerationStructure, RTC_BUILD_QUALITY_HIGH);
	rtcSetSceneFlags(TopLevelAccelerationStructure, RTC_SCENE_FLAG_ROBUST);
}

std::optional<SurfaceInteraction> Scene::Intersect(const RayDesc& Ray) const
{
	/*
	 * The intersect context can be used to set intersection
	 * filters or flags, and it also contains the instance ID stack
	 * used in multi-level instancing.
	 */
	RTCIntersectContext RTCIntersectContext;
	rtcInitIntersectContext(&RTCIntersectContext);

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
	rtcIntersect1(TopLevelAccelerationStructure, &RTCIntersectContext, &RTCRayHit);

	if (RTCRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		return {};
	}

	Ray.TMax = RTCRayHit.ray.tfar;

	const auto& hit			 = RTCRayHit.hit;
	auto		Instance	 = TopLevelAccelerationStructure[hit.instID[0]];
	const auto& GeometryDesc = (*Instance.BLAS)[hit.geomID];

	DirectX::XMMATRIX mMatrix = Instance.Transform.Matrix();

	// Fetch indices
	unsigned int idx0 = GeometryDesc.pIndices[hit.primID * 3 + 0];
	unsigned int idx1 = GeometryDesc.pIndices[hit.primID * 3 + 1];
	unsigned int idx2 = GeometryDesc.pIndices[hit.primID * 3 + 2];

	// Fetch vertices
	Vertex vtx0 = GeometryDesc.pVertices[idx0];
	vtx0.TransformToWorld(mMatrix);
	Vertex vtx1 = GeometryDesc.pVertices[idx1];
	vtx1.TransformToWorld(mMatrix);
	Vertex vtx2 = GeometryDesc.pVertices[idx2];
	vtx2.TransformToWorld(mMatrix);

	Vector3f p0 = vtx0.Position, p1 = vtx1.Position, p2 = vtx2.Position;
	// Compute 2 edges of the triangle
	Vector3f e0 = p1 - p0;
	Vector3f e1 = p2 - p0;
	Vector3f n	= normalize(cross(e0, e1));

	Vector3f barycentrics = { 1.f - hit.u - hit.v, hit.u, hit.v };
	Vertex	 vertex		  = BarycentricInterpolation(vtx0, vtx1, vtx2, barycentrics);

	SurfaceInteraction si = {};
	si.p				  = Ray.At(RTCRayHit.ray.tfar);
	si.wo				  = -Ray.Direction;
	si.n				  = n;
	if (GeometryDesc.MediumInterface.IsMediumTransition())
	{
		si.mediumInterface = GeometryDesc.MediumInterface;
	}
	else
	{
		si.mediumInterface = Ray.Medium;
	}
	si.uv = vertex.TextureCoordinate;

	si.Instance = Instance;

	// Compute geometry basis and shading basis
	si.GeometryFrame = si.ShadingFrame = Frame(n);

	if (GeometryDesc.HasNormals)
	{
		Vector3f Ns		= normalize(vertex.Normal);
		si.ShadingFrame = Frame(Ns);
	}

	// Update BSDF's internal data
	si.BSDF = GeometryDesc.BSDF.Clone();
	si.BSDF.SetInteraction(si);

	return si;
}

bool Scene::Occluded(const RayDesc& Ray) const
{
	RTCIntersectContext RTCIntersectContext;
	rtcInitIntersectContext(&RTCIntersectContext);

	RTCRay RTCRay = Ray;

	// This function sets RTCRay::tfar to -inf if intersection was found
	rtcOccluded1(TopLevelAccelerationStructure, &RTCIntersectContext, &RTCRay);

	return RTCRay.tfar == -std::numeric_limits<float>::infinity();
}

bool Scene::IntersectTr(RayDesc ray, Sampler& sampler, Spectrum* OutTr)
{
	Spectrum Tr;
	while (true)
	{
		auto isect = Intersect(ray);
		if (ray.Medium)
		{
			Tr *= ray.Medium->Tr(ray, sampler);
		}

		if (!isect)
		{
			return false;
		}
		if (isect->BSDF)
		{
			return true;
		}

		ray = isect->SpawnRay(ray.Direction);
	}
}

void Scene::AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc)
{
	TopLevelAccelerationStructure.AddBottomLevelAccelerationStructure(Desc);
}

void Scene::AddLight(Light* pLight)
{
	Lights.push_back(pLight);
}

void Scene::Generate()
{
	TopLevelAccelerationStructure.Generate();
}
