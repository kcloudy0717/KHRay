#include "AccelerationStructure.h"

#include "Utility.h"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

using namespace DirectX;

static Assimp::Importer Importer;

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
	const aiScene* paiScene = Importer.ReadFile(Path.string(),
		aiProcess_ConvertToLeftHanded |
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph);

	if (paiScene)
	{
		for (unsigned int m = 0; m < paiScene->mNumMeshes; ++m)
		{
			const aiMesh* paiMesh = paiScene->mMeshes[m];

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
				paiMesh->mNumVertices));

			auto pIndices = reinterpret_cast<unsigned*>(rtcSetNewGeometryBuffer(Geometry,
				RTC_BUFFER_TYPE_INDEX,
				0,
				RTC_FORMAT_UINT3,
				sizeof(unsigned int) * 3,
				paiMesh->mNumFaces));

			if (pVertices && pIndices)
			{
				auto indices = pIndices;

				for (unsigned int v = 0; v < paiMesh->mNumVertices; ++v)
				{
					Vertex vertex = {};
					// Position
					vertex.Position = Vector3f(paiMesh->mVertices[v].x, paiMesh->mVertices[v].y, paiMesh->mVertices[v].z);

					// Texture coordinates
					if (paiMesh->HasTextureCoords(0))
					{
						vertex.TextureCoordinate = Vector2f(paiMesh->mTextureCoords[0][v].x, paiMesh->mTextureCoords[0][v].y);
					}

					// Normal
					if (paiMesh->HasNormals())
					{
						vertex.Normal = Vector3f(paiMesh->mNormals[v].x, paiMesh->mNormals[v].y, paiMesh->mNormals[v].z);
					}

					pVertices[v] = vertex;
				}

				for (unsigned int f = 0; f < paiMesh->mNumFaces; ++f)
				{
					const aiFace& aiFace = paiMesh->mFaces[f];
					assert(aiFace.mNumIndices == 3);

					indices[0] = aiFace.mIndices[0];
					indices[1] = aiFace.mIndices[1];
					indices[2] = aiFace.mIndices[2];

					indices += 3;
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
			GeometryDesc.Name = paiMesh->mName.C_Str();
			GeometryDesc.pVertices = pVertices;
			GeometryDesc.pIndices = pIndices;
			GeometryDesc.HasNormals = paiMesh->HasNormals();
			GeometryDesc.HasTextureCoordinates = paiMesh->HasTextureCoords(0);

			auto paiMaterial = paiScene->mMaterials[paiMesh->mMaterialIndex];
			aiColor3D color(0.0f, 0.0f, 0.0f);
			paiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);

			if (color.IsBlack())
			{
				color.r = color.g = color.b = 1.0f;
			}

			GeometryDesc.BSDF.SetBxDF(std::make_shared<LambertianReflection>(Spectrum(color.r, color.g, color.b)));

			GeometryDescs.push_back(GeometryDesc);
			Geometries.push_back(Geometry);
			NumGeometries++;
		}
	}
	else
	{
		throw std::exception("File DNE");
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