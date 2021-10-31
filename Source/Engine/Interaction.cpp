#include "Interaction.h"

constexpr float ShadowEpsilon = 0.0001f;

Ray Interaction::SpawnRay(const Vector3f& d) const
{
	return Ray(p, 0.0001f, Normalize(d), INFINITY);
}

Ray Interaction::SpawnRayTo(const Interaction& Interaction) const
{
	Vector3f d	  = Interaction.p - p;
	float	 tmax = d.Length();
	d			  = Normalize(d);

	return Ray(p, 0.0001f, d, tmax - ShadowEpsilon);
}
