#include "stdafx.h"
#include "Mesh.h"


CMesh::CMesh() {
}

CMesh::~CMesh() {
}



void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if (m_nIndices)
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, m_nInstance, m_nStartIndex, m_nOffset, m_nStartInstance);
	else
		pd3dCommandList->DrawInstanced(m_nVertices, m_nInstance, m_nOffset, m_nStartInstance);
}



//
CCubeMeshDiffused::CCubeMeshDiffused(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, float fWidth, float fHeight, float fDepth, XMFLOAT4 color)  : CMesh()
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	//정점 정보 추가
	m_nOffset = pVertex.size();
	pVertex.emplace_back(XMFLOAT3(-fx, +fy, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, +fy, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, +fy, +fz), color);
	pVertex.emplace_back(XMFLOAT3(-fx, +fy, +fz), color);
	pVertex.emplace_back(XMFLOAT3(-fx, -fy, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, -fy, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, -fy, +fz), color);
	pVertex.emplace_back(XMFLOAT3(-fx, -fy, +fz), color);
	m_nVertices = pVertex.size() - m_nOffset;


	//인덱스 정보 추가
	if (pMesh) {
		m_nStartIndex = pMesh->m_nStartIndex;
		m_nIndices = pMesh->m_nIndices;
	}
	else {
		m_nStartIndex = pIndex.size();
		pIndex.emplace_back(3); pIndex.emplace_back(1); pIndex.emplace_back(0);
		pIndex.emplace_back(2); pIndex.emplace_back(1); pIndex.emplace_back(3);
		pIndex.emplace_back(0); pIndex.emplace_back(5); pIndex.emplace_back(4);
		pIndex.emplace_back(1); pIndex.emplace_back(5); pIndex.emplace_back(0);
		pIndex.emplace_back(3); pIndex.emplace_back(4); pIndex.emplace_back(7);
		pIndex.emplace_back(0); pIndex.emplace_back(4); pIndex.emplace_back(3);
		pIndex.emplace_back(1); pIndex.emplace_back(6); pIndex.emplace_back(5);
		pIndex.emplace_back(2); pIndex.emplace_back(6); pIndex.emplace_back(1);
		pIndex.emplace_back(2); pIndex.emplace_back(7); pIndex.emplace_back(6);
		pIndex.emplace_back(3); pIndex.emplace_back(7); pIndex.emplace_back(2);
		pIndex.emplace_back(6); pIndex.emplace_back(4); pIndex.emplace_back(5);
		pIndex.emplace_back(7); pIndex.emplace_back(4); pIndex.emplace_back(6);
		m_nIndices = pIndex.size() - m_nStartIndex;
	}
}
CCubeMeshDiffused::~CCubeMeshDiffused()
{
}

//

CTreeMesh::CTreeMesh(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, float fWidth, float fHeight, float fDepth) : CMesh()
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	float fx = fWidth * 0.5f, fy = fHeight * 0.4f, fz = fDepth * 0.5f;



	//정점 정보 추가
	m_nOffset = pVertex.size();
	pVertex.emplace_back(XMFLOAT3(-fx, +fHeight, -fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, +fHeight, -fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, +fHeight, +fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(-fx, +fHeight, +fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(-fx, +fy, -fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, +fy, -fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, +fy, +fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(-fx, +fy, +fz), XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f));

	fx = fWidth * 0.2f; fz = fDepth * 0.2f;
	pVertex.emplace_back(XMFLOAT3(-fx, +fy, -fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, +fy, -fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, +fy, +fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(-fx, +fy, +fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(-fx, 0, -fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, 0, -fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(+fx, 0, +fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	pVertex.emplace_back(XMFLOAT3(-fx, 0, +fz), XMFLOAT4(0.6f, 0.2f, 0.0f, 1.0f));
	m_nVertices = pVertex.size() - m_nOffset;


	//인덱스 정보 추가
	if (pMesh) {
		m_nStartIndex = pMesh->m_nStartIndex;
		m_nIndices = pMesh->m_nIndices;
	}
	else {
		m_nStartIndex = pIndex.size();
		pIndex.emplace_back(3); pIndex.emplace_back(1); pIndex.emplace_back(0);
		pIndex.emplace_back(2); pIndex.emplace_back(1); pIndex.emplace_back(3);
		pIndex.emplace_back(0); pIndex.emplace_back(5); pIndex.emplace_back(4);
		pIndex.emplace_back(1); pIndex.emplace_back(5); pIndex.emplace_back(0);
		pIndex.emplace_back(3); pIndex.emplace_back(4); pIndex.emplace_back(7);
		pIndex.emplace_back(0); pIndex.emplace_back(4); pIndex.emplace_back(3);
		pIndex.emplace_back(1); pIndex.emplace_back(6); pIndex.emplace_back(5);
		pIndex.emplace_back(2); pIndex.emplace_back(6); pIndex.emplace_back(1);
		pIndex.emplace_back(2); pIndex.emplace_back(7); pIndex.emplace_back(6);
		pIndex.emplace_back(3); pIndex.emplace_back(7); pIndex.emplace_back(2);
		pIndex.emplace_back(6); pIndex.emplace_back(4); pIndex.emplace_back(5);
		pIndex.emplace_back(7); pIndex.emplace_back(4); pIndex.emplace_back(6);

		pIndex.emplace_back(11); pIndex.emplace_back(9); pIndex.emplace_back(8);
		pIndex.emplace_back(10); pIndex.emplace_back(9); pIndex.emplace_back(11);
		pIndex.emplace_back(8); pIndex.emplace_back(13); pIndex.emplace_back(12);
		pIndex.emplace_back(9); pIndex.emplace_back(13); pIndex.emplace_back(8);
		pIndex.emplace_back(11); pIndex.emplace_back(12); pIndex.emplace_back(15);
		pIndex.emplace_back(8); pIndex.emplace_back(12); pIndex.emplace_back(11);
		pIndex.emplace_back(9); pIndex.emplace_back(14); pIndex.emplace_back(13);
		pIndex.emplace_back(10); pIndex.emplace_back(14); pIndex.emplace_back(9);
		pIndex.emplace_back(10); pIndex.emplace_back(15); pIndex.emplace_back(14);
		pIndex.emplace_back(11); pIndex.emplace_back(15); pIndex.emplace_back(10);
		pIndex.emplace_back(14); pIndex.emplace_back(12); pIndex.emplace_back(13);
		pIndex.emplace_back(15); pIndex.emplace_back(12); pIndex.emplace_back(14);
		m_nIndices = pIndex.size() - m_nStartIndex;
	}
}
CTreeMesh::~CTreeMesh() {}

//

CAirplaneMeshDiffused::CAirplaneMeshDiffused(std::vector<CDiffusedVertex>& pVertex, float fWidth, float fHeight, float fDepth, XMFLOAT4 xmf4Color) : CMesh()
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);

	//정점 정보 추가
	m_nOffset = pVertex.size();

	//비행기 메쉬의 위쪽 면
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	//비행기 메쉬의 아래쪽 면
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	//비행기 메쉬의 오른쪽 면
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	//비행기 메쉬의 뒤쪽/오른쪽 면
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(+x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	//비행기 메쉬의 왼쪽 면
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, +(fy + y3), 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x2, +y2, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	//비행기 메쉬의 뒤쪽/왼쪽 면
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(0.0f, 0.0f, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-x1, -y1, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, 0), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertex.emplace_back(XMFLOAT3(-fx, -y3, -fDepth), Vector4::Add(xmf4Color, RANDOM_COLOR));

	m_nVertices = pVertex.size() - m_nOffset;
}
CAirplaneMeshDiffused::~CAirplaneMeshDiffused()
{
}

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	BYTE *pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_fMaxHeight = -FLT_MAX;

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapPixels[x + (y*m_nWidth)];
			if (m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)] > m_fMaxHeight)
				m_fMaxHeight = m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)];
		}
	}
	if (pHeightMapPixels) 
		delete[] pHeightMapPixels;
}
CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) 
		delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}
XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	//범위밖이면 월드좌표 기준 Y축 반환
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength))
		return XMFLOAT3(0.0f, 1.0f, 0.0f);

	//테두리 점이면 오프셋 반전
	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;

	//세 점을 구해 노말 계산
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;

	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return xmf3Normal;
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER
float CHeightMapImage::GetHeight(float fx, float fz)
{
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) 
		return 0.0f;

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;
	float fBottomLeft = (float)m_pHeightMapPixels[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1)*m_nWidth)];

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	bool bRightToLeft = ((z % 2) != 0);
	if (bRightToLeft)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;
	return fHeight;
}

CHeightMapGridMesh::CHeightMapGridMesh(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void *pContext)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	m_nOffset = pVertex.size();
	for (int z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++)
		{
			//정점의 높이와 색상을 높이 맵으로부터 구한다. 
			XMFLOAT3 xmf3Position = XMFLOAT3((x*m_xmf3Scale.x), OnGetHeight(x, z, pContext), (z*m_xmf3Scale.z));
			XMFLOAT4 xmf3Color = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);
			pVertex.emplace_back(CDiffusedVertex(xmf3Position, xmf3Color));
		}
	}
	m_nVertices = pVertex.size() - m_nOffset;

	//인덱스 삽입
	if (pMesh) {
		m_nStartIndex = pMesh->m_nStartIndex;
		m_nIndices = pMesh->m_nIndices;
	}
	else {
		m_nStartIndex = pIndex.size();
		for (int z = 0; z < nLength - 1; z++)
		{
			if ((z % 2) == 0)
			{
				for (int x = 0; x < nWidth; x++)
				{
					//더미 인덱스 추가
					if ((x == 0) && (z > 0))
						pIndex.emplace_back((UINT)(x + (z * nWidth)));
					pIndex.emplace_back((UINT)(x + (z * nWidth)));
					pIndex.emplace_back((UINT)((x + (z * nWidth)) + nWidth));
				}
			}
			else
			{
				for (int x = nWidth - 1; x >= 0; x--)
				{
					//더미 인덱스 추가
					if (x == (nWidth - 1))
						pIndex.emplace_back((UINT)(x + (z * nWidth)));
					pIndex.emplace_back((UINT)(x + (z * nWidth)));
					pIndex.emplace_back((UINT)((x + (z * nWidth)) + nWidth));
				}
			}
		}
		m_nIndices = pIndex.size() - m_nStartIndex;
	}
}
CHeightMapGridMesh::~CHeightMapGridMesh()
{
}
float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
	return fHeight;
}
XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);

	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z),xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z),xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1),xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1),xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return xmf4Color;
}

//

CCubeMeshTop::CCubeMeshTop(std::vector<CDiffusedVertex>& pVertex, std::vector<UINT>& pIndex, CMesh* pMesh, float fWidth , float fDepth, XMFLOAT4 color) {
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fz = fDepth * 0.5f;

	//정점 정보 추가
	m_nOffset = pVertex.size();
	pVertex.emplace_back(XMFLOAT3(-fx, 0.0f, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, 0.0f, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, 0.0f, +fz), color);
	pVertex.emplace_back(XMFLOAT3(-fx, 0.0f, +fz), color);
	pVertex.emplace_back(XMFLOAT3(-fx, -1.0f, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, -1.0f, -fz), color);
	pVertex.emplace_back(XMFLOAT3(+fx, -1.0f, +fz), color);
	pVertex.emplace_back(XMFLOAT3(-fx, -1.0f, +fz), color);
	m_nVertices = pVertex.size() - m_nOffset;

	//인덱스 정보 추가
	if (pMesh) {
		m_nStartIndex = pMesh->m_nStartIndex;
		m_nIndices = pMesh->m_nIndices;
	}
	else {
		m_nStartIndex = pIndex.size();
		pIndex.emplace_back(3); pIndex.emplace_back(1); pIndex.emplace_back(0);
		pIndex.emplace_back(2); pIndex.emplace_back(1); pIndex.emplace_back(3);
		pIndex.emplace_back(0); pIndex.emplace_back(5); pIndex.emplace_back(4);
		pIndex.emplace_back(1); pIndex.emplace_back(5); pIndex.emplace_back(0);
		pIndex.emplace_back(3); pIndex.emplace_back(4); pIndex.emplace_back(7);
		pIndex.emplace_back(0); pIndex.emplace_back(4); pIndex.emplace_back(3);
		pIndex.emplace_back(1); pIndex.emplace_back(6); pIndex.emplace_back(5);
		pIndex.emplace_back(2); pIndex.emplace_back(6); pIndex.emplace_back(1);
		pIndex.emplace_back(2); pIndex.emplace_back(7); pIndex.emplace_back(6);
		pIndex.emplace_back(3); pIndex.emplace_back(7); pIndex.emplace_back(2);
		pIndex.emplace_back(6); pIndex.emplace_back(4); pIndex.emplace_back(5);
		pIndex.emplace_back(7); pIndex.emplace_back(4); pIndex.emplace_back(6);
		m_nIndices = pIndex.size() - m_nStartIndex;
	}
}
CCubeMeshTop::~CCubeMeshTop() {}