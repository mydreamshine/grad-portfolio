#include "stdafx.h"
#include "Object.h"

using namespace DirectX;

void TransformInfo::SetBound(const DirectX::BoundingBox& newBound, BoundPivot pivot)
{
	m_BoundPivot = pivot;
	m_OriginBound = m_Bound = newBound;
	if (m_BoundPivot == BoundPivot::Bottom)
	{
		float bottom_y = m_OriginBound.Center.y - m_OriginBound.Extents.y;
		m_Bound.Center.y = bottom_y + m_Bound.Extents.y;
	}
}

void TransformInfo::SetWorldScale(const DirectX::XMFLOAT3& newScale)
{
	m_WorldScale = newScale;
	TransformInfo::UpdateBound();
}

void TransformInfo::SetWorldRotationEuler(const DirectX::XMFLOAT3& newRot)
{
	m_WorldRotationEuler = newRot;
	float deg2rad = MathHelper::Pi / 180.0f;
	XMMATRIX RotM = DirectX::XMMatrixRotationRollPitchYaw(newRot.x * deg2rad, newRot.y * deg2rad, newRot.z * deg2rad);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, RotM);
	XMStoreFloat4(&m_WorldRotationQut, R);
}

void TransformInfo::SetWorldPosition(const DirectX::XMFLOAT3& newPos)
{
	m_WorldPosition = newPos;
	TransformInfo::UpdateBound();
}

void TransformInfo::SetLocalScale(const DirectX::XMFLOAT3& newScale)
{
	m_LocalScale = newScale;
	TransformInfo::UpdateBound();
}

void TransformInfo::SetLocalRotationEuler(const DirectX::XMFLOAT3& newRot)
{
	m_LocalRotationEuler = newRot;
	float deg2rad = MathHelper::Pi / 180.0f;
	XMMATRIX RotM = DirectX::XMMatrixRotationRollPitchYaw(newRot.x * deg2rad, newRot.y * deg2rad, newRot.z * deg2rad);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, RotM);
	XMStoreFloat4(&m_LocalRotationQut, R);
}

void TransformInfo::SetLocalPosition(const DirectX::XMFLOAT3& newPos)
{
	m_LocalPosition = newPos;
	TransformInfo::UpdateBound();
}

void TransformInfo::SetWorldTransform(const DirectX::XMFLOAT3& newScale, const DirectX::XMFLOAT3& newRotationEuler, const DirectX::XMFLOAT3& newPosition)
{
	m_WorldScale = newScale;
	m_WorldRotationEuler = newRotationEuler;
	m_WorldPosition = newPosition;

	TransformInfo::UpdateBound();
	TransformInfo::UpdateWorldTransform();
}

void TransformInfo::SetWorldTransform(const DirectX::XMFLOAT4X4& newTransform)
{
	m_WorldTransform = newTransform;

	XMMATRIX WorldM = XMLoadFloat4x4(&m_WorldTransform);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, WorldM);
	XMStoreFloat3(&m_WorldScale, S);
	XMStoreFloat4(&m_WorldRotationQut, R);
	XMStoreFloat3(&m_WorldPosition, T);

	TransformInfo::UpdateBaseAxis();
	TransformInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

void TransformInfo::SetLocalTransform(const DirectX::XMFLOAT3& newScale, const DirectX::XMFLOAT3& newRotationEuler, const DirectX::XMFLOAT3& newPosition)
{
	m_LocalScale = newScale;
	m_LocalRotationEuler = newRotationEuler;
	m_LocalPosition = newPosition;

	TransformInfo::UpdateBound();
	TransformInfo::UpdateLocalTransform();
}

void TransformInfo::SetLocalTransform(const DirectX::XMFLOAT4X4& newTransform)
{
	m_LocalTransform = newTransform;

	XMMATRIX LocalM = XMLoadFloat4x4(&m_LocalTransform);
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, LocalM);
	XMStoreFloat3(&m_LocalScale, S);
	XMStoreFloat4(&m_LocalRotationQut, R);
	XMStoreFloat3(&m_LocalPosition, T);

	TransformInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

void TransformInfo::SetVelocity(const DirectX::XMFLOAT3& newVelocity)
{
	m_Velocity = newVelocity;
	//NumObjectCBDirty = gNumFrameResources;
	// 매번 업데이트마다 AnimteMovementWithVelocity()를 호출하기 때문에
	// 속도를 지정해줄 때에는 굳이 NumObjectCBDirty = gNumFrameResources;을 해줄 필요가 없다.
}

void TransformInfo::UpdateBound()
{
	m_Bound.Center.x = m_OriginBound.Center.x + m_LocalPosition.x + m_WorldPosition.x;
	m_Bound.Center.y = m_OriginBound.Center.y + m_LocalPosition.y + m_WorldPosition.y;
	m_Bound.Center.z = m_OriginBound.Center.z + m_LocalPosition.z + m_WorldPosition.z;
	m_Bound.Extents.x = m_OriginBound.Extents.x * m_LocalScale.x * m_WorldScale.x;
	m_Bound.Extents.y = m_OriginBound.Extents.y * m_LocalScale.y * m_WorldScale.y;;
	m_Bound.Extents.z = m_OriginBound.Extents.z * m_LocalScale.z * m_WorldScale.z;;

	if (m_BoundPivot == BoundPivot::Bottom)
	{
		float bottom_y = m_OriginBound.Center.y - m_OriginBound.Extents.y;
		m_Bound.Center.y = bottom_y + m_Bound.Extents.y;
		m_Bound.Center.y += m_LocalPosition.y + m_WorldPosition.y;
	}
}

void TransformInfo::UpdateWorldTransform()
{
	XMVECTOR World_S, World_R, World_T;
	World_S = XMLoadFloat3(&m_WorldScale);
	World_R = XMLoadFloat4(&m_WorldRotationQut);
	World_T = XMLoadFloat3(&m_WorldPosition);

	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMMATRIX WorldM = XMMatrixAffineTransformation(World_S, zero, World_R, World_T);

	XMStoreFloat4x4(&m_WorldTransform, WorldM);

	TransformInfo::UpdateBaseAxis();
	TransformInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

void TransformInfo::UpdateLocalTransform()
{
	XMVECTOR Local_S, Local_R, Local_T;
	Local_S = XMLoadFloat3(&m_LocalScale);
	Local_R = XMLoadFloat4(&m_LocalRotationQut);
	Local_T = XMLoadFloat3(&m_LocalPosition);;

	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMMATRIX LocalM = XMMatrixAffineTransformation(Local_S, zero, Local_R, Local_T);

	XMStoreFloat4x4(&m_LocalTransform, LocalM);

	TransformInfo::UpdateBound();

	NumObjectCBDirty = gNumFrameResources;
}

// 쉐이더에 WorldTransform을 넘길 때 Transpose를 시켜서 넘기기 때문에
// 쉐이더 내에선 기저벡터가 달라진다.
// 렌더링되는 모델의 전,후,좌,우가 WorldTransform과는 다르기 때문에
// 렌더링되는 모델을 기준으로 기저벡터를 계산해줘야 한다.
void TransformInfo::UpdateBaseAxis()
{
	XMFLOAT4X4 TransposeWorldM;
	XMStoreFloat4x4(&TransposeWorldM, XMMatrixTranspose(XMLoadFloat4x4(&m_WorldTransform)));
	m_Right.x = TransposeWorldM(0, 0);
	m_Right.y = TransposeWorldM(1, 0);
	m_Right.z = TransposeWorldM(2, 0);

	m_Up.x = TransposeWorldM(0, 1);
	m_Up.y = TransposeWorldM(1, 1);
	m_Up.z = TransposeWorldM(2, 1);

	m_Look.x = TransposeWorldM(0, 2);
	m_Look.y = TransposeWorldM(1, 2);
	m_Look.z = TransposeWorldM(2, 2);

	XMStoreFloat3(&m_Right, XMVector3Normalize(XMLoadFloat3(&m_Right)));
	XMStoreFloat3(&m_Up, XMVector3Normalize(XMLoadFloat3(&m_Up)));
	XMStoreFloat3(&m_Look, XMVector3Normalize(XMLoadFloat3(&m_Look)));
}

void TransformInfo::Animate(CTimer& gt)
{
	if (compareFloat(m_Velocity.x, 0.0f) && compareFloat(m_Velocity.y, 0.0f) && compareFloat(m_Velocity.z, 0.0f)) return;

	XMFLOAT3 newPos;
	XMVECTOR Velocity = XMLoadFloat3(&m_Velocity) * gt.GetTimeElapsed();
	XMStoreFloat3(&newPos, XMVectorAdd(XMLoadFloat3(&m_WorldPosition), Velocity));
	this->SetWorldPosition(newPos);
}

DirectX::XMFLOAT4X4 TransformInfo::GetWorldTransform()
{
	TransformInfo::UpdateWorldTransform();

	return m_WorldTransform;
}

DirectX::XMFLOAT4X4 TransformInfo::GetLocalTransform()
{
	TransformInfo::UpdateLocalTransform();

	return m_LocalTransform;
}

DirectX::XMFLOAT3 TransformInfo::GetRight()
{
	TransformInfo::UpdateWorldTransform();

	return m_Right;
}

DirectX::XMFLOAT3 TransformInfo::GetUp()
{
	TransformInfo::UpdateWorldTransform();

	return m_Up;;
}

DirectX::XMFLOAT3 TransformInfo::GetLook()
{
	TransformInfo::UpdateWorldTransform();

	return m_Look;
}

void TransformInfo::Init()
{
	NumObjectCBDirty = gNumFrameResources;
	m_WorldTransform = MathHelper::Identity4x4();
	m_TexTransform = MathHelper::Identity4x4();
	m_TexAlpha = 1.0f;
	m_nonShadowRender = false;

	m_OriginBound.Center = { 0.0f, 0.0f, 0.0f };
	m_OriginBound.Extents = { 0.0f, 0.0f, 0.0f };
	m_Bound.Center = { 0.0f, 0.0f, 0.0f };
	m_Bound.Extents = { 0.0f, 0.0f, 0.0f };
	m_LocalTransform = MathHelper::Identity4x4();

	m_WorldPosition = { 0.0f, 0.0f, 0.0f };
	m_WorldRotationQut = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
	m_WorldScale = { 1.0f, 1.0f, 1.0f };
	m_LocalPosition = { 0.0f, 0.0f, 0.0f };
	m_LocalRotationQut = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
	m_LocalScale = { 1.0f, 1.0f, 1.0f };

	m_Right = { 1.0f, 0.0f, 0.0f };
	m_Up = { 0.0f, 1.0f, 0.0f };
	m_Look = { 0.0f, 0.0f, 1.0f };

	m_Velocity = { 0.0f, 0.0f, 0.0f };

	m_AttachingTargetBoneID = -1;
}

bool Object::ProcessSelfDeActivate(CTimer& gt)
{
	if (SelfDeActivated == false) return !Activated;

	if (DeActivatedDecrease - gt.GetTimeElapsed() <= 0.0f)
	{
		ObjectManager objManager;
		objManager.DeActivateObj(this);
	}
	else DeActivatedDecrease -= gt.GetTimeElapsed();

	if (DisappearForDeAcTime == true)
		m_TransformInfo->m_TexAlpha = DeActivatedDecrease / DeActivatedTime;

	return !Activated;
}
