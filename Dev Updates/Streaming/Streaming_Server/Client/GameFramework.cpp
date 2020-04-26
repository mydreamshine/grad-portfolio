#include "stdafx.h"
#include "GameFramework.h"

#include <fstream>
#include <Wincodec.h>
#include <memory>
#include <atlbase.h>

CGameFramework::CGameFramework()
{
	//씬 설정
	m_pScene = NULL;

	m_pdxgiFactory					= NULL;
	m_pdxgiSwapChain				= NULL;
	m_pd3dDevice					= NULL;

	m_pd3dCommandAllocator			= NULL;
	m_pd3dCommandQueue				= NULL;
	m_pd3dCommandList				= NULL;
	m_pd3dPipelineState				= NULL;

	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_ppd3dRenderTargetBuffers[i] = NULL;
	m_pd3dRtvDescriptorHeap			= NULL;
	m_nRtvDescriptorIncrementSize	= 0;

	m_pd3dDepthStencilBuffer		= NULL;
	m_pd3dDsvDescriptorHeap			= NULL;
	m_nDsvDescriptorIncrementSize	= 0;

	m_nSwapChainBufferIndex			= 0;

	m_hFenceEvent					= NULL;
	m_pd3dFence						= NULL;
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_nFenceValues[i] = 0;

	m_nWndClientWidth				= FRAME_BUFFER_WIDTH;
	m_nWndClientHeight				= FRAME_BUFFER_HEIGHT;

	m_ppd3dScreenshotBuffer = NULL;
	m_pData = NULL;
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate() {
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	//CreateSwapChain();
	CreateRenderTargetView();
	CreateDepthStencilView();

	BuildObjects();
	return true;
}
void CGameFramework::OnDestroy() {
	ReleaseObjects();

	CloseHandle(m_hFenceEvent);

	#if defined(_DEBUG)
		if (m_pd3dDebugController) m_pd3dDebugController->Release();
	#endif

	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		if(m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

	//스크린샷 버퍼
	if (m_ppd3dScreenshotBuffer) m_ppd3dScreenshotBuffer->Release();
	if (m_pData) delete[] m_pData;
}
void CGameFramework::CreateSwapChain()
{
	//RECT rcClient;
	//GetClientRect(m_hWnd, &rcClient);
	//m_nWndClientWidth = rcClient.right - rcClient.left;
	//m_nWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = 0;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = FALSE;

	//바탕화면을 윈도우 크기에 맞게 변경
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
	/*m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);*/
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();


}
void CGameFramework::CreateDirect3DDevice() {
#if defined(_DEBUG)
	D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&m_pd3dDebugController);
	m_pd3dDebugController->EnableDebugLayer();
#endif

	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&m_pdxgiFactory);
	
	//디바이스 생성
	IDXGIAdapter1 *pdxgiAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pdxgiAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pdxgiAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pdxgiAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	//디바이스 생성 실패시 WARP 생성
	if (!pdxgiAdapter) {
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**)&pdxgiAdapter);
		D3D12CreateDevice(pdxgiAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	//어댑터 해제
	if (pdxgiAdapter) pdxgiAdapter->Release();

	//멀티샘플링 체크
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(d3dMsaaQualityLevels));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	//펜스 생성
	m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_nFenceValues[i] = 0;
	m_hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}
void CGameFramework::CreateCommandQueueAndList() {
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));

	//DIRECT 타입으로 생성
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	//커맨드 시리즈 생성
	HRESULT hResult;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);

	//최초생성 시 열림상태 -> 닫힘으로 전환
	hResult = m_pd3dCommandList->Close();
}
void CGameFramework::CreateRtvAndDsvDescriptorHeaps() {
	//RTV DESC 생성 및 설정
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	//RTV Heap 생성
	m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	//RTV 사이즈 저장
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//DSV DESC 설정
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	//DSV Heap 생성
	m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	//DSV 사이즈 저장
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}
void CGameFramework::CreateRenderTargetView() {
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	//스왑체인에서 버퍼를 받아오고, 서술자 생성
	for (UINT i = 0; i < m_nSwapChainBuffers; ++i) {
		//HRESULT hResult = m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);

		//힙 서술
		D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
		ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
		d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		d3dHeapPropertiesDesc.CreationNodeMask = 1;
		d3dHeapPropertiesDesc.VisibleNodeMask = 1;

		//리소스 서술
		D3D12_RESOURCE_DESC d3dResourceDesc;
		ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
		d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		d3dResourceDesc.Alignment = 65536;
		d3dResourceDesc.Width = FRAME_BUFFER_WIDTH;
		d3dResourceDesc.Height = FRAME_BUFFER_HEIGHT;
		d3dResourceDesc.DepthOrArraySize = 1;
		d3dResourceDesc.MipLevels = 1;
		d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dResourceDesc.SampleDesc.Count = 1;
		d3dResourceDesc.SampleDesc.Quality = 0;
		d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		//d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
		HRESULT hResult = m_pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)& m_ppd3dRenderTargetBuffers[i]);

		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}

	//스크린샷 버퍼 생성
	CoInitialize(NULL);
	dataLength = m_nWndClientWidth * m_nWndClientHeight * 4;
	m_ppd3dScreenshotBuffer = CreateBufferResource(m_pd3dDevice, m_pd3dCommandList, NULL, dataLength, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
	m_pData = new BYTE[dataLength];
}
void CGameFramework::CreateDepthStencilView() {
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels-1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	//DSB 생성
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	//DSV 생성
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::BuildObjects() {
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	//씬 생성
	m_pScene = new CScene();
	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	//플레이어 생성
	m_pPlayer = m_pScene->getPlayer();
	m_pCamera = m_pPlayer->GetCamera();

	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuCompleteSync();
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}
void CGameFramework::ReleaseObjects() {
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다. 
		SetCapture(hWnd);
		GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//마우스 캡쳐를 해제한다. 
		ReleaseCapture();
		break;
	}
}
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM	wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		}
		break;
	default:
		m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	}
}
LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		case WM_SIZE:
		{
			m_nWndClientWidth = LOWORD(lParam);
			m_nWndClientHeight = HIWORD(lParam);
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return 0;
}

void CGameFramework::ProcessInput() {
	}

void CGameFramework::AnimateObjects() {
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}
void CGameFramework::WaitForGpuComplete() {
	//CPU 펜스의 값을 증가한다. 
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	//GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다. 
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		//펜스 비동기 시 대기 시작
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
	}
}
void CGameFramework::WaitForGpuCompleteSync() {
	//CPU 펜스의 값을 증가한다. 
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	//GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다. 
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		//펜스 비동기 시 대기 시작
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}
void CGameFramework::FrameAdvance() {
	m_GameTimer.Tick(60.0f);

	ProcessInput();

	AnimateObjects();

	//명령 할당자와 명령 리스트를 리셋한다. 
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	//현재 렌더타겟 프레젠트 대기
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//현재의 DSV, 렌더 타겟 서술자의 핸들 계산 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =	m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex *	m_nRtvDescriptorIncrementSize);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTV, DSV를 OM에 연결
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	//RTV 초기화
	float pfClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,	pfClearColor/*Colors::Azure*/, 0, NULL);
	//DSV 초기화
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,	D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//렌더링 코드
	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera, &d3dDsvCPUDescriptorHandle);

	//스크린샷
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_TEXTURE_COPY_LOCATION dst, src;
	dst.pResource = m_ppd3dScreenshotBuffer;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dst.PlacedFootprint.Offset = 0;
	dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dst.PlacedFootprint.Footprint.Width = 640;
	dst.PlacedFootprint.Footprint.Height = 480;
	dst.PlacedFootprint.Footprint.Depth = 1;
	dst.PlacedFootprint.Footprint.RowPitch = m_nWndClientWidth *4;

	src.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	m_pd3dCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	//렌더링 대기
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//명령 리스트 닫기
	hResult = m_pd3dCommandList->Close();

	//명령 큐에 삽입
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//GPU 동기화
	WaitForGpuComplete();
}

HANDLE CGameFramework::GetFenceEvent()
{
	return m_hFenceEvent;
}
BYTE* CGameFramework::GetFrameData()
{
	D3D12_RANGE d3dReadRange = { 0, 0 };
	UINT8* pBufferDataBegin = NULL;
	m_ppd3dScreenshotBuffer->Map(0, &d3dReadRange, (void**)& pBufferDataBegin);
	memcpy(m_pData, pBufferDataBegin, dataLength);
	m_ppd3dScreenshotBuffer->Unmap(0, NULL);
	return m_pData;
}
void CGameFramework::ChangeSwapChainState() {
	//GPU 동기화
	WaitForGpuComplete();

	//현재 상태를 받아와서, 반대모드로 전환
	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	//타겟 윈도우 변경
	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	//스왑체인 참조 초기화
	for (int i = 0; i < m_nSwapChainBuffers; i++) 
		if (m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();

	//버퍼 변경
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetView();
}
void CGameFramework::MoveToNextFrame() {
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}