//#pragma once
//#include <wrl.h>
//#include <wrl/client.h>
//
//#include <dxgi1_4.h>
//#include <d3d12.h>
//
//using namespace Microsoft::WRL;
//
//class GraphicsPipeline
//{
//private:
//	int						m_nWndClientWidth;
//	int						m_nWndClientHeight;
//
//	ComPtr<IDXGIFactory4> m_pdxgiFactory;
//	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;
//	ComPtr<ID3D12Device> m_pd3dDevice;
//
//	bool					m_bMsaa4xEnable = false;
//	UINT					m_nMsaa4xQualityLevels = 0;
//
//	static const UINT		m_nSwapChainBuffers = 4;
//	UINT					m_nSwapChainBufferIndex;
//
//	//렌더타겟버퍼, 서술자 힙 포인터, 렌더타겟 서술자 크기
//	ComPtr<ID3D12Resource> m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
//	ComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescriptorHeap;
//	UINT					m_nRtvDescriptorIncrementSize;
//
//	//뎁스스텐실버퍼 -
//	ComPtr<ID3D12Resource> m_pd3dDepthStencilBuffer;
//	ComPtr<ID3D12DescriptorHeap> m_pd3dDsvDescriptorHeap;
//	UINT					m_nDsvDescriptorIncrementSize;
//
//	//PSO 포인터
//	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;
//
//#if defined(_DEBUG)
//	ComPtr<ID3D12Debug> m_pd3dDebugController;
//#endif
//
//public:
//	GraphicsPipeline();
//	~GraphicsPipeline();
//
//	bool OnCreate();
//	void CreateSwapChain();
//	void CreateDirect3DDevice();
//	void CreateRtvAndDsvDescriptorHeaps();
//	void CreateRenderTargetView();
//	void CreateDepthStencilView();
//	void CreateCommandQueueAndList();
//};
//
