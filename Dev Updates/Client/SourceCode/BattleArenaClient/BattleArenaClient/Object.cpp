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
	TransformInfo::SetWorldRotationEuler(newRotationEuler);
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
	TransformInfo::SetLocalRotationEuler(newRotationEuler);
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
	// �Ź� ������Ʈ���� AnimteMovementWithVelocity()�� ȣ���ϱ� ������
	// �ӵ��� �������� ������ ���� NumObjectCBDirty = gNumFrameResources;�� ���� �ʿ䰡 ����.
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

// ���̴��� WorldTransform�� �ѱ� �� Transpose�� ���Ѽ� �ѱ�� ������
// ���̴� ������ �������Ͱ� �޶�����.
// �������Ǵ� ���� ��,��,��,�찡 WorldTransform���� �ٸ��� ������
// �������Ǵ� ���� �������� �������͸� �������� �Ѵ�.
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

	WorldPosIsInterpolation = false;
	m_InterPolationDestPosition = { 0.0f, 0.0f, 0.0f };

	SetedSpritedTexture = false;
	SpritedTextureSize = { 1.0f, 1.0f };
	ClipedTextureSize = { 1.0f, 1.0f };
	ClipedUV_StartPos = { -1.0f, -1.0f };
	ClipedUV_Size = { -1.0f, -1.0f };
}

void TransformInfo::SetInterpolatedDestPosition(const DirectX::XMFLOAT3& newPosition, bool WorldPosInterpolated)
{
	m_InterPolationDestPosition = newPosition;
	WorldPosIsInterpolation = WorldPosInterpolated;
}

void TransformInfo::InterpolateTransformWorldPosition(CTimer& gt)
{
	XMVECTOR CURR_WORLD_POS = XMLoadFloat3(&m_WorldPosition);
	XMVECTOR DEST_WORLD_POS = XMLoadFloat3(&m_InterPolationDestPosition);
	float bias = (gt.GetTimeElapsed() * 20.0f);
	bias = (bias > 1.0f) ? 1.0f : bias;

	XMVECTOR dir = XMVectorSubtract(DEST_WORLD_POS, CURR_WORLD_POS);
	dir = XMVectorScale(dir, bias);
	CURR_WORLD_POS = XMVectorAdd(dir, CURR_WORLD_POS);

	XMFLOAT3 newWorldPos; XMStoreFloat3(&newWorldPos, CURR_WORLD_POS);
	TransformInfo::SetWorldPosition(newWorldPos);
	TransformInfo::UpdateWorldTransform();
}

void TransformInfo::SetSpritedTextureSize(const DirectX::XMFLOAT2& TextureSize)
{
	SpritedTextureSize = TextureSize;
}

void TransformInfo::SetClipedTextureSize(const DirectX::XMFLOAT2& ClipedSize)
{
	ClipedTextureSize = ClipedSize;
}

void TransformInfo::SetClipedTexturePos(float start_x, float start_y)
{
	ClipedUV_StartPos.x = MathHelper::Clamp(start_x / SpritedTextureSize.x, 0.0f, 1.0f); // Cliped Start Position_X
	ClipedUV_StartPos.y = MathHelper::Clamp(start_y / SpritedTextureSize.y, 0.0f, 1.0f); // Cliped Start Position_Y
	ClipedUV_Size.x = MathHelper::Clamp(ClipedUV_StartPos.x + ClipedTextureSize.x / SpritedTextureSize.x, 0.0f, 1.0f); // Cliped Width
	ClipedUV_Size.y = MathHelper::Clamp(ClipedUV_StartPos.y + ClipedTextureSize.y / SpritedTextureSize.y, 0.0f, 1.0f); // Cliped Height
}

void TransformInfo::UpdateClipedTexture(float CurrClipTime, float ClipChangedTotalTime, CTimer& gt)
{
	int ClipCol = (int)(SpritedTextureSize.x / ClipedTextureSize.x);
	int ClipRow = (int)(SpritedTextureSize.y / ClipedTextureSize.y);

	float ClipChangeInterval = ClipChangedTotalTime / (float)(ClipCol * ClipRow);

	int CurrClipNum = (int)(CurrClipTime / ClipChangeInterval);
	int CurrClipCol = CurrClipNum % ClipCol;
	int CurrClipRow = CurrClipNum / ClipCol;

	float CurrClipStart_x = (float)CurrClipCol * ClipedTextureSize.x;
	float CurrClipStart_y = (float)CurrClipRow * ClipedTextureSize.y;

	TransformInfo::SetClipedTexturePos(CurrClipStart_x, CurrClipStart_y);
	m_TexAlpha = 1.0f;
}

bool Object::ProcessSelfDeActivate(CTimer& gt)
{
	if (SelfDeActivated == false) return !Activated;

	if (DeActivatedDecrease - gt.GetTimeElapsed() <= 0.0f)
	{
		ObjectManager objManager;
		objManager.DeActivateObj(this);
	}
	else
	{
		DeActivatedDecrease -= gt.GetTimeElapsed();
		if (DeActivatedDecrease < 0.0f) DeActivatedDecrease = 0.0f;
	}

	if (DisappearForDeAcTime == true)
		m_TransformInfo->m_TexAlpha = DeActivatedDecrease / DeActivatedTime;

	return !Activated;
}
