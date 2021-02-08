#pragma once
#include "Math.h"

Vector2f UniformSampleDisk(const Vector2f& Xi);

Vector2f ConcentricSampleDisk(const Vector2f& Xi);

Vector3f UniformSampleHemisphere(const Vector2f& Xi);
float UniformHemispherePdf();

Vector3f CosineSampleHemisphere(const Vector2f& Xi);
float CosineHemispherePdf(float cosTheta);

inline float BalanceHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}