#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Camera.h"


CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
}

CGameObject::~CGameObject()
{
}
void CGameObject::Animate(float fTimeElapsed)
{
}
void CGameObject::OnPrepareRender()
{
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle) {
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}
void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
XMFLOAT3 CGameObject::GetPosition()
{
	return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}

XMFLOAT3 CGameObject::GetLook()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33));
}
XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22,
		m_xmf4x4World._23)));
}
XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12,
		m_xmf4x4World._13)));
}
void CGameObject::MatchUp(XMFLOAT3& Axis) {
	XMFLOAT3 UpVector = this->GetUp();
	
	XMFLOAT3 NormalAxis = Vector3::Normalize(Axis);
	XMFLOAT3 RotateAxis = Vector3::CrossProduct(UpVector, NormalAxis, true);

	if (Vector3::IsZero(RotateAxis))
		return;

	float Angle = acos(Vector3::DotProduct(UpVector, NormalAxis));
	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&RotateAxis), Angle);

	XMFLOAT3 LookVector = Vector3::TransformNormal(GetLook(), xmmtxRotate);
	UpVector = NormalAxis;
	XMFLOAT3 RightVector = Vector3::TransformNormal(GetRight(), xmmtxRotate);

	m_xmf4x4World._11 = RightVector.x; m_xmf4x4World._12 = RightVector.y; m_xmf4x4World._13 = RightVector.z;
	m_xmf4x4World._21 = UpVector.x; m_xmf4x4World._22 = UpVector.y; m_xmf4x4World._23 = UpVector.z;
	m_xmf4x4World._31 = LookVector.x; m_xmf4x4World._32 = LookVector.y; m_xmf4x4World._33 = LookVector.z;
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::ClearMatrix() {
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
}

// =============================================================================
CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 90.0f;
}
CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

//

CHeightMapTerrain::CHeightMapTerrain(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);
}
CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}
void CHeightMapTerrain::CreateMesh(CMesh** ppMesh, std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, int nBlockWidth, int nBlockLength, XMFLOAT4 xmf4Color, int nStartInstance) {
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;


	ppMesh[0] = NULL;
	for (int i=0, z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++, i++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			ppMesh[i] = new CHeightMapGridMesh(pVertex, pIndex, ppMesh[0], xStart, zStart, nBlockWidth, nBlockLength, m_xmf3Scale, xmf4Color, m_pHeightMapImage);
			ppMesh[i]->m_nInstance = 1;
			ppMesh[i]->m_nStartInstance = nStartInstance;
		}
	}
}