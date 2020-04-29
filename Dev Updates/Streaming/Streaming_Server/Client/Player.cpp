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
	//bUpdateVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경한다. 
	if (bUpdateVelocity)
	{
		//플레이어의 속도 벡터를 xmf3Shift 벡터만큼 변경한다. 
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다. 
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//플레이어의 위치가 변경되었으므로 카메라의 위치도 xmf3Shift 벡터만큼 이동한다. 
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::SetAxisFromMatrix(XMFLOAT4X4& xmMatrix) {
	this->m_xmf3Right = XMFLOAT3(&xmMatrix._11);
	this->m_xmf3Up = XMFLOAT3(&xmMatrix._21);
	this->m_xmf3Look = XMFLOAT3(&xmMatrix._31);
}

//이 함수는 매 프레임마다 호출된다.
void CPlayer::Update(float fTimeElapsed)
{
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	//플레이어의 위치가 변경되었으므로 3인칭 카메라를 갱신한다. 
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//카메라의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//카메라의 카메라 변환 행렬을 다시 생성한다. 
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
	//카메라 생성
	m_pCamera = new CThirdPersonCamera(NULL);
	m_pCamera->SetPlayer(this);

	//카메라 설정
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	//플레이어 위치를 설정 
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