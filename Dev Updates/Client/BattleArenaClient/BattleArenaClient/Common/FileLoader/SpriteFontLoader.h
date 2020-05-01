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
class DXTK_FONT
{
	std::unique_ptr<DirectX::SpriteFont> m_Font;
	std::unique_ptr<DirectX::DescriptorHeap> m_FontDescriptors;
	std::unique_ptr<DirectX::SpriteBatch> m_FontSpriteBatch;
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicMemory;

public:
	// .spritefont������ ���� fontsprite �ؽ��� �� Descriptors�� ����
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

		// Make SpriteBatch
		{
			DirectX::ResourceUploadBatch SpriteBatch_upload(device);
			SpriteBatch_upload.Begin(); // ��ü������ CommandAllocator�� CommadList�� ����

			m_FontSpriteBatch = std::make_unique<DirectX::SpriteBatch>(device, SpriteBatch_upload, psoDesc, viewport);

			auto uploadResourcesFinished = SpriteBatch_upload.End(commandQueue); // CommadList ���� �� FenceEvent �߻�
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