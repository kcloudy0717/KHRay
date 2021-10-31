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

	Ray SpawnRay(const Vector3f& d) const;
	Ray SpawnRayTo(const Interaction& Interaction) const;

	InteractionType Type;

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
		const Vector3f& p,
		const Vector3f& wo,
		const Vector3f& n,
		const IMedium*	medium,
		IPhaseFunction* phase)
		: Interaction(p, wo, n, medium)
		, phase(phase)
	{
	}

	[[nodiscard]] bool IsValid() const noexcept { return phase; }

	IPhaseFunction* phase = nullptr;
};
