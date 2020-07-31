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


// ��ü������ Font�� ���� Resource �� Descriptor�� ���� �� ����
// m_graphicMemory�� ���� commandAllocator ���� ������ ���ϰ� �ִ�.
// �������� �ؽ�Ʈ���� SpriteBatch�� ��������� �Ѵ�.
class DXTK_FONT
{
	std::unique_ptr<DirectX::SpriteFont> m_Font;
	std::unique_ptr<DirectX::DescriptorHeap> m_FontDescriptors;

public:
	// Text�� �������� �� TextPos�� ��������
	// Text_Width��ŭ Offset��ų ���� �������ش�.
	// ex) Center: TextPos.x += Text_Width / 2
	//     Right:  TextPos.x += Text_Width
	//     Left:   TextPos.x += 0
	enum class TEXT_PIVOT
	{
		LEFT, CENTER, RIGHT
	};

public:
	// .spritefont������ ���� fontsprite �ؽ��� �� Descriptors�� ����
	DXTK_FONT(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const std::wstring& font_filepath)
	{
		m_FontDescriptors = std::make_unique<DirectX::DescriptorHeap>(device,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			1);

		// ResourceUploadBatch �߿��� �ٸ� ���ε尡 ����� �� ����
		// ��� �Ʒ��� ���� SpriteFont_upload�� SpriteBatch_upload ������
		// ���� �и����ش�.

		// Make SpriteFont
		{
			DirectX::ResourceUploadBatch SpriteFont_upload(device);
			SpriteFont_upload.Begin(); // ��ü������ CommandAllocator�� CommadList�� ����

			m_Font = std::make_unique<DirectX::SpriteFont>(device, SpriteFont_upload, font_filepath.c_str(),
				m_FontDescriptors->GetCpuHandle(0),
				m_FontDescriptors->GetGpuHandle(0));

			// ResourceUploadBatch�� ���� ��ü������ ������ CommadList�� Close�ϰ�
			// �Ķ���ͷ� ���޹��� commandQueue�� CommandList�� �����ϰ� �����Ų��.
			// �̶�, ��ü������ Fence�� Fenece Event�� �����Ͽ�
			// Fence�� ������ ��ٸ��� �ȴ�.
			// Fence�� ���� CommandList�� ��� ����� ���� ���� Ȯ�εǸ�
			// commandQueue�� ������ ResourceUploadBatch ���� �������� ���� �����ȴ�.
			auto uploadResourcesFinished = SpriteFont_upload.End(commandQueue); // CommadList ���� �� FenceEvent �߻�
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