#pragma once
#include "Mesh.h"

class CShader;
class CCamera;

class CGameObject
{
private:
	int m_nReferences = 0;
	CMesh *m_pMesh = NULL;
	XMFLOAT4 m_xmf4Color;

public:
	BoundingOrientedBox	m_xmOOBB;
	XMFLOAT4X4 m_xmf4x4World;

	CGameObject();
	virtual ~CGameObject();

	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	void MatchUp(XMFLOAT3& Axis);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	void ReleaseUploadBuffers();

	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();

	void SetMesh(CMesh *pMesh) { m_pMesh = pMesh; }
	void UpdateOOBB() {
		if (m_pMesh)
		{
			m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
			XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
		}
	}
	void SetColor(XMFLOAT4& Color) { m_xmf4Color = Color; }
	void SetColor(float red, float green, float blue) { m_xmf4Color.x = red; m_xmf4Color.y = green; m_xmf4Color.z = blue; }
	XMFLOAT4& GetColor() { return m_xmf4Color; }

	void ClearMatrix();
};

class CRotatingObject : public CGameObject
{
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	CRotatingObject();
	virtual ~CRotatingObject();

	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) {
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}
	virtual void Animate(float fTimeElapsed);
};

class CBirdObject : public CRotatingObject 
{
private:
	XMFLOAT3 m_xmf3Direction;
	float m_fVelocity;

	float m_fWidth;
	float m_fDepth;
	float m_fMinHeight;
	float m_fMaxHeight;

public:
	CBirdObject() {
		m_xmf3Direction = Vector3::Normalize(XMFLOAT3( float(rand()%100), float(rand() % 100), float(rand() % 100)));
		m_fVelocity = float(rand() % 50) + 30;
	}
	virtual ~CBirdObject() {}

	//새가 날아다닐 수 있는 경계를 설정한다.
	void SetBound(float width, float height, float mindepth, float maxdepth) {
		m_fWidth = width;
		m_fDepth = height;
		m_fMinHeight = mindepth;
		m_fMaxHeight = maxdepth;
	}
	//새가 날라다니는 방향을 재설정한다.
	void SetDirection() { m_xmf3Direction = Vector3::Normalize(XMFLOAT3(float(rand() % 100), float(rand() % 100), float(rand() % 100))); }

	virtual void Animate(float fTimeElapsed) {
		CRotatingObject::Animate(fTimeElapsed);
		XMFLOAT3 Pos = Vector3::Add(GetPosition(), m_xmf3Direction, m_fVelocity * fTimeElapsed);

		//경계 밖으로 나가면 반대방향으로 날아간다.
		if (Pos.x < 0) {
			Pos.x = 0;
			m_xmf3Direction.x *= -1;
		}
		else if (Pos.x > m_fWidth) {
			Pos.x = m_fWidth;
			m_xmf3Direction.x *= -1;
		}
		else if (Pos.y < 0) {
			Pos.y = 0;
			m_xmf3Direction.y *= -1;
		}
		else if (Pos.y < m_fMinHeight) {
			Pos.y = m_fMinHeight;
			m_xmf3Direction.y *= -1;
		}
		else if (Pos.y > m_fMaxHeight) {
			Pos.y = m_fMaxHeight;
			m_xmf3Direction.y *= -1;
		}
		else if (Pos.z < 0) {
			Pos.y = 0;
			m_xmf3Direction.z *= -1;
		}
		else if (Pos.z > m_fDepth) {
			Pos.y = m_fDepth;
			m_xmf3Direction.z *= -1;
		}
		SetPosition(Pos);
		UpdateOOBB();
	}
};

class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	virtual ~CHeightMapTerrain();
private:
	CHeightMapImage *m_pHeightMapImage;
	int m_nWidth;
	int m_nLength;
	XMFLOAT3 m_xmf3Scale;
	
public:
	float GetHeight(float x, float z) { return m_pHeightMapImage->GetHeight(x /	m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y; }
	XMFLOAT3 GetNormal(float x, float z) { return m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z /	m_xmf3Scale.z)); }
	int GetHeightMapWidth() { return m_pHeightMapImage->GetHeightMapWidth(); }
	int GetHeightMapLength() { return m_pHeightMapImage->GetHeightMapLength(); }
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	
	//지형 크기 반환
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
	//최대 높이 반환
	float GetMaxHeight() { return m_pHeightMapImage->GetMaxHeight() * m_xmf3Scale.y; }

	void CreateMesh(CMesh** ppMesh, std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, int nBlockWidth, int nBlockLength, XMFLOAT4 xmf4Color, int nStartInstance);
};
