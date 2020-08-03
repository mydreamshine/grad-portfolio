#pragma once
#include <list>
#include <memory>
#include <vector>
#include <queue>
#include <string>

#include "Common/Util/d3d12/MathHelper.h"
#include "Common/Timer/Timer.h"
#include "WND_MessageBlock.h"
#include "Object.h"
#include "FrameworkEvent.h"

class ChattingList
{
public:
	void Init();

	void SetTextRenderObj(Object* obj);

	void SetMaxChattingLog(size_t newMaxChattingLog);
	void SetLineTabCharacter(wchar_t newLineTabCharacter);

	void SetSize(float newWidth, float newHeight);
	void SetPosition(DirectX::XMFLOAT2 newPosition);

	void AddMessage(const std::wstring& newMessage);

	void Update(CTimer& gt);
	void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

private:
	float Width = 0.0f;
	float Height = 0.0f;
	WND_MessageBlock ChattingListMessageBlock;
	wchar_t LineTabChararcter = L'\0';

	bool ChattingListUpdate = false;
	bool ChattingListScrolling = false;
	bool ChattingListScrollingIntervalAct = false;

	const float ChattingListScrollingInterval = 0.06f;
	float ChattingListScrollingTimeStack = 0.0f;

	size_t MaxChattingLog = 20;
	std::list<std::wstring> ChattingList;

private:
	Object* TextRenderObj = nullptr;
};