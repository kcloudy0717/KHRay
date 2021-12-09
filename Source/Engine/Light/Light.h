#pragma once
#include "Math/Math.h"
#include "../Spectrum.h"

struct Interaction;
struct VisibilityTester;

struct Light
{
	enum Flags
	{
		DeltaPosition  = 1 << 0,
		DeltaDirection = 1 << 1,
		Area		   = 1 << 2,
		Infinite	   = 1 << 3
	};

	virtual ~Light() = default;

	virtual Spectrum Le(const RayDesc& Ray);

	virtual Spectrum SampleLi(
		const Interaction& Interaction,
		const Vector2f&	   Xi,
		Vector3f*		   pWi,
		float*			   pPdf,
		VisibilityTester*  pVisibilityTester) const = 0;

	bool IsDeltaLight() const { return _Flags & DeltaPosition || _Flags & DeltaDirection; }

	Transform Transform;
	Flags	  _Flags;
};

struct PointLight : Light
{
	PointLight(const Spectrum& I)
		: I(I)
	{
		_Flags = Light::DeltaPosition;
	}

	Spectrum SampleLi(
		const Interaction& Interaction,
		const Vector2f&	   Xi,
		Vector3f*		   pWi,
		float*			   pPdf,
		VisibilityTester*  pVisibilityTester) const override;

	Spectrum I;
};
