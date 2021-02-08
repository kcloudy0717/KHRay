#include "Sampling.h"

Vector2f UniformSampleDisk(const Vector2f& Xi)
{
	float radius = std::sqrt(Xi.x);
	float theta = g_2PI * Xi.y;

	return { radius * std::cos(theta), radius * std::sin(theta) };
}

Vector2f ConcentricSampleDisk(const Vector2f& Xi)
{
	// Map Xi to $[-1,1]^2$
	auto XiOffset = 2.0f * Xi - 1.0f;

	// Handle degeneracy at the origin
	if (XiOffset.x == 0.0f && XiOffset.y == 0.0f)
	{
		return { 0.0f, 0.0f };
	}

	// Apply concentric mapping to point
	float radius, theta;
	if (std::abs(XiOffset.x) > std::abs(XiOffset.y))
	{
		radius = XiOffset.x;
		theta = g_PIDIV4 * (XiOffset.y / XiOffset.x);
	}
	else
	{
		radius = XiOffset.y;
		theta = g_PIDIV2 - g_PIDIV4 * (XiOffset.x / XiOffset.y);
	}

	return { radius * std::cos(theta), radius * std::sin(theta) };
}

Vector3f UniformSampleHemisphere(const Vector2f& Xi)
{
	float z = Xi[0];
	float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
	float phi = 2.0f * g_PI * Xi[1];
	return { r * std::cos(phi), r * std::sin(phi), z };
}

float UniformHemispherePdf()
{
	return g_1DIV2PI;
}

Vector3f CosineSampleHemisphere(const Vector2f& Xi)
{
	auto p = ConcentricSampleDisk(Xi);
	float z = std::sqrt(std::max(0.0f, 1.0f - p.x * p.x - p.y * p.y));
	return { p.x, p.y, z };
}

float CosineHemispherePdf(float cosTheta)
{
	return cosTheta * g_1DIVPI;
}