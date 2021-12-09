#include "Interaction.h"

constexpr float ShadowEpsilon = 0.0001f;

RayDesc Interaction::SpawnRay(const Vector3f& d) const
{
	return RayDesc(p, 0.0001f, normalize(d), INFINITY, GetMedium(d));
}

RayDesc Interaction::SpawnRayTo(const Interaction& Interaction) const
{
	Vector3f d	  = Interaction.p - p;
	float	 tmax = d.Length();
	d			  = normalize(d);

	return RayDesc(p, 0.0001f, d, tmax - ShadowEpsilon, GetMedium(d));
}
