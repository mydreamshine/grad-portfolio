#pragma once
#include "../Util/DirectXTK/include/RenderTargetState.h"
#include "../Util/DirectXTK/include/SpriteBatch.h"
#include "../Util/DirectXTK/include/SpriteFont.h"
#include "../Util/DirectXTK/include/SimpleMath.h"

#include <memory>
#include <string>

class DXTK_FONT
{
	std::unique_ptr<DirectX::SpriteFont> m_font;
	std::unique_ptr <DirectX::SpriteBatch> m_spriteBatch;

	DXTK_FONT(ID3D12Device* device,
		DirectX::ResourceUploadBatch& SpriteFont_upload,
		DirectX::ResourceUploadBatch& SpriteBatch_upload,
		const DirectX::SpriteBatchPipelineStateDescription& psoDesc,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor,
		const std::string& font_name,
		bool forceSRGB = false,
		const D3D12_VIEWPORT* viewport = nullptr)
	{
		std::wstring wstr_font_name;
		wstr_font_name.assign(font_name.begin(), font_name.end());
		m_font = std::make_unique<DirectX::SpriteFont>(device, SpriteFont_upload, wstr_font_name.c_str(),
			cpuDescriptorDest, gpuDescriptor, forceSRGB);
		m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(device, SpriteBatch_upload, psoDesc, viewport);
	}

	void DrawString(ID3D12GraphicsCommandList* commandList,
		const DirectX::XMFLOAT2& text_Pos, DirectX::XMVECTOR color,
		const std::string& text)
	{
		std::wstring wstr_text;
		wstr_text.assign(text.begin(), text.end());
		DirectX::SimpleMath::Vector2 origin = m_font->MeasureString(wstr_text.c_str());

		m_spriteBatch->Begin(commandList, DirectX::SpriteSortMode_Deferred);
		m_font->DrawString(m_spriteBatch.get(), wstr_text.c_str(),
			text_Pos, color, 0.f, origin);
		m_spriteBatch->End();
	}
};