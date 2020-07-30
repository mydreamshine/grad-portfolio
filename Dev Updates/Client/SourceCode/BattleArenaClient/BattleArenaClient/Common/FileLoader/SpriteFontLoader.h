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
// 렌더링할 텍스트마다 SpriteBatch를 지정해줘야 한다.
class DXTK_FONT
{
	std::unique_ptr<DirectX::SpriteFont> m_Font;
	std::unique_ptr<DirectX::DescriptorHeap> m_FontDescriptors;

public:
	// Text를 렌더링할 때 TextPos를 기준으로
	// Text_Width만큼 Offset시킬 양을 지정해준다.
	// ex) Center: TextPos.x += Text_Width / 2
	//     Right:  TextPos.x += Text_Width
	//     Left:   TextPos.x += 0
	enum class TEXT_PIVOT
	{
		LEFT, CENTER, RIGHT
	};

public:
	// .spritefont파일을 통해 fontsprite 텍스쳐 및 Descriptors를 생성
	DXTK_FONT(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const std::wstring& font_filepath)
	{
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
	}

	void DrawString(ID3D12GraphicsCommandList* commandList,
		DirectX::SpriteBatch* TextBatch,
		const DirectX::XMFLOAT2& text_Pos, const TEXT_PIVOT& text_Pivot, DirectX::XMVECTOR color,
		const std::wstring& text)
	{
		DirectX::SimpleMath::Vector2 origin = m_Font->MeasureString(text.c_str());
		switch (text_Pivot)
		{
		case TEXT_PIVOT::LEFT:   origin.x = 0.0f;     break;
		case TEXT_PIVOT::CENTER: origin.x /= 2.0f;    break;
		case TEXT_PIVOT::RIGHT:  origin.x = origin.x; break;
		}
		origin.y /= 2.0f;

		ID3D12DescriptorHeap* heaps[] = { m_FontDescriptors->Heap() };
		commandList->SetDescriptorHeaps(_countof(heaps), heaps);

		try {
			TextBatch->Begin(commandList, DirectX::SpriteSortMode_Deferred);

			m_Font->DrawString(TextBatch, text.c_str(),
				text_Pos, color, 0.f, origin);

			TextBatch->End();
		}
		catch (...) {
			abort();
		}
	}

	DirectX::XMFLOAT2 GetStringSize(const std::wstring& text)
	{
		DirectX::XMVECTOR STRING_SIZE = m_Font->MeasureString(text.c_str());
		DirectX::XMFLOAT2 StringSize; XMStoreFloat2(&StringSize, STRING_SIZE);
		return StringSize;
	}
};