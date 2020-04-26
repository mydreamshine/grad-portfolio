//#include "GraphicsPipeline.h"
//#include "DXSampleHelper.h"
//
//GraphicsPipeline::GraphicsPipeline() :
//	m_nRtvDescriptorIncrementSize(0),
//	m_nDsvDescriptorIncrementSize(0)
//{
//}
//
//GraphicsPipeline::~GraphicsPipeline()
//{
//}
//
//bool GraphicsPipeline::OnCreate()
//{
//	CreateDirect3DDevice();
//	CreateCommandQueueAndList();
//	CreateRtvAndDsvDescriptorHeaps();
//	CreateSwapChain();
//	CreateRenderTargetView();
//	CreateDepthStencilView();
//
//	return true;
//}
//
//void GraphicsPipeline::CreateDirect3DDevice()
//{
//#if defined(_DEBUG)
//	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pd3dDebugController))))
//	{
//		m_pd3dDebugController->EnableDebugLayer();
//	}
//#endif
//
//	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_pdxgiFactory)));
//
//	//디바이스 생성
//	ComPtr<IDXGIAdapter1> pdxgiAdapter = NULL;
//	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pdxgiAdapter); i++)
//	{
//		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
//		pdxgiAdapter->GetDesc1(&dxgiAdapterDesc);
//		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
//		if (SUCCEEDED(D3D12CreateDevice(pdxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pd3dDevice)))) break;
//	}
//
//	//디바이스 생성 실패시 WARP 생성
//	if (!pdxgiAdapter) {
//		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**)& pdxgiAdapter);
//		D3D12CreateDevice(pdxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pd3dDevice));
//	}
//
//	//멀티샘플링 체크
//	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
//	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	d3dMsaaQualityLevels.SampleCount = 4;
//	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
//	d3dMsaaQualityLevels.NumQualityLevels = 0;
//	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(d3dMsaaQualityLevels));
//	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
//	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
//}
//
//void GraphicsPipeline::CreateCommandQueueAndList()
//{
//
//}
