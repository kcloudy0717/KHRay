#include "AccelerationStructure.h"

#include <DirectXMath.h>
using namespace DirectX;

#include "WaveFrontReader.h"
#include "Utility.h"

AccelerationStructure::AccelerationStructure(RTCDevice Device)
	: Device(Device)
{
	Scene = rtcNewScene(Device);
}

AccelerationStructure::~AccelerationStructure()
{
	if (Scene)
	{
		rtcReleaseScene(Scene);
		Scene = nullptr;
	}
}

AccelerationStructure::operator RTCScene() const
{
	return Scene;
}

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(RTCDevice Device)
	: AccelerationStructure(Device)
{

}

void BottomLevelAccelerationStructure::AddGeometry(const std::filesystem::path& Path)
{
	WaveFrontReader<unsigned int> Reader;
	if (SUCCEEDED(Reader.Load(Path.c_str())))
	{
		auto Geometry = rtcNewGeometry(Device, RTC_GEOMETRY_TYPE_TRIANGLE);

		/*
		* Create a triangle mesh geometry
		* You can look up geometry types in the API documentation to
		* find out which type expects which buffers.
		*
		* We create buffers directly on the device, but you can also use
		* shared buffers. For shared buffers, special care must be taken
		* to ensure proper alignment and padding. This is described in
		* more detail in the API documentation.
		*/

		auto pVertices = reinterpret_cast<Vertex*>(rtcSetNewGeometryBuffer(Geometry,
			RTC_BUFFER_TYPE_VERTEX,
			0,
			RTC_FORMAT_FLOAT3,
			sizeof(Vertex),
			Reader.vertices.size()));

		assert(Reader.indices.size() % 3 == 0);
		size_t NumTriangles = Reader.indices.size() / 3;
		auto pIndices = reinterpret_cast<unsigned*>(rtcSetNewGeometryBuffer(Geometry,
			RTC_BUFFER_TYPE_INDEX,
			0,
			RTC_FORMAT_UINT3,
			sizeof(unsigned int) * 3,
			NumTriangles));

		if (pVertices && pIndices)
		{
			for (auto [i, v] : enumerate(Reader.vertices))
			{
				pVertices[i].Position = v.position;
				pVertices[i].Normal = v.normal;
			}

			for (auto [i, index] : enumerate(Reader.indices))
			{
				pIndices[i] = index;
			}
		}

		/*
		 * You must commit geometry objects when you are done setting them up,
		 * or you will not get any intersections.
		 */
		rtcCommitGeometry(Geometry);

		/*
		 * In rtcAttachGeometry(...), the scene takes ownership of the geom
		 * by increasing its reference count. This means that we don't have
		 * to hold on to the geom handle, and may release it. The geom object
		 * will be released automatically when the scene is destroyed.
		 *
		 * rtcAttachGeometry() returns a geometry ID. We could use this to
		 * identify intersected objects later on.
		 */
		rtcAttachGeometry(Scene, Geometry);
		rtcReleaseGeometry(Geometry);

		RAYTRACING_GEOMETRY_DESC GeometryDesc = {};
		GeometryDesc.pVertices = pVertices;
		GeometryDesc.pIndices = pIndices;
		GeometryDesc.NumVertices = Reader.vertices.size();
		GeometryDesc.NumIndices = Reader.indices.size();

		GeometryDescs.push_back(GeometryDesc);
		Geometries.push_back(Geometry);
		NumGeometries++;
	}
}

void BottomLevelAccelerationStructure::Generate()
{
	rtcCommitScene(Scene);
}

TopLevelAccelerationStructure::TopLevelAccelerationStructure(RTCDevice Device)
	: AccelerationStructure(Device)
{

}

void TopLevelAccelerationStructure::AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc)
{
	auto InstanceGeometry = rtcNewGeometry(Device, RTC_GEOMETRY_TYPE_INSTANCE);

	rtcSetGeometryInstancedScene(InstanceGeometry, *Desc.pBLAS);
	rtcAttachGeometry(Scene, InstanceGeometry);
	rtcReleaseGeometry(InstanceGeometry);

	/*
	* In rtcAttachGeometry(...), the scene takes ownership of the geom
	* by increasing its reference count. This means that we don't have
	* to hold on to the geom handle, and may release it. The geom object
	* will be released automatically when the scene is destroyed.
	*
	* rtcAttachGeometry() returns a geometry ID. We could use this to
	* identify intersected objects later on.
	*/
	InstanceDescs.push_back(Desc);
	InstanceGeometries.push_back(InstanceGeometry);
	NumInstances++;
}

void TopLevelAccelerationStructure::Generate()
{
	for (size_t i = 0; i < NumInstances; ++i)
	{
		auto& Instance = InstanceDescs[i];
		auto& InstanceGeometry = InstanceGeometries[i];

		XMMATRIX mMatrix = Instance.Transform.Matrix();
		XMFLOAT3X4 Matrix;
		XMStoreFloat3x4(&Matrix, mMatrix);

		rtcSetGeometryTransform(InstanceGeometry, 0, RTC_FORMAT_FLOAT3X4_ROW_MAJOR, reinterpret_cast<float*>(&Matrix));
		rtcCommitGeometry(InstanceGeometry);
	}

	/*
	* Like geometry objects, scenes must be committed. This lets
	* Embree know that it may start building an acceleration structure.
	*/
	rtcCommitScene(Scene);
}