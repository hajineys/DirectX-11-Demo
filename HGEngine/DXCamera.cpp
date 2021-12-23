#include "DXCamera.h"
#include "MathHelper.h"

DXCamera::DXCamera()
	: mPosition(0.0f, 0.0f, 0.0f),
	mRight(1.0f, 0.0f, 0.0f),
	mUp(0.0f, 1.0f, 0.0f),
	mLook(0.0f, 0.0f, 1.0f),
	mNearZ(0.0f),
	mFarZ(0.0f),
	mAspect(0.0f),
	mFovY(0.0f),
	mNearWindowHeight(0.0f),
	mFarWindowHeight(0.0f),
	mView(), mProj()
{
	SetLens(0.25f * MathHelper::Pi, 1.0f, 0.0001f, 1000.0f);
}

DXCamera::~DXCamera()
{

}

XMVECTOR DXCamera::GetPositionXM() const
{
	return XMLoadFloat3(&mPosition);
}

XMFLOAT3 DXCamera::GetPosition() const
{
	return mPosition;
}

void DXCamera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
}

void DXCamera::SetPosition(const XMFLOAT3& v)
{
	mPosition = v;
}

XMVECTOR DXCamera::GetRightXM() const
{
	return XMLoadFloat3(&mRight);
}

XMFLOAT3 DXCamera::GetRight() const
{
	return mRight;
}

XMVECTOR DXCamera::GetUpXM() const
{
	return XMLoadFloat3(&mUp);
}

XMFLOAT3 DXCamera::GetUp() const
{
	return mUp;
}

XMVECTOR DXCamera::GetLookXM() const
{
	return XMLoadFloat3(&mLook);
}

XMFLOAT3 DXCamera::GetLook() const
{
	return mLook;
}

float DXCamera::GetNearZ() const
{
	return mNearZ;
}

float DXCamera::GetFarZ() const
{
	return mFarZ;
}

float DXCamera::GetAspect() const
{
	return mAspect;
}

float DXCamera::GetFovY() const
{
	return mFovY;
}

float DXCamera::GetFovX() const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return (float)(2.0f * atan(halfWidth / mNearZ));
}

float DXCamera::GetNearWindowWidth() const
{
	return mAspect * mNearWindowHeight;
}

float DXCamera::GetNearWindowHeight() const
{
	return mNearWindowHeight;
}

float DXCamera::GetFarWindowWidth() const
{
	return mAspect * mFarWindowHeight;
}

float DXCamera::GetFarWindowHeight() const
{
	return mFarWindowHeight;
}

void DXCamera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
}

void DXCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	// 시선벡터와 월드업을 외적하면 right벡터가 나오고
	// 다시 그 right벡터와 시선벡터를 외적하면 내 up벡터가 나옴
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
}

void DXCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

XMMATRIX DXCamera::View() const
{
	return XMLoadFloat4x4(&mView);
}

XMMATRIX DXCamera::Proj() const
{
	return XMLoadFloat4x4(&mProj);
}

XMMATRIX DXCamera::ViewProj() const
{
	return XMMatrixMultiply(View(), Proj());
}

void DXCamera::Strafe(float d)
{
	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));
}

void DXCamera::Walk(float d)
{
	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
}

void DXCamera::WorldUpDown(float d)
{
	// mPosition += d * mUp
	XMVECTOR scale = XMVectorReplicate(d);
	XMVECTOR up = XMLoadFloat3(&mUp);
	XMVECTOR position = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(scale, up, position));
}

void DXCamera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
}

void DXCamera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
}

void DXCamera::UpdateViewMatrix()
{
	XMVECTOR R = XMLoadFloat3(&mRight);
	XMVECTOR U = XMLoadFloat3(&mUp);
	XMVECTOR L = XMLoadFloat3(&mLook);
	XMVECTOR P = XMLoadFloat3(&mPosition);

	// Keep camera's axes orthogonal to each other and of unit length.
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = XMVector3Cross(U, L);

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	XMStoreFloat3(&mLook, L);

	mView(0, 0) = mRight.x;
	mView(1, 0) = mRight.y;
	mView(2, 0) = mRight.z;
	mView(3, 0) = x;

	mView(0, 1) = mUp.x;
	mView(1, 1) = mUp.y;
	mView(2, 1) = mUp.z;
	mView(3, 1) = y;

	mView(0, 2) = mLook.x;
	mView(1, 2) = mLook.y;
	mView(2, 2) = mLook.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;
}