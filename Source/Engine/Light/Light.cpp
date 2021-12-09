#include "Light.h"
#include "../Scene.h"

Spectrum Light::Le(const RayDesc& Ray)
{
	return Spectrum(0);
}

Spectrum PointLight::SampleLi(
	const Interaction& Interaction,
	const Vector2f&	   Xi,
	Vector3f*		   pWi,
	float*			   pPdf,
	VisibilityTester*  pVisibilityTester) const
{
	Vector3f P(Transform.Position.x, Transform.Position.y, Transform.Position.z);
	*pWi				  = normalize(P - Interaction.p);
	*pPdf				  = 1.0f;
	pVisibilityTester->I0 = Interaction;
	pVisibilityTester->I1 = { P, {}, {}, {} };

	return I / distancesquared(P, Interaction.p);
}
