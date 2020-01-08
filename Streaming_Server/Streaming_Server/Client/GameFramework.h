#pragma once
//#define _WITH_PLAYER_TOP
#pragma comment(lib, "ws2_32")

#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include <WS2tcpip.h>

class CGameFramework
{
private:
	int						m_nWndClientWidth;
	int						m_nWndClientHeight;

	SOCKET					m_listenSocket;
	SOCKET					m_clientSocket;

	//Ÿ�̸�
	CGameTimer m_GameTimer;
	//FPS ��¿� ����
	_TCHAR m_pszFrameRate[50];
	//��
	CScene *m_pScene = NULL;
	//ī�޶�
	CCamera *m_pCamera = NULL;
	//�÷��̾�
	CPlayer *m_pPlayer = NULL;
	//Ŀ�� ������ ��ġ 
	POINT m_ptOldCursorPos; 


	//���丮 ������
	IDXGIFactory4			*m_pdxgiFactory;
	//����ü�� ������
	IDXGISwapChain3			*m_pdxgiSwapChain;
	//����̽� ������
	ID3D12Device			*m_pd3dDevice;

	//���� ���ø�
	bool					m_bMsaa4xEnable				= false;
	UINT					m_nMsaa4xQualityLevels		= 0;

	//�ĸ������ ����
	static const UINT		m_nSwapChainBuffers			= 2;
	//���� �ĸ���� �ε���
	UINT					m_nSwapChainBufferIndex;

	//����Ÿ�ٹ���, ������ �� ������, ����Ÿ�� ������ ũ��
	ID3D12Resource			*m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap	*m_pd3dRtvDescriptorHeap;
	UINT					m_nRtvDescriptorIncrementSize;

	//�������ٽǹ��� -
	ID3D12Resource			*m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap	*m_pd3dDsvDescriptorHeap;
	UINT					m_nDsvDescriptorIncrementSize;

	//��� ť, �Ҵ���, ����Ʈ ������
	ID3D12CommandQueue		*m_pd3dCommandQueue;
	ID3D12CommandAllocator	*m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList *m_pd3dCommandList;

	//PSO ������
	ID3D12PipelineState		*m_pd3dPipelineState;

	//�潺 ������, ��, �ڵ�
	ID3D12Fence				*m_pd3dFence;
	UINT64					m_nFenceValues[m_nSwapChainBuffers];
	HANDLE					m_hFenceEvent;

	//��ũ��������
	ID3D12Resource* m_ppd3dScreenshotBuffer;
	BYTE* m_pData;
	int dataLength;

	#if defined(_DEBUG)
		ID3D12Debug *m_pd3dDebugController;
	#endif

public:
	CGameFramework();
	~CGameFramework();

	//�����ӿ�ũ �ʱ�ȭ
	bool OnCreate();
	void OnDestroy();

	//���� ü��, ����̽�, ��ũ���� ��, ��� ť, �Ҵ���, ����Ʈ ����
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetView();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	//��üȭ�� ��ȯ
	void ChangeSwapChainState();

	//�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ�
	void BuildObjects();
	void ReleaseObjects();

	//�Է�, �ִϸ��̼�, ������
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	//CPU, GPU�� ����� ����ȭ
	void WaitForGpuComplete();
	void WaitForGpuCompleteSync();

	//������ �޼��� ó��
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//
	void MoveToNextFrame();
	BYTE* CGameFramework::GetFrameData();
	HANDLE GetFenceEvent();
};