#include "Ray.h"

using namespace DirectX;

Ray::Ray(const DirectX::XMFLOAT3& Origin, const DirectX::XMFLOAT3& Direction, float Time)
	: Origin(Origin), Direction(Direction), Time(Time)
{

}

DirectX::XMVECTOR XM_CALLCONV Ray::At(float T)
{
	auto vOrigin = XMLoadFloat3(&this->Origin);
	auto vDirection = XMLoadFloat3(&this->Direction);

	return vOrigin + vDirection * T;
}