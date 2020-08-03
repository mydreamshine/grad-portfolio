#include "WND_MessageBlock.h"

const std::wstring& WND_MessageBlock::GetMessageBlock(DXTK_FONT* Font, MessageBlockFrom messageBlockFrom, int FromIndexOffset)
{
	if (FromIndexOffset == 0) return m_MessageBlock;

	bool MessageCutted = false;
	if (messageBlockFrom == MessageBlockFrom::MessageText)
		WND_MessageBlock::MoveStartIndex_InMessageText(FromIndexOffset, Font, MessageCutted);
	else if (messageBlockFrom == MessageBlockFrom::MessageLines)
		WND_MessageBlock::MoveStartIndex_InMessageLines(FromIndexOffset, Font);

	return m_MessageBlock;
}

void WND_MessageBlock::GenerateMessageBlockFrom(const std::wstring& Source, DXTK_FONT* Font, bool& MessageCutted, SetMessageBlockPosInMessageText messageBlockPivot)
{
	m_MessageLines.clear();
	m_MessageBlock.clear();
	m_StartIndex_InMessageLines = 0;
	m_StartIndex_InMessageText = 0;
	if (Source.empty() != true)
	{
		m_MessageLines.push_back(Source);
		WND_MessageBlock::GenerateMessageBlock(m_MessageLines[0], Font, m_MessageBlock, MessageCutted, messageBlockPivot);
	}
}

void WND_MessageBlock::GenerateMessageBlockFrom(
	const std::list<std::wstring>& Source,
	wchar_t LineTabChar,
	DXTK_FONT* Font,
	SourceMessageListPopOrder SourcePopOrder,
	SetMessageBlockPosInMessageLines messageBlockPivot)
{
	m_MessageLines.clear();
	m_MessageBlock.clear();
	m_StartIndex_InMessageLines = 0;
	m_StartIndex_InMessageText = 0;

	if (SourcePopOrder == SourceMessageListPopOrder::FRONT)
	{
		for (auto Message_iter = Source.begin(); Message_iter != Source.end(); ++Message_iter)
		{
			auto& Message = (*Message_iter);
			WND_MessageBlock::GenerateMessageLines(Message, LineTabChar, m_MessageLines, Font, m_Cutting_Width);
		}
	}
	else if (SourcePopOrder == SourceMessageListPopOrder::BACK)
	{
		for (auto Message_iter = Source.rbegin(); Message_iter != Source.rend(); ++Message_iter)
		{
			auto& Message = (*Message_iter);
			WND_MessageBlock::GenerateMessageLines(Message, LineTabChar, m_MessageLines, Font, m_Cutting_Width);
		}
	}

	WND_MessageBlock::GenerateMessageBlock(m_MessageLines, Font, m_MessageBlock, messageBlockPivot);
}

void WND_MessageBlock::AddMessageLine(const std::wstring& Source, DXTK_FONT* Font,
	wchar_t LineTabChar,
	SetMessageBlockPosInMessageLines messageBlockPivot)
{
	WND_MessageBlock::GenerateMessageLines(Source, LineTabChar, m_MessageLines, Font, m_Cutting_Width);

	m_MessageBlock.clear();
	WND_MessageBlock::GenerateMessageBlock(m_MessageLines, Font, m_MessageBlock, messageBlockPivot);
}

void WND_MessageBlock::AddMessageCharacter(const wchar_t& Source, DXTK_FONT* Font, int AddPosIndex, bool& MessageCutted)
{
	if (m_MessageLines.empty() == true)
	{
		std::wstring newMessage;
		newMessage.push_back(Source);
		m_StartIndex_InMessageText = 0;
		m_MessageLines.push_back(newMessage);
		m_MessageBlock = newMessage;
		return;
	}

	std::wstring& TotalMessage = m_MessageLines[0];
	int MaxCharCount = (int)TotalMessage.size();

	AddPosIndex = (AddPosIndex < 0) ? 0 : ((AddPosIndex > MaxCharCount) ? MaxCharCount : AddPosIndex);

	TotalMessage.insert(AddPosIndex, 1, Source);

	if (AddPosIndex == MaxCharCount)
	{
		m_MessageBlock.clear();
		WND_MessageBlock::GenerateMessageBlock(TotalMessage, Font, m_MessageBlock, MessageCutted, WND_MessageBlock::SetMessageBlockPosInMessageText::RIGHT);
	}
	else if (AddPosIndex < m_StartIndex_InMessageText)
	{
		m_MessageBlock.clear();
		WND_MessageBlock::MoveStartIndex_InMessageText(AddPosIndex - m_StartIndex_InMessageText, Font, MessageCutted);
	}
	else
	{
		std::wstring CuttingLine;
		DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };

		// SpriteFont가 문자열 끝의 여백글자(' ')에 대한 크기를 측정하지 못하기 때문에
		// 문자열 끝부분에 여백이 1개 이상일 경우
		// 다음과 같이 임시로 종료문자를 넣어준다.
		// 문자열 중간에 대한 여백글자(' ')는 측정가능하다.
		wchar_t SpacingEndCharacter = L'.';
		float SpacingEndCharacterWidth = Font->GetStringSize(L".").x;

		for (int i = m_StartIndex_InMessageText; i < MaxCharCount; ++i)
		{
			auto& ch = TotalMessage[i];
			CuttingLine.push_back(ch);

			if (ch == L' ')
				CuttingLine.push_back(SpacingEndCharacter);

			CuttingLineSize = Font->GetStringSize(CuttingLine);

			if (CuttingLineSize.x > m_Cutting_Width)
			{
				CuttingLine.pop_back();
				if (i + 1 == AddPosIndex || i == AddPosIndex)
				{
					if (ch == L' ')
					{
						if ((CuttingLineSize.x - SpacingEndCharacterWidth) > m_Cutting_Width)
						{
							WND_MessageBlock::MoveStartIndex_InMessageText(+1, Font, MessageCutted);
							MessageCutted = true;
						}
						else break;
					}
					MessageCutted = true;
					WND_MessageBlock::MoveStartIndex_InMessageText(+1, Font, MessageCutted);
				}
				break;
			}

			if (ch == L' ')
				CuttingLine.pop_back(); // erase SpacingEndCharacter
		}

		if (MessageCutted == true)
			m_MessageBlock = WND_MessageBlock::GetMessageBlock(Font, MessageBlockFrom::MessageText);
		else
			m_MessageBlock = CuttingLine;
	}
}

void WND_MessageBlock::EraseMessageCharacter(int ErasePosIndex, DXTK_FONT* Font, bool& MessageCutted)
{
	if (m_MessageLines.empty() == true) return;
	if (m_MessageBlock.empty() == true) return;

	std::wstring& TotalMessage = m_MessageLines[0];

	ErasePosIndex = (ErasePosIndex < 0) ? 0 : ((ErasePosIndex > (int)TotalMessage.size() - 1) ? (int)TotalMessage.size() - 1 : ErasePosIndex);

	TotalMessage.erase(ErasePosIndex, 1);

	int newFromIndex = m_StartIndex_InMessageText - 1;
	if (newFromIndex < 0) newFromIndex = 0;

	std::wstring CuttingLine;
	DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };

	// SpriteFont가 문자열 끝의 여백글자(' ')에 대한 크기를 측정하지 못하기 때문에
	// 문자열 끝부분에 여백이 1개 이상일 경우
	// 다음과 같이 임시로 종료문자를 넣어준다.
	// 문자열 중간에 대한 여백글자(' ')는 측정가능하다.
	wchar_t SpacingEndCharacter = L'.';

	// 텍스트가 하나 지워졌을 때엔
	// MessageBlock의 위치를 한칸 옆으로 이동해보고
	// 만약 MessageBlock의 크기가 지정된 크기를 초과할 경우
	// 원래의 위치를 취하게 한다.
	for (int i = newFromIndex; i < (int)TotalMessage.size(); ++i)
	{
		auto& ch = TotalMessage[i];
		CuttingLine.push_back(ch);

		if (ch == L' ')
			CuttingLine.push_back(SpacingEndCharacter);

		CuttingLineSize = Font->GetStringSize(CuttingLine);

		// 만약 MessageBlock의 크기가 지정된 크기를 초과할 경우
		if (CuttingLineSize.x > m_Cutting_Width)
		{
			// 기존 MessageBlock의 위치로.
			newFromIndex = m_StartIndex_InMessageText;
			if (ch == L' ')
				CuttingLine.pop_back(); // erase SpacingEndCharacter
			else // 앞에 추가된 텍스트를 하나 지워서 원래의 MessageBlock으로 만든다.
				CuttingLine.erase(0, 1);

			MessageCutted = true;
			break;
		}

		if (ch == L' ')
			CuttingLine.pop_back(); // erase SpacingEndCharacter
	}

	m_StartIndex_InMessageText = newFromIndex;

	m_MessageBlock = CuttingLine;
}

void WND_MessageBlock::MoveStartIndex_InMessageText(int IndexOffset, DXTK_FONT* Font, bool& MessageCutted)
{
	if (IndexOffset == 0) return;
	if (m_MessageLines.empty() == true) return;
	if (m_MessageBlock.empty() == true) return;
	auto& TotalMessage = m_MessageLines[0];

	if (m_StartIndex_InMessageText == 0 && IndexOffset < 0) return;
	if ((m_StartIndex_InMessageText + (int)(m_MessageBlock.size())) == (int)TotalMessage.size() && IndexOffset > 0) return;

	// SpriteFont가 문자열 끝의 여백글자(' ')에 대한 크기를 측정하지 못하기 때문에
	// 문자열 끝부분에 여백이 1개 이상일 경우
	// 다음과 같이 임시로 종료문자를 넣어준다.
	// 문자열 중간에 대한 여백글자(' ')는 측정가능하다.
	wchar_t SpacingEndCharacter = L'.';

	std::wstring CuttingLine;
	DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };

	int newFromIndex = 0;
	if (IndexOffset > 0)
	{
		newFromIndex = m_StartIndex_InMessageText + (int)(m_MessageBlock.size() - 1) + IndexOffset;
		newFromIndex = (newFromIndex > (int)TotalMessage.size() - 1) ? (int)TotalMessage.size() - 1 : newFromIndex;

		for (int i = newFromIndex; i > 0; --i)
		{
			auto& ch = TotalMessage[i];
			CuttingLine.push_back(ch);

			if (ch == L' ')
				CuttingLine.push_back(SpacingEndCharacter);

			CuttingLineSize = Font->GetStringSize(CuttingLine);

			if (CuttingLineSize.x > m_Cutting_Width)
			{
				CuttingLine.pop_back();
				MessageCutted = true;
				break;
			}

			if (ch == L' ')
				CuttingLine.pop_back(); // erase SpacingEndCharacter
		}

		std::reverse(CuttingLine.begin(), CuttingLine.end());
		int StartIndex_Offset = 0;
		if ((int)CuttingLine.size() <= (int)m_MessageBlock.size())
		{
			StartIndex_Offset += (int)m_MessageBlock.size() - (int)CuttingLine.size();
			StartIndex_Offset += 1;
		}
		m_StartIndex_InMessageText += StartIndex_Offset;
	}
	else
	{
		newFromIndex = m_StartIndex_InMessageText + IndexOffset;
		newFromIndex = (newFromIndex < 0) ? 0 : newFromIndex;

		int MaxCharCount = (int)TotalMessage.size();
		for (int i = newFromIndex; i < MaxCharCount; ++i)
		{
			auto& ch = TotalMessage[i];
			CuttingLine.push_back(ch);

			if (ch == L' ')
				CuttingLine.push_back(SpacingEndCharacter);

			CuttingLineSize = Font->GetStringSize(CuttingLine);

			if (CuttingLineSize.x > m_Cutting_Width)
			{
				CuttingLine.pop_back();
				MessageCutted = true;
				break;
			}

			if (ch == L' ')
				CuttingLine.pop_back(); // erase SpacingEndCharacter
		}

		m_StartIndex_InMessageText = newFromIndex;
	}

	m_MessageBlock = CuttingLine;
}

void WND_MessageBlock::MoveStartIndex_InMessageLines(int IndexOffset, DXTK_FONT* Font)
{
	if (IndexOffset == 0) return;
	if (m_MessageLines.empty() == true) return;
	if (m_MessageBlock.empty() == true) return;

	int newFromIndex = 0;

	newFromIndex = m_StartIndex_InMessageLines + IndexOffset;
	if (newFromIndex < 0) newFromIndex = 0;

	m_MessageBlock.clear();
	int MaxMessageLine = (int)m_MessageLines.size();

	if (newFromIndex > MaxMessageLine)
		newFromIndex = MaxMessageLine - 1;

	std::wstring newMessageBlock;
	DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };

	bool FullStackLines = false;
	while (newFromIndex >= 0)
	{
		m_MessageBlock.clear();
		for (int i = newFromIndex; i < (int)m_MessageLines.size(); ++i)
		{
			newMessageBlock = m_MessageBlock + m_MessageLines[i];
			CuttingLineSize = Font->GetStringSize(newMessageBlock);
			if (CuttingLineSize.y > m_Cutting_Height)
			{
				FullStackLines = true;
				break;
			}
			m_MessageBlock += m_MessageLines[i] + L'\n';
		}

		if (FullStackLines == false) newFromIndex--;
		else break;
	}

	m_StartIndex_InMessageLines = newFromIndex;

	if (0 <= m_StartIndex_InMessageLines
		&& m_StartIndex_InMessageLines < m_MessageLines.size())
		m_MessageBlock.pop_back(); // erase '\n'
}

void WND_MessageBlock::GenerateMessageLines(
	const std::wstring& Source,
	wchar_t LineTabChar,
	std::vector<std::wstring>& Dest,
	DXTK_FONT* Font, float CuttingLineWidth)
{
	std::wstring CuttingLine;
	std::vector <std::wstring>& CuttingLines = Dest;

	if (Font->GetStringSize(Source).x <= CuttingLineWidth)
	{
		CuttingLines.push_back(Source);
		return;
	}

	// SpriteFont가 문자열 끝의 여백글자(' ')에 대한 크기를 측정하지 못하기 때문에
	// 문자열 끝부분에 여백이 1개 이상일 경우
	// 다음과 같이 임시로 종료문자를 넣어준다.
	// 문자열 중간에 대한 여백글자(' ')는 측정가능하다.
	wchar_t SpacingEndCharacter = L'.';
	std::wstring SpacingEndCharacter_str; SpacingEndCharacter_str.push_back(SpacingEndCharacter);
	float SpacingEndCharacterWidth = Font->GetStringSize(SpacingEndCharacter_str).x;

	std::wstring LineSplitTabText = Source.substr(0, Source.find(LineTabChar) + 1);
	LineSplitTabText += L"  ";
	LineSplitTabText += SpacingEndCharacter;
	float LineSplitTabWidth = Font->GetStringSize(LineSplitTabText).x;
	LineSplitTabWidth -= SpacingEndCharacterWidth;
	LineSplitTabText.pop_back(); // erase SpacingEndCharacter

	DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };
	
	for (int i = 0; i < (int)Source.size(); ++i)
	{
		auto ch = Source[i];
		CuttingLine.push_back(ch);
		if (ch == L'\n')
		{
			CuttingLine.pop_back();
			CuttingLines.push_back(CuttingLine);
			CuttingLine.clear();
			continue;
		}
		while (ch == L' ')
		{
			if (i == (int)Source.size() - 1)
				break;

			++i;

			ch = Source[i];
			CuttingLine.push_back(ch);

			if (ch == L' ')
				CuttingLine.push_back(SpacingEndCharacter);
			else break;

			CuttingLineSize = Font->GetStringSize(CuttingLine);

			auto over_char = CuttingLine.back();
			CuttingLine.pop_back(); // erase SpacingEndCharacter
			if (CuttingLineSize.x > CuttingLineWidth)
			{
				CuttingLines.push_back(CuttingLine);
				CuttingLine.clear();

				do
				{
					CuttingLine.push_back(L' ');
					CuttingLine.push_back(over_char);
					CuttingLineSize = Font->GetStringSize(CuttingLine);
					CuttingLine.pop_back(); // erase over_char
				} while (CuttingLineSize.x < LineSplitTabWidth);
			}
		}
		CuttingLineSize = Font->GetStringSize(CuttingLine);
		if (CuttingLineSize.x > CuttingLineWidth)
		{
			auto over_char = CuttingLine.back();
			CuttingLine.pop_back();
			CuttingLines.push_back(CuttingLine);
			CuttingLine.clear();

			do
			{
				CuttingLine.push_back(L' ');
				CuttingLine.push_back(over_char);
				CuttingLineSize = Font->GetStringSize(CuttingLine);
				CuttingLine.pop_back(); // erase over_char
			} while (CuttingLineSize.x < LineSplitTabWidth);
			CuttingLine.push_back(over_char);
		}
	}

	if(CuttingLine.empty() != true)
		CuttingLines.push_back(CuttingLine);
}

void WND_MessageBlock::GenerateMessageBlock(
	const std::vector<std::wstring>& MessageLines,
	DXTK_FONT* Font,
	std::wstring& DestMessageBlock,
	SetMessageBlockPosInMessageLines messageBlockPivot)
{
	if (messageBlockPivot == SetMessageBlockPosInMessageLines::TOP)
	{
		int LineStack = 0;
		DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };
		std::wstring newMessageBlock;
		for (int i = 0; i < (int)m_MessageLines.size(); ++i)
		{
			newMessageBlock = DestMessageBlock + m_MessageLines[i];
			CuttingLineSize = Font->GetStringSize(newMessageBlock);
			if (CuttingLineSize.y > m_Cutting_Height)
			{
				m_StartIndex_InMessageLines = i - LineStack;
				break;
			}
			DestMessageBlock += m_MessageLines[i] + L'\n';
			LineStack++;
		}
	}
	else if (messageBlockPivot == SetMessageBlockPosInMessageLines::BOTTOM)
	{
		std::list<std::wstring> MessageList;
		std::wstring MessageBlock;
		std::wstring newMessageBlock;
		DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };
		for (int i = (int)m_MessageLines.size() - 1; i >= 0; --i)
		{
			newMessageBlock = MessageBlock + m_MessageLines[i];
			CuttingLineSize = Font->GetStringSize(newMessageBlock);
			if (CuttingLineSize.y > m_Cutting_Height)
			{
				m_StartIndex_InMessageLines = i + 1;
				break;
			}
			MessageList.push_back(m_MessageLines[i]);
			MessageBlock += m_MessageLines[i] + L'\n';
		}

		std::reverse(MessageList.begin(), MessageList.end());

		for (auto& MessageOneLine : MessageList)
			DestMessageBlock += MessageOneLine + L'\n';
	}

	if (0 <= m_StartIndex_InMessageLines
		&& m_StartIndex_InMessageLines < m_MessageLines.size())
		DestMessageBlock.pop_back(); // erase '\n'
}

void WND_MessageBlock::GenerateMessageBlock(
	const std::wstring& SourceMessage,
	DXTK_FONT* Font,
	std::wstring& DestMessageBlock,
	bool& MessageCutted,
	SetMessageBlockPosInMessageText messageBlockPivot)
{
	std::wstring CuttingLine;
	DirectX::XMFLOAT2 CuttingLineSize = { 0.0f, 0.0f };
	// SpriteFont가 문자열 끝의 여백글자(' ')에 대한 크기를 측정하지 못하기 때문에
	// 문자열 끝부분에 여백이 1개 이상일 경우
	// 다음과 같이 임시로 종료문자를 넣어준다.
	// 문자열 중간에 대한 여백글자(' ')는 측정가능하다.
	wchar_t SpacingEndCharacter = L'.';

	if (messageBlockPivot == SetMessageBlockPosInMessageText::LEFT)
	{
		for (auto ch_iter = SourceMessage.begin(); ch_iter != SourceMessage.end(); ++ch_iter)
		{
			auto ch = (*ch_iter);
			CuttingLine.push_back(ch);

			if (ch == L' ')
				CuttingLine.push_back(SpacingEndCharacter);

			CuttingLineSize = Font->GetStringSize(CuttingLine);
			if (CuttingLineSize.x > m_Cutting_Width)
			{
				CuttingLine.pop_back();
				MessageCutted = true;
				break;
			}

			if (ch == L' ')
				CuttingLine.pop_back(); // erase SpacingEndCharacter
		}
	}
	else if (messageBlockPivot == SetMessageBlockPosInMessageText::RIGHT)
	{
		for (auto ch_iter = SourceMessage.rbegin(); ch_iter != SourceMessage.rend(); ++ch_iter)
		{
			auto ch = (*ch_iter);
			CuttingLine.push_back(ch);

			if (ch == L' ')
				CuttingLine.push_back(SpacingEndCharacter);

			CuttingLineSize = Font->GetStringSize(CuttingLine);
			if (CuttingLineSize.x > m_Cutting_Width)
			{
				CuttingLine.pop_back();
				MessageCutted = true;
				break;
			}

			if (ch == L' ')
				CuttingLine.pop_back(); // erase SpacingEndCharacter
		}
		std::reverse(CuttingLine.begin(), CuttingLine.end());
		m_StartIndex_InMessageText = (int)SourceMessage.size() - (int)CuttingLine.size();
	}

	DestMessageBlock = CuttingLine;
}
