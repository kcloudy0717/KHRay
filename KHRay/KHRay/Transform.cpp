#include "Transform.h"

using namespace DirectX;

Transform::Transform()
{
	XMStoreFloat3(&Position, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
	XMStoreFloat4(&Orientation, XMQuaternionIdentity());
	XMStoreFloat3(&Scale, XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f));
}

void Transform::SetTransform(DirectX::FXMMATRIX M)
{
	XMVECTOR translation, orientation, scale;
	XMMatrixDecompose(&scale, &orientation, &translation, M);

	XMStoreFloat3(&Position, translation);
	XMStoreFloat4(&Orientation, orientation);
	XMStoreFloat3(&Scale, scale);
}

void Transform::Translate(float DeltaX, float DeltaY, float DeltaZ)
{
	XMVECTOR CurrentPosition = XMLoadFloat3(&Position);
	XMVECTOR Velocity = XMVector3Rotate(XMVectorSet(DeltaX, DeltaY, DeltaZ, 0.0f), XMLoadFloat4(&Orientation));
	XMStoreFloat3(&Position, XMVectorAdd(CurrentPosition, Velocity));
}

void Transform::SetScale(float ScaleX, float ScaleY, float ScaleZ)
{
	Scale.x = ScaleX;
	Scale.y = ScaleY;
	Scale.z = ScaleZ;
}

void Transform::SetOrientation(float AngleX, float AngleY, float AngleZ)
{
	XMVECTOR EulerRotation = XMQuaternionRotationRollPitchYaw(AngleX, AngleY, AngleZ);
	XMStoreFloat4(&Orientation, EulerRotation);
}

void Transform::Rotate(float AngleX, float AngleY, float AngleZ)
{
	XMVECTOR currentRotation = XMLoadFloat4(&Orientation);
	XMVECTOR pitch = XMQuaternionNormalize(XMQuaternionRotationAxis(Right(), AngleX));
	XMVECTOR yaw = XMQuaternionNormalize(XMQuaternionRotationAxis(g_XMIdentityR1, AngleY));
	XMVECTOR roll = XMQuaternionNormalize(XMQuaternionRotationAxis(Forward(), AngleZ));

	XMStoreFloat4(&Orientation, XMQuaternionMultiply(currentRotation, pitch));
	currentRotation = XMLoadFloat4(&Orientation);
	XMStoreFloat4(&Orientation, XMQuaternionMultiply(currentRotation, yaw));
	currentRotation = DirectX::XMLoadFloat4(&Orientation);
	XMStoreFloat4(&Orientation, XMQuaternionMultiply(currentRotation, roll));
}

DirectX::XMMATRIX Transform::Matrix() const
{
	XMVECTOR scale = XMLoadFloat3(&Scale);
	XMVECTOR orientation = XMLoadFloat4(&Orientation);
	XMVECTOR position = XMLoadFloat3(&Position);

	XMMATRIX S = XMMatrixScalingFromVector(scale);
	XMMATRIX R = XMMatrixRotationQuaternion(orientation);
	XMMATRIX T = XMMatrixTranslationFromVector(position);

	// S * R * T
	return S * R * T;
}

DirectX::XMVECTOR Transform::Right() const
{
	return XMVector3Rotate(g_XMIdentityR0, XMLoadFloat4(&Orientation));
}

DirectX::XMVECTOR Transform::Up() const
{
	return XMVector3Rotate(g_XMIdentityR1, XMLoadFloat4(&Orientation));
}

DirectX::XMVECTOR Transform::Forward() const
{
	return XMVector3Rotate(g_XMIdentityR2, XMLoadFloat4(&Orientation));
}

bool Transform::operator==(const Transform& Transform) const
{
	return
		XMVector3Equal(XMLoadFloat3(&Position), XMLoadFloat3(&Transform.Position)) &&
		XMVector3Equal(XMLoadFloat3(&Scale), XMLoadFloat3(&Transform.Scale)) &&
		XMVector4Equal(XMLoadFloat4(&Orientation), XMLoadFloat4(&Transform.Orientation));
}

bool Transform::operator!=(const Transform& Transform) const
{
	return !(*this == Transform);
}