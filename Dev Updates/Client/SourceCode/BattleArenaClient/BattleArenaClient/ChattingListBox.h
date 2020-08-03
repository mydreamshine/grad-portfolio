#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <string>

#include "Common/Util/d3d12/MathHelper.h"
#include "Common/Timer/Timer.h"
#include "Common/FileLoader/SpriteFontLoader.h"
#include "WND_MessageBlock.h"
#include "Object.h"
#include "FrameworkEvent.h"

class ChattingListBox
{
public:
	void Init(
		float newWidth = 0.0f, float newHeight = 0.0f,
		DXTK_FONT* newFont = nullptr,
		Object* newTextRenderObj = nullptr,
		wchar_t newLineTabChararcter = L'	',
		size_t newMaxChattingLog = 20);

	void SetTextRenderObj(Object* obj);
	void SetFont(DXTK_FONT* font);
	void SetMaxChattingLog(size_t newMaxChattingLog);
	void SetLineTabCharacter(wchar_t newLineTabCharacter);

	void SetMaxSize(float newWidth, float newHeight);
	void SetPosition(DirectX::XMFLOAT2 newPosition);

	DirectX::XMFLOAT2 GetSize();
	DirectX::XMFLOAT2 GetPosition();

	bool IsSizeChanged();

	void AddMessage(const std::wstring& newMessage);

	void RenderTextUpdate();
	void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

private:
	float m_MaxWidth = 0.0f;
	float m_MaxHeight = 0.0f;
	WND_MessageBlock m_RenderTextBlock;
	wchar_t m_LineTabChararcter = L'\0';

	bool m_ScrollingIntervalAct = false;
	const float m_ScrollingInterval = 0.06f;
	float m_ScrollingTimeStack = 0.0f;

	size_t m_MaxChattingLog = 20;
	std::list<std::wstring> m_ChattingMessageList;

	bool SizeChanged = false;

private:
	Object* m_TextRenderObj = nullptr;
	DXTK_FONT* m_Font = nullptr;
};
