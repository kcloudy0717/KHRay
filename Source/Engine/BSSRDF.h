#pragma once
#include "Scene.h"

class BSSRDF
{
public:
	BSSRDF(const SurfaceInteraction& po, float eta)
		: po(po)
		, eta(eta)
	{
	}

	[[nodiscard]] virtual Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi) = 0;

private:
	SurfaceInteraction po;
	float			   eta;
};
