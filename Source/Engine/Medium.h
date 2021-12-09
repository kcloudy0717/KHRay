#pragma once
#include "Spectrum.h"
#include "Sampler/Sampler.h"

struct MediumInteraction;

class IPhaseFunction
{
public:
	virtual ~IPhaseFunction() = default;

	[[nodiscard]] virtual float p(Vector3f wo, Vector3f wi) const noexcept = 0;

	[[nodiscard]] virtual float Sample_p(Vector3f wo, Vector3f* wi, Vector2f Xi) const noexcept = 0;
};

inline float PhaseHG(float cosTheta, float g)
{
	float denom = 1 + g * g + 2 * g * cosTheta;
	return g_1DIV4PI * (1 - g * g) / (denom * std::sqrt(denom));
}

class HenyeyGreenstein : public IPhaseFunction
{
public:
	// HenyeyGreenstein Public Methods
	HenyeyGreenstein(float g)
		: g(g)
	{
	}

	[[nodiscard]] float p(Vector3f wo, Vector3f wi) const noexcept override;

	[[nodiscard]] float Sample_p(Vector3f wo, Vector3f* wi, Vector2f Xi) const noexcept override;

private:
	float g;
};

class IMedium
{
public:
	virtual ~IMedium() = default;

	[[nodiscard]] virtual Spectrum Tr(RayDesc ray, Sampler& sampler) const noexcept = 0;

	[[nodiscard]] virtual Spectrum Sample(RayDesc ray, Sampler& sampler, MediumInteraction* mi) const noexcept = 0;
};

struct MediumInterface
{
	MediumInterface() noexcept = default;
	MediumInterface(const IMedium* medium)
		: inside(medium)
		, outside(medium)
	{
	}
	MediumInterface(const IMedium* inside, const IMedium* outside)
		: inside(inside)
		, outside(outside)
	{
	}

	bool IsMediumTransition() const { return inside != outside; }

	const IMedium* inside  = nullptr;
	const IMedium* outside = nullptr;
};

class HomogeneousMedium : public IMedium
{
public:
	HomogeneousMedium(const Spectrum& sigma_a, const Spectrum& sigma_s, float g)
		: sigma_a(sigma_a)
		, sigma_s(sigma_s)
		, sigma_t(sigma_s + sigma_a)
		, g(g)
	{
	}

	[[nodiscard]] Spectrum Tr(RayDesc ray, Sampler& sampler) const noexcept override;

	[[nodiscard]] Spectrum Sample(RayDesc ray, Sampler& sampler, MediumInteraction* mi) const noexcept override;

private:
	Spectrum sigma_a, sigma_s, sigma_t;
	float	 g;
};
