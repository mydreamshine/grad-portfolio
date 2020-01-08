#pragma once
#include "stdafx.h"

#define RANDOM_COLOR XMFLOAT4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX))

class CVertex {
protected:
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }

};

class CDiffusedVertex : public CVertex {
protected:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CDiffusedVertex() {
		m_xmf3Position	= XMFLOAT3(0.0f, 0.0f, 0.0f); 
		m_xmf4Diffuse	= XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position	= XMFLOAT3(x, y, z); 
		m_xmf4Diffuse	= xmf4Diffuse;
	}
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position	= xmf3Position; 
		m_xmf4Diffuse	= xmf4Diffuse;
	}
	~CDiffusedVertex() { }
};

class CMesh
{
private:
	int m_nReferences = 0;

protected:
	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	//정점버퍼
	UINT m_nVertices = 0;
	UINT m_nOffset = 0;

	//인덱스버퍼
	UINT m_nIndices = 0;
	UINT m_nStartIndex = 0;

	//인스턴스 수
	UINT m_nInstance = 0;
	UINT m_nStartInstance = 0;

	//OOBB
	BoundingOrientedBox	m_xmOOBB;

	CMesh();
	virtual ~CMesh();

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }
};

class CCubeMeshDiffused : public CMesh
{
public:
	//직육면체의 가로, 세로, 깊이의 길이를 지정하여 직육면체 메쉬를 생성한다. 
	CCubeMeshDiffused(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f, XMFLOAT4 color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	virtual ~CCubeMeshDiffused();
};

class CAirplaneMeshDiffused : public CMesh
{
public:
	CAirplaneMeshDiffused(std::vector<CDiffusedVertex>& pVertex, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 4.0f, XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CAirplaneMeshDiffused();
};

class CTreeMesh : public CMesh {
public:
	CTreeMesh(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CTreeMesh();
};

class CHeightMapImage
{
private:
	//이미지 저장
	BYTE *m_pHeightMapPixels;
	//이미지의 가로세로
	int m_nWidth;
	int m_nLength;
	//스케일 벡터
	XMFLOAT3 m_xmf3Scale;
	//높이 최댓값
	float m_fMaxHeight;
public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z); 
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	BYTE *GetHeightMapPixels() { return m_pHeightMapPixels; }
	int GetHeightMapWidth() { return m_nWidth; }
	int GetHeightMapLength() { return m_nLength; }
	float GetMaxHeight() { return m_fMaxHeight; }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	int m_nWidth;
	int m_nLength;
	XMFLOAT3 m_xmf3Scale;
public:
	CHeightMapGridMesh(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale =	XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void	*pContext = NULL);
	virtual ~CHeightMapGridMesh();
	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	int GetWidth() { return m_nWidth; }
	int GetLength() { return m_nLength; }
	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};

class CCubeMeshTop : public CMesh {
public:
	CCubeMeshTop(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, float fWidth = 2.0f, float fDepth = 2.0f, XMFLOAT4 color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	virtual ~CCubeMeshTop();
};