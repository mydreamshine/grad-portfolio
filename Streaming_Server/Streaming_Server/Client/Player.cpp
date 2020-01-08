#include "stdafx.h"
#include "Player.h"


CPlayer::CPlayer()
{
	m_pCamera = NULL;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_fSpeed = 100.0f;
	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
	m_Status = 0;
	m_Count = 0;
	m_PowerTime = 1.0f;
}
CPlayer::~CPlayer()
{
	ReleaseShaderVariables();
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}
void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}
void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity�� ���̸� �÷��̾ �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�. 
	if (bUpdateVelocity)
	{
		//�÷��̾��� �ӵ� ���͸� xmf3Shift ���͸�ŭ �����Ѵ�. 
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�. 
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� xmf3Shift ���͸�ŭ �̵��Ѵ�. 
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::SetAxisFromMatrix(XMFLOAT4X4& xmMatrix) {
	this->m_xmf3Right = XMFLOAT3(&xmMatrix._11);
	this->m_xmf3Up = XMFLOAT3(&xmMatrix._21);
	this->m_xmf3Look = XMFLOAT3(&xmMatrix._31);
}

//�� �Լ��� �� �����Ӹ��� ȣ��ȴ�.
void CPlayer::Update(float fTimeElapsed)
{
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� 3��Ī ī�޶� �����Ѵ�. 
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//ī�޶��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. 
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//ī�޶��� ī�޶� ��ȯ ����� �ٽ� �����Ѵ�. 
	m_pCamera->RegenerateViewMatrix();

	if (m_Status) {
		m_Count -= fTimeElapsed;
		if (m_Count <= 0.0f)
			m_Status = 0;
	}
}
void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z);
	if (xmf3PlayerPosition.y < fHeight)
	{
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}
void CPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
		p3rdPersonCamera->SetLookAt(GetPosition());
	}
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}

//-------------------------------------------------------------------------------------------------------------------------------------

CAirplanePlayer::CAirplanePlayer()
{
	//ī�޶� ����
	m_pCamera = new CThirdPersonCamera(NULL);
	m_pCamera->SetPlayer(this);

	//ī�޶� ����
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	//�÷��̾� ��ġ�� ���� 
	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
}
CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f,	0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}