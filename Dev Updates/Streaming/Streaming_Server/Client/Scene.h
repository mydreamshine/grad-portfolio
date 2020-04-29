#pragma once
#include "Timer.h"
#include "Shader.h"
#include "Camera.h"
#include "Player.h"

class CScene
{
public:
	CScene();
	~CScene();

	bool ProcessInput();
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, D3D12_CPU_DESCRIPTOR_HANDLE *pHandle);

	void ReleaseUploadBuffers();

	CPlayer *getPlayer();

	//��Ʈ�ñ׳�ó ����
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature();

	bool mapOut();
	void Renew();
	void SetPillar(CGameObject *pRail, CGameObject *pPillar, XMFLOAT3 xmfOffset);

protected:
	//���̴�
	//CInstancingShader * *m_ppShaders = NULL;
	CShader **m_ppShaders = NULL;
	int m_nShaders = 0;

	//�޽�
	CMesh **m_ppMeshs = NULL;
	int m_nMeshs = 0;

	//���Ͽ� �ε���
	int m_nFirst = 0;
	int m_nDest = 0;
	int m_nEnd = 0;

	//���� ȸ����
	float m_Degree = 5.0f;
	int m_Pitch = 0;
	int m_Yaw = 0;
	int m_Roll = 0;

	//���� ����
	int m_nWidth;
	int m_nHeight;

	//�� ������Ʈ
	CGameObject **m_ppObjects = NULL;
	int m_nObjects = 0;

	//��
	CHeightMapTerrain *m_pTerrain = NULL;

	//����
	CGameObject **m_ppRails = NULL;
	int m_nRails = 0;

	//����
	CGameObject **m_ppTrees = NULL;
	int m_nTrees = 0;

	//��
	CBirdObject **m_ppBirds = NULL;
	int m_nBirds = 0;

	//�÷��̾�
	CPlayer *m_pPlayer = NULL;
	int m_nLife = 3;

	//���
	CGameObject **m_ppPillars = NULL;
	int m_nPillars = 0;

	//������ �������ۿ� �ε��� ���� ����
	std::vector<UINT> m_Index;
	std::vector<CDiffusedVertex> m_Vertex;

	ID3D12Resource * m_pd3dVertexBuffer = NULL;
	ID3D12Resource *m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;

	ID3D12Resource *m_pd3dIndexBuffer = NULL;
	ID3D12Resource *m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	ID3D12RootSignature *m_pd3dGraphicsRootSignature = NULL;
};