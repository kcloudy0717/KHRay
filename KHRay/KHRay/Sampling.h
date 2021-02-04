#pragma once
#include "Math.h"

Vector2f UniformSampleDisk(const Vector2f& Xi);

Vector2f ConcentricSampleDisk(const Vector2f& Xi);

Vector3f CosineSampleHemisphere(const Vector2f& Xi);
float CosineHemispherePdf(float cosTheta);