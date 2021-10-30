#pragma once
#include "Math/Math.h"

Vector2f SampleUniformDisk(const Vector2f& Xi);

Vector2f SampleConcentricDisk(const Vector2f& Xi);

Vector3f SampleUniformHemisphere(const Vector2f& Xi);
float	 UniformHemispherePdf();

Vector3f SampleCosineHemisphere(const Vector2f& Xi);
float	 CosineHemispherePdf(float CosTheta);

inline float BalanceHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}
