#include "ChattingList.h"

void ChattingList::Init()
{
	Width = 0.0f;
	Height = 0.0f;
	ChattingListMessageBlock.init(Width, Height);
	LineTabChararcter = L'\0';

	ChattingListUpdate = false;
	ChattingListScrolling = false;
	ChattingListScrollingIntervalAct = false;

	ChattingListScrollingTimeStack = 0.0f;

	MaxChattingLog = 20;
	ChattingList.clear();

	TextRenderObj = nullptr;
}

void ChattingList::SetTextRenderObj(Object* obj)
{
	TextRenderObj = obj;
}

void ChattingList::SetMaxChattingLog(size_t newMaxChattingLog)
{
	MaxChattingLog = newMaxChattingLog;
}

void ChattingList::SetLineTabCharacter(wchar_t newLineTabCharacter)
{
	LineTabChararcter = newLineTabCharacter;
}

void ChattingList::SetSize(float newWidth, float newHeight)
{
	Width = newWidth;
	Height = newHeight;

	ChattingListMessageBlock.SetCuttingSize(Width, Height);
}

void ChattingList::SetPosition(DirectX::XMFLOAT2 newPosition)
{
	if (TextRenderObj == nullptr) return;

	auto& TextRenderObjPos = TextRenderObj->m_Textinfo->m_TextPos;
	TextRenderObjPos = newPosition;
}

void ChattingList::AddMessage(const std::wstring& newMessage)
{
	if (ChattingList.size() == MaxChattingLog) ChattingList.pop_front();
	ChattingList.push_back(newMessage);
}

void ChattingList::Update(CTimer& gt)
{
    if (TextRenderObj == nullptr) return;

    auto TextInfo = TextRenderObj->m_Textinfo.get();

}
