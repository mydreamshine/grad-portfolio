#pragma once
#include "../Util/DirectXTK12_srcinc/RenderTargetState.h"
#include "../Util/DirectXTK12_srcinc/DescriptorHeap.h"
#include "../Util/DirectXTK12_srcinc/ResourceUploadBatch.h"
#include "../Util/DirectXTK12_srcinc/SpriteBatch.h"
#include "../Util/DirectXTK12_srcinc/SpriteFont.h"
#include "../Util/DirectXTK12_srcinc/SimpleMath.h"

#include <memory>
#include <string>
#include <vector>


// 자체적으로 Font에 대한 Resource 및 Descriptor를 생성 및 관리
// m_graphicMemory를 통해 commandAllocator 또한 별도로 지니고 있다.
class DXTK_FONT
{
	std::unique_ptr<DirectX::SpriteFont> m_Font;
	std::unique_ptr<DirectX::DescriptorHeap> m_FontDescriptors;
	std::unique_ptr<DirectX::SpriteBatch> m_FontSpriteBatch;
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicMemory;

public:
	// .spritefont파일을 통해 fontsprite 텍스쳐 및 Descriptors를 생성
	DXTK_FONT(ID3D12Device* device, ID3D12CommandQueue* commandQueue,
		const D3D12_VIEWPORT* viewport,
		const DirectX::SpriteBatchPipelineStateDescription& psoDesc,
		const std::wstring& font_filepath)
	{
		m_graphicMemory = std::make_unique<DirectX::GraphicsMemory>(device);

		m_FontDescriptors = std::make_unique<DirectX::DescriptorHeap>(device,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			1);

		// ResourceUploadBatch 중에는 다른 업로드가 끼어들 수 없다
		// 고로 아래와 같이 SpriteFont_upload와 SpriteBatch_upload 구문을
		// 따로 분리해준다.

		// Make SpriteFont
		{
			DirectX::ResourceUploadBatch SpriteFont_upload(device);
			SpriteFont_upload.Begin(); // 자체적으로 CommandAllocator와 CommadList를 생성

			m_Font = std::make_unique<DirectX::SpriteFont>(device, SpriteFont_upload, font_filepath.c_str(),
				m_FontDescriptors->GetCpuHandle(0),
				m_FontDescriptors->GetGpuHandle(0));

			// ResourceUploadBatch를 통해 자체적으로 생성한 CommadList를 Close하고
			// 파라미터로 전달받은 commandQueue에 CommandList를 적재하고 실행시킨다.
			// 이때, 자체적으로 Fence와 Fenece Event를 생성하여
			// Fence값 증가를 기다리게 된다.
			// Fence를 통해 CommandList의 모든 명령이 끝난 것이 확인되면
			// commandQueue를 제외한 ResourceUploadBatch 관련 변수들이 전부 삭제된다.
			auto uploadResourcesFinished = SpriteFont_upload.End(commandQueue); // CommadList 실행 및 FenceEvent 발생
			uploadResourcesFinished.wait();
		}

		// Make SpriteBatch
		{
			DirectX::ResourceUploadBatch SpriteBatch_upload(device);
			SpriteBatch_upload.Begin(); // 자체적으로 CommandAllocator와 CommadList를 생성

			m_FontSpriteBatch = std::make_unique<DirectX::SpriteBatch>(device, SpriteBatch_upload, psoDesc, viewport);

			auto uploadResourcesFinished = SpriteBatch_upload.End(commandQueue); // CommadList 실행 및 FenceEvent 발생
			uploadResourcesFinished.wait();
		}
	}

	void DrawString(ID3D12GraphicsCommandList* commandList,
		const DirectX::XMFLOAT2& text_Pos, DirectX::XMVECTOR color,
		const std::wstring& text)
	{
		DirectX::SimpleMath::Vector2 origin = m_Font->MeasureString(text.c_str());
		origin.x /= 2.0f;
		origin.y /= 2.0f;

		ID3D12DescriptorHeap* heaps[] = { m_FontDescriptors->Heap() };
		commandList->SetDescriptorHeaps(_countof(heaps), heaps);

		try {
			m_FontSpriteBatch->Begin(commandList, DirectX::SpriteSortMode_Deferred);
			m_Font->DrawString(m_FontSpriteBatch.get(), text.c_str(),
				text_Pos, color, 0.f, origin);

			m_FontSpriteBatch->End();
		}
		catch (...) {
			abort();
		}
	}
};