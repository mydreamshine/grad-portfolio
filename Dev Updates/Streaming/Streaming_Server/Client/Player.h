#pragma once
#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

#include "GameObject.h"
#include "Camera.h"
#include "Shader.h"

class CPlayer : public CGameObject
{
protected:
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다. 
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다. 
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;
	float m_fSpeed;

	//플레이어의 이동 속도를 나타내는 벡터이다. 
	XMFLOAT3 m_xmf3Velocity;

	//플레이어에 작용하는 중력을 나타내는 벡터이다. 
	XMFLOAT3 m_xmf3Gravity;

	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다. 
	float m_fMaxVelocityXZ;
	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다. 
	float m_fMaxVelocityY;
	//플레이어에 작용하는 마찰력을 나타낸다. 
	float m_fFriction;
	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다. 
	LPVOID m_pPlayerUpdatedContext;
	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다. 
	LPVOID m_pCameraUpdatedContext;
	//플레이어에 현재 설정된 카메라이다. 
	CCamera *m_pCamera = NULL;

	int m_Status;
	float m_Count;
	float m_PowerTime;

public:
	CPlayer();
	~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	void SetPosition(XMFLOAT3& xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z),	false);
	}

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	float GetYaw() { return(m_fYaw); }
	float GetPitch() { return(m_fPitch); }
	float GetRoll() { return(m_fRoll); }
	CCamera *GetCamera() { return(m_pCamera); }
	float GetSpd() { return m_fSpeed; }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	//플레이어를 이동하는 함수이다. 
	void Move(XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	void SetAxisFromMatrix(XMFLOAT4X4& xmMatrix);
	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다. 
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다. 
	virtual void OnPrepareRender();

	//충돌체크용 함수들
	void SetPower() { m_Status = 1; m_Count = m_PowerTime; }
	int GetStatus() { return m_Status; }
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();
	virtual void OnPrepareRender();
};