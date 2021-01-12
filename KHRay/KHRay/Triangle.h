#pragma once
#include "Vertex.h"

struct Triangle
{
	Triangle(Vertex V0, Vertex V1, Vertex V2)
		: V0(V0), V1(V1), V2(V2)
	{

	}

	Vertex V0;
	Vertex V1;
	Vertex V2;
};

bool RayIntersectsTriangle(Vector3f RayOrigin,
	Vector3f RayDirection,
	const Triangle& Triangle,
	Vector3f& IntersectionPoint)
{
	const float EPSILON = 0.0000001;
	Vector3f vertex0 = Triangle.V0.Position;
	Vector3f vertex1 = Triangle.V1.Position;
	Vector3f vertex2 = Triangle.V2.Position;
	Vector3f edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = Cross(RayDirection, edge2);
	a = Dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false;    // This ray is parallel to this triangle.
	f = 1.0 / a;
	s = RayOrigin - vertex0;
	u = f * Dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;
	q = Cross(s, edge1);
	v = f * Dot(RayDirection, q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * Dot(edge2, q);
	if (t > EPSILON) // ray intersection
	{
		IntersectionPoint = RayOrigin + RayDirection * t;
		return true;
	}

	// This means that there is a line intersection but not a ray intersection.
	return false;
}