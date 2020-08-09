#include "ChattingListBox.h"

void ChattingListBox::Init(
	float newWidth, float newHeight,
	DXTK_FONT* newFont,
	Object* newTextRenderObj,
	wchar_t newLineTabChararcter,
	size_t newMaxChattingLog)
{
	m_MaxWidth = newWidth;
	m_MaxHeight = newHeight;
	m_RenderTextBlock.init(m_MaxWidth, m_MaxHeight);
	m_LineTabChararcter = newLineTabChararcter;

	m_ScrollingIntervalAct = false;
	m_ScrollingTimeStack = 0.0f;

	m_MaxChattingLog = newMaxChattingLog;
	m_ChattingMessageList.clear();

	if (m_TextRenderObj != nullptr)
		m_TextRenderObj->m_Textinfo->m_Text.clear();
	m_TextRenderObj = newTextRenderObj;
}

void ChattingListBox::SetTextRenderObj(Object* obj)
{
	m_TextRenderObj = obj;
}

void ChattingListBox::SetFont(DXTK_FONT* font)
{
	m_Font = font;
}

void ChattingListBox::SetMaxChattingLog(size_t newMaxChattingLog)
{
	m_MaxChattingLog = newMaxChattingLog;
}

void ChattingListBox::SetLineTabCharacter(wchar_t newLineTabCharacter)
{
	m_LineTabChararcter = newLineTabCharacter;
}

void ChattingListBox::SetMaxSize(float newWidth, float newHeight)
{
	m_MaxWidth = newWidth;
	m_MaxHeight = newHeight;

	m_RenderTextBlock.SetCuttingSize(m_MaxWidth, m_MaxHeight);
}

void ChattingListBox::SetPosition(DirectX::XMFLOAT2 newPosition)
{
	if (m_TextRenderObj == nullptr) return;

	auto& TextRenderObjPos = m_TextRenderObj->m_Textinfo->m_TextPos;
	TextRenderObjPos = newPosition;
}

DirectX::XMFLOAT2 ChattingListBox::GetSize()
{
	if (m_Font == nullptr) return DirectX::XMFLOAT2(0.0f, 0.0f);
	if (m_TextRenderObj == nullptr) return DirectX::XMFLOAT2(0.0f, 0.0f);

	auto& ChattingLogRenderText = m_TextRenderObj->m_Textinfo->m_Text;
	return m_Font->GetStringSize(ChattingLogRenderText);
}

DirectX::XMFLOAT2 ChattingListBox::GetPosition()
{
	if (m_TextRenderObj == nullptr) return DirectX::XMFLOAT2(0.0f, 0.0f);

	return m_TextRenderObj->m_Textinfo->m_TextPos;;
}

bool ChattingListBox::IsSizeChanged()
{
	bool PickUp = SizeChanged;
	SizeChanged = false;
	return PickUp;
}

void ChattingListBox::AddMessage(const std::wstring& newMessage)
{
	if (m_Font == nullptr) return;
	if (m_TextRenderObj == nullptr) return;

	if (m_ChattingMessageList.size() == m_MaxChattingLog) m_ChattingMessageList.pop_front();
	m_ChattingMessageList.push_back(newMessage);

	m_RenderTextBlock.AddMessageLine(m_ChattingMessageList.back(), m_Font, m_LineTabChararcter,
		WND_MessageBlock::SetMessageBlockPosInMessageLines::BOTTOM);

	ChattingListBox::RenderTextUpdate();
}

void ChattingListBox::RenderTextUpdate()
{
	auto& ChattingLogRenderText = m_TextRenderObj->m_Textinfo->m_Text;

	DirectX::XMFLOAT2 UpdatePrevSize = m_Font->GetStringSize(ChattingLogRenderText);
	ChattingLogRenderText = m_RenderTextBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageLines);
	DirectX::XMFLOAT2 UpdateAfterSize = m_Font->GetStringSize(ChattingLogRenderText);

	if (UpdatePrevSize.y != UpdateAfterSize.y) SizeChanged = true;
}

void ChattingListBox::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (key_state[VK_UP] == true || key_state[VK_DOWN] == true)
    {
		auto& ChattingLogRenderText = m_TextRenderObj->m_Textinfo->m_Text;
        int Offset = 0;

        if (key_state[VK_UP] == true) Offset = -1;
        else if (key_state[VK_DOWN] == true) Offset = +1;

        if (m_ScrollingIntervalAct == false)
        {
			m_RenderTextBlock.MoveStartIndex_InMessageLines(Offset, m_Font);
			ChattingListBox::RenderTextUpdate();

			m_ScrollingIntervalAct = true;
        }
        else
        {
            if (m_ScrollingTimeStack >= m_ScrollingInterval)
            {
				m_RenderTextBlock.MoveStartIndex_InMessageLines(Offset, m_Font);
				ChattingListBox::RenderTextUpdate();

				m_ScrollingTimeStack = 0.0f;
            }
            else m_ScrollingTimeStack += gt.GetTimeElapsed();
        }
    }
    else
    {
		m_ScrollingTimeStack = 0.0f;
		m_ScrollingIntervalAct = false;
    }
}
