#pragma once
#include <embree/rtcore.h>
#include <filesystem>
#include <vector>
#include "Transform.h"
#include "Vertex.h"

struct BSDF;

class AccelerationStructure
{
public:
	AccelerationStructure(RTCDevice Device);
	~AccelerationStructure();

	operator RTCScene() const;
protected:
	RTCDevice Device;
	RTCScene Scene;
};

struct RAYTRACING_GEOMETRY_DESC
{
	Vertex* pVertices;
	unsigned int* pIndices;
	size_t NumVertices;
	size_t NumIndices;
	bool HasNormals;
	bool HasTextureCoordinates;
};

class BottomLevelAccelerationStructure : public AccelerationStructure
{
public:
	BottomLevelAccelerationStructure(RTCDevice Device);

	RAYTRACING_GEOMETRY_DESC operator[](size_t i) const
	{
		return GeometryDescs[i];
	}

	void AddGeometry(const std::filesystem::path& Path);

	void Generate();
private:
	size_t NumGeometries = 0;
	std::vector<RAYTRACING_GEOMETRY_DESC> GeometryDescs;
	std::vector<RTCGeometry> Geometries;
};

struct RAYTRACING_INSTANCE_DESC
{
	Transform Transform;
	BSDF* pBSDF;
	BottomLevelAccelerationStructure* pBLAS;
};

class TopLevelAccelerationStructure : public AccelerationStructure
{
public:
	TopLevelAccelerationStructure(RTCDevice Device);

	RAYTRACING_INSTANCE_DESC operator[](size_t i) const
	{
		return InstanceDescs[i];
	}

	void AddBottomLevelAccelerationStructure(const RAYTRACING_INSTANCE_DESC& Desc);

	void Generate();
private:
	size_t NumInstances = 0;
	std::vector<RAYTRACING_INSTANCE_DESC> InstanceDescs;
	std::vector<RTCGeometry> InstanceGeometries;
};