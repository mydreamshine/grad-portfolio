#include "Object.h"

using namespace DirectX;

void ObjectInfo::SetBound(const DirectX::BoundingBox& newBound)
{
	m_Bound = newBound;
	m_BoundExtendsOrigin = m_Bound.Extents;
}

void ObjectInfo::SetWorldScale(const DirectX::XMFLOAT3& newScale)
{
	m_WorldScale = newScale;
	ObjectInfo::UpdateBound();
}

void ObjectInfo::SetWorldRotationEuler(const DirectX::XMFLOAT3& newRot)
{
	m_WorldRotationEuler = newRot;
	float deg2rad = MathHelper::Pi / 180.0f;
	XMMATRIX RotM = DirectX::XMMatrixRotationRollPitchYaw(newRot.x * deg2rad, newRot.y * deg2rad, newRot.z * deg2rad);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, RotM);
	XMStoreFloat4(&m_WorldRotationQut, R);
}

void ObjectInfo::SetWorldPosition(const DirectX::XMFLOAT3& newPos)
{
	m_WorldPosition = newPos;
	ObjectInfo::UpdateBound();
}

void ObjectInfo::SetLocalScale(const DirectX::XMFLOAT3& newScale)
{
	m_LocalScale = newScale;
	ObjectInfo::UpdateBound();
}

void ObjectInfo::SetLocalRotationEuler(const DirectX::XMFLOAT3& newRot)
{
	m_LocalRotationEuler = newRot;
	float deg2rad = MathHelper::Pi / 180.0f;
	XMMATRIX RotM = DirectX::XMMatrixRotationRollPitchYaw(newRot.x * deg2rad, newRot.y * deg2rad, newRot.z * deg2rad);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, RotM);
	XMStoreFloat4(&m_LocalRotationQut, R);
}

void ObjectInfo::RotationLocalY(float angle)
{
	m_LocalRotationEuler.y += angle;
	float deg2rad = MathHelper::Pi / 180.0f;
	XMMATRIX R_y = XMMatrixRotationY(angle * deg2rad);
	XMMATRIX WorldM = XMLoadFloat4x4(&m_WorldTransform);
	// Matrix Multiply Order in DirectX: local ¡æ world -> view -> proj
	WorldM = R_y * WorldM;

	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, WorldM);
	XMStoreFloat3(&m_WorldScale, S);
	XMStoreFloat4(&m_WorldRotationQut, R);
	XMStoreFloat3(&m_WorldPosition, T);
}

void ObjectInfo::SetLocalPosition(const DirectX::XMFLOAT3& newPos)
{
	m_LocalPosition = newPos;
	ObjectInfo::UpdateBound();
}

void ObjectInfo::SetWorldTransform(const DirectX::XMFLOAT4X4& newTransform)
{
	m_WorldTransform = newTransform;

	XMMATRIX WorldM = XMLoadFloat4x4(&m_WorldTransform);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, WorldM);
	XMStoreFloat3(&m_WorldScale, S);
	XMStoreFloat4(&m_WorldRotationQut, R);
	XMStoreFloat3(&m_WorldPosition, T);

	m_Right.x = m_WorldTransform(0, 0);
	m_Right.y = m_WorldTransform(1, 0);
	m_Right.z = m_WorldTransform(2, 0);

	m_Up.x = m_WorldTransform(0, 1);
	m_Up.y = m_WorldTransform(1, 1);
	m_Up.z = m_WorldTransform(2, 1);

	m_Look.x = m_WorldTransform(0, 2);
	m_Look.y = m_WorldTransform(1, 2);
	m_Look.z = m_WorldTransform(2, 2);

	ObjectInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

void ObjectInfo::SetLocalTransform(const DirectX::XMFLOAT4X4& newTransform)
{
	m_LocalTransform = newTransform;

	XMMATRIX LocalM = XMLoadFloat4x4(&m_LocalTransform);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, LocalM);
	XMStoreFloat3(&m_LocalScale, S);
	XMStoreFloat4(&m_LocalRotationQut, R);
	XMStoreFloat3(&m_LocalPosition, T);

	ObjectInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

void ObjectInfo::UpdateBound()
{
	XMFLOAT3 BoundCenter;
	BoundCenter.x = m_WorldPosition.x + m_LocalPosition.x;
	BoundCenter.y = m_WorldPosition.y + m_LocalPosition.y;
	BoundCenter.z = m_WorldPosition.z + m_LocalPosition.z;
	m_Bound.Center = BoundCenter;
	m_Bound.Center.y += m_Bound.Extents.y;
	m_Bound.Extents.x = m_BoundExtendsOrigin.x * m_LocalScale.x * m_WorldScale.x;
	m_Bound.Extents.y = m_BoundExtendsOrigin.y * m_LocalScale.y * m_WorldScale.y;;
	m_Bound.Extents.z = m_BoundExtendsOrigin.z * m_LocalScale.z * m_WorldScale.z;;
}

void ObjectInfo::UpdateWorldTransform()
{
	XMVECTOR World_S, World_R, World_T;
	World_S = XMLoadFloat3(&m_WorldScale);
	World_R = XMLoadFloat4(&m_WorldRotationQut);
	World_T = XMLoadFloat3(&m_WorldPosition);

	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMMATRIX WorldM = XMMatrixAffineTransformation(World_S, zero, World_R, World_T);

	XMStoreFloat4x4(&m_WorldTransform, WorldM);

	m_Right.x = m_WorldTransform(0, 0);
	m_Right.y = m_WorldTransform(1, 0);
	m_Right.z = m_WorldTransform(2, 0);

	m_Up.x = m_WorldTransform(0, 1);
	m_Up.y = m_WorldTransform(1, 1);
	m_Up.z = m_WorldTransform(2, 1);

	m_Look.x = m_WorldTransform(0, 2);
	m_Look.y = m_WorldTransform(1, 2);
	m_Look.z = m_WorldTransform(2, 2);

	ObjectInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

void ObjectInfo::UpdateLocalTransform()
{
	XMVECTOR Local_S, Local_R, Local_T;
	Local_S = XMLoadFloat3(&m_LocalScale);
	Local_R = XMLoadFloat4(&m_LocalRotationQut);
	Local_T = XMLoadFloat3(&m_LocalPosition);;

	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMMATRIX LocalM = XMMatrixAffineTransformation(Local_S, zero, Local_R, Local_T);

	XMStoreFloat4x4(&m_LocalTransform, LocalM);

	ObjectInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

DirectX::XMFLOAT4X4 ObjectInfo::GetWorldTransform()
{
	ObjectInfo::UpdateWorldTransform();

	return m_WorldTransform;
}

DirectX::XMFLOAT4X4 ObjectInfo::GetLocalTransform()
{
	ObjectInfo::UpdateLocalTransform();

	return m_LocalTransform;
}

DirectX::XMFLOAT3 ObjectInfo::GetRight()
{
	ObjectInfo::UpdateWorldTransform();

	return m_Right;
}

DirectX::XMFLOAT3 ObjectInfo::GetUp()
{
	ObjectInfo::UpdateWorldTransform();

	return m_Up;;
}

DirectX::XMFLOAT3 ObjectInfo::GetLook()
{
	ObjectInfo::UpdateWorldTransform();

	return m_Look;
}
