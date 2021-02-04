#include "Camera.h"

using namespace DirectX;

Ray Camera::GetRay(float U, float V) const
{
	//const float h = tanf(VerticalFOV * 0.5f);
	const float viewportHeight = 2.0f; //* h;
	const float viewportWidth = AspectRatio * viewportHeight;

	XMVECTOR vPosition = XMLoadFloat3(&Transform.Position);

	XMVECTOR vU = Transform.Right();
	XMVECTOR vV = Transform.Up();
	XMVECTOR vW = Transform.Forward();

	XMVECTOR vHorizontal = viewportWidth * vU;
	XMVECTOR vVertical = viewportHeight * vV;
	XMVECTOR vLowerLeftCorner = vPosition - vHorizontal * 0.5f - vVertical * 0.5f + FocalLength * vW;

	XMVECTOR vDirection = XMVector3Normalize(vLowerLeftCorner + U * vHorizontal + V * vVertical - vPosition);

	XMFLOAT3 rayOrigin, rayDirection;
	XMStoreFloat3(&rayOrigin, vPosition);
	XMStoreFloat3(&rayDirection, vDirection);

	return Ray({ rayOrigin.x, rayOrigin.y, rayOrigin.z },
		0.0f,
		{ rayDirection.x, rayDirection.y, rayDirection.z },
		INFINITY,
		1);
}

void Camera::SetLookAt(DirectX::FXMVECTOR EyePosition, DirectX::FXMVECTOR FocusPosition, DirectX::FXMVECTOR UpDirection)
{
	XMMATRIX view = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
	Transform.SetTransform(XMMatrixInverse(nullptr, view));
}

void Camera::SetPosition(float x, float y, float z)
{
	Transform.Position = XMFLOAT3(x, y, z);
}