#pragma once
#include "AccelerationStructure.h"

enum class InteractionType
{
	Surface,
	Medium
};

struct Interaction
{
	Interaction() = default;
	Interaction(const Vector3f& p, const Vector3f& wo, const Vector3f& n, const MediumInterface& mediumInterface)
		: p(p)
		, wo(wo)
		, n(n)
		, mediumInterface(mediumInterface)
	{
	}

	bool IsSurfaceInteraction() const noexcept { return Type == InteractionType::Surface; }

	const IMedium* GetMedium(const Vector3f& w) const noexcept
	{
		return dot(w, n) > 0.0f ? mediumInterface.outside : mediumInterface.inside;
	}

	RayDesc SpawnRay(const Vector3f& d) const;
	RayDesc SpawnRayTo(const Interaction& Interaction) const;

	InteractionType Type = InteractionType::Surface;

	Vector3f		p; // Hit point
	Vector3f		wo;
	Vector3f		n; // Normal
	MediumInterface mediumInterface;
};

struct SurfaceInteraction : Interaction
{
	using Interaction::Interaction;

	RAYTRACING_INSTANCE_DESC Instance;
	Vector2f				 uv; // Texture coord
	Frame					 GeometryFrame;
	Frame					 ShadingFrame;
	BSDF					 BSDF;
};

struct MediumInteraction : Interaction
{
	MediumInteraction() noexcept = default;
	MediumInteraction(
		const Vector3f&					  p,
		const Vector3f&					  wo,
		const IMedium*					  medium,
		std::unique_ptr<IPhaseFunction>&& phase)
		: Interaction(p, wo, Vector3f(0.0f), medium)
		, phase(std::move(phase))
	{
		Type = InteractionType::Medium;
	}

	[[nodiscard]] bool IsValid() const noexcept { return static_cast<bool>(phase); }

	std::unique_ptr<IPhaseFunction> phase;
};
