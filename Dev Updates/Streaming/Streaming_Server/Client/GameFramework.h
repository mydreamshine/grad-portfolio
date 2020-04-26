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

	//타이머
	CGameTimer m_GameTimer;
	//FPS 출력용 버퍼
	_TCHAR m_pszFrameRate[50];
	//씬
	CScene *m_pScene = NULL;
	//카메라
	CCamera *m_pCamera = NULL;
	//플레이어
	CPlayer *m_pPlayer = NULL;
	//커서 마지막 위치 
	POINT m_ptOldCursorPos; 


	//팩토리 포인터
	IDXGIFactory4			*m_pdxgiFactory;
	//스왑체인 포인터
	IDXGISwapChain3			*m_pdxgiSwapChain;
	//디바이스 포인터
	ID3D12Device			*m_pd3dDevice;

	//다중 샘플링
	bool					m_bMsaa4xEnable				= false;
	UINT					m_nMsaa4xQualityLevels		= 0;

	//후면버퍼의 갯수
	static const UINT		m_nSwapChainBuffers			= 2;
	//현재 후면버퍼 인덱스
	UINT					m_nSwapChainBufferIndex;

	//렌더타겟버퍼, 서술자 힙 포인터, 렌더타겟 서술자 크기
	ID3D12Resource			*m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap	*m_pd3dRtvDescriptorHeap;
	UINT					m_nRtvDescriptorIncrementSize;

	//뎁스스텐실버퍼 -
	ID3D12Resource			*m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap	*m_pd3dDsvDescriptorHeap;
	UINT					m_nDsvDescriptorIncrementSize;

	//명령 큐, 할당자, 리스트 포인터
	ID3D12CommandQueue		*m_pd3dCommandQueue;
	ID3D12CommandAllocator	*m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList *m_pd3dCommandList;

	//PSO 포인터
	ID3D12PipelineState		*m_pd3dPipelineState;

	//펜스 포인터, 값, 핸들
	ID3D12Fence				*m_pd3dFence;
	UINT64					m_nFenceValues[m_nSwapChainBuffers];
	HANDLE					m_hFenceEvent;

	//스크린샷버퍼
	ID3D12Resource* m_ppd3dScreenshotBuffer;
	BYTE* m_pData;
	int dataLength;

	#if defined(_DEBUG)
		ID3D12Debug *m_pd3dDebugController;
	#endif

public:
	CGameFramework();
	~CGameFramework();

	//프레임워크 초기화
	bool OnCreate();
	void OnDestroy();

	//스왑 체인, 디바이스, 디스크립터 힙, 명령 큐, 할당자, 리스트 생성
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetView();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	//전체화면 전환
	void ChangeSwapChainState();

	//렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수
	void BuildObjects();
	void ReleaseObjects();

	//입력, 애니메이션, 렌더링
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	//CPU, GPU간 명시적 동기화
	void WaitForGpuComplete();
	void WaitForGpuCompleteSync();

	//윈도우 메세지 처리
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//
	void MoveToNextFrame();
	BYTE* CGameFramework::GetFrameData();
	HANDLE GetFenceEvent();
};