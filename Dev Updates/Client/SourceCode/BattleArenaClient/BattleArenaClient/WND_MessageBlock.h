#pragma once
#include "Common/FileLoader/SpriteFontLoader.h"
#include <string>
#include <vector>
#include <list>
#include <algorithm>

// WND 상에서 지정된 크기의 Block으로
// Message를 Cutting해주는 클래스
class WND_MessageBlock
{
public:
	WND_MessageBlock() = default;

	enum class SourceMessageListPopOrder { FRONT, BACK };
	enum class SetMessageBlockPosInMessageLines { TOP, BOTTOM, NON };
	enum class SetMessageBlockPosInMessageText { LEFT, RIGHT, NON };
	enum class MessageBlockFrom { MessageLines, MessageText };

	void SetCuttingSize(float width, float height)
	{
		m_Cutting_Width = width;
		m_Cutting_Height = height;
	}

	int GetStartIndex_InMessageLines() { return m_StartIndex_InMessageLines; }
	int GetStartIndex_InMessageText() { return m_StartIndex_InMessageText; }

	const std::wstring& GetMessageBlock(DXTK_FONT* Font, MessageBlockFrom messageBlockFrom, int FromIndexOffset)
	{
		int newFromIndex = m_StartIndex_InMessageText + FromIndexOffset;

		if (newFromIndex < 0) newFromIndex = 0;

		if (messageBlockFrom == MessageBlockFrom::MessageText)
		{
			int MaxCharCount = m_MessageLines[0].size();
			if (newFromIndex > MaxCharCount)
				newFromIndex = MaxCharCount - 1;

			bool FullCuttedText = false;
			std::wstring& Source = m_MessageLines[0];

			std::wstring CuttingLine;
			for (int i = newFromIndex; i < MaxCharCount; ++i)
			{
				auto& ch = Source[i];
				CuttingLine.push_back(ch);

				DirectX::XMFLOAT2 CuttingLineSize = Font->GetStringSize(CuttingLine);
				if (CuttingLineSize.x >= m_Cutting_Width)
				{
					FullCuttedText = true;
					m_StartIndex_InMessageText = newFromIndex;
					break;
				}
			}

			if (FullCuttedText == false)
			{
				CuttingLine.clear();
				for (auto ch_iter = Source.rbegin(); ch_iter != Source.rend(); ++ch_iter)
				{
					auto& ch = (*ch_iter);
					CuttingLine.push_back(ch);

					DirectX::XMFLOAT2 CuttingLineSize = Font->GetStringSize(CuttingLine);
					if (CuttingLineSize.x >= m_Cutting_Width) break;
				}
				std::reverse(CuttingLine.begin(), CuttingLine.end());
				m_StartIndex_InMessageText = (int)Source.size() - (int)CuttingLine.size();
			}

			m_MessageBlock = CuttingLine;
		}
		else if (messageBlockFrom == MessageBlockFrom::MessageLines)
		{
			m_MessageBlock.clear();
			int MaxMessageLine = m_MessageLines.size();

			if (newFromIndex > MaxMessageLine)
				newFromIndex = MaxMessageLine - 1;

			bool FullCuttedLines = false;

			for (int i = newFromIndex; i < (int)m_MessageLines.size(); ++i)
			{
				std::wstring MessageOneLine = m_MessageBlock + m_MessageLines[i];
				if (Font->GetStringSize(MessageOneLine).y > m_Cutting_Height)
				{
					m_StartIndex_InMessageLines = newFromIndex;
					FullCuttedLines = true;
					break;
				}
				m_MessageBlock += MessageOneLine + L'\n';
			}

			if (FullCuttedLines == false)
			{
				m_MessageBlock.clear();

				std::list<std::wstring> MessageList;
				std::wstring MessageBlock;
				for (int i = (int)m_MessageLines.size() - 1; i > 0; --i)
				{
					std::wstring MessageOneLine = MessageBlock + m_MessageLines[i];
					if (Font->GetStringSize(MessageOneLine).y > m_Cutting_Height)
					{
						m_StartIndex_InMessageLines = i + 1;
						break;
					}
					MessageList.push_back(m_MessageLines[i]);
					MessageBlock += MessageOneLine + L'\n';
				}

				std::reverse(MessageList.begin(), MessageList.end());

				for (auto& MessageOneLine : MessageList)
					m_MessageBlock += MessageOneLine + L'\n';
			}

			if (0 < m_StartIndex_InMessageLines
				&& m_StartIndex_InMessageLines < m_MessageLines.size())
				m_MessageBlock.pop_back(); // erase '\n'
		}
	}

	void GenerateMessageBlockFrom(const std::wstring& Source, DXTK_FONT* Font,
		SetMessageBlockPosInMessageText messageBlockPivot = SetMessageBlockPosInMessageText::LEFT)
	{
		m_MessageLines.clear();
		m_MessageBlock.clear();
		m_StartIndex_InMessageLines = 0;
		m_StartIndex_InMessageText = 0;

		std::wstring CuttingLine;
		if (messageBlockPivot == SetMessageBlockPosInMessageText::LEFT)
		{
			for (auto ch_iter = Source.begin(); ch_iter != Source.end(); ++ch_iter)
			{
				auto& ch = (*ch_iter);
				CuttingLine.push_back(ch);

				DirectX::XMFLOAT2 CuttingLineSize = Font->GetStringSize(CuttingLine);
				if (CuttingLineSize.x >= m_Cutting_Width) break;
			}
		}
		else if (messageBlockPivot == SetMessageBlockPosInMessageText::RIGHT)
		{
			for (auto ch_iter = Source.rbegin(); ch_iter != Source.rend(); ++ch_iter)
			{
				auto& ch = (*ch_iter);
				CuttingLine.push_back(ch);

				DirectX::XMFLOAT2 CuttingLineSize = Font->GetStringSize(CuttingLine);
				if (CuttingLineSize.x >= m_Cutting_Width) break;
			}
			std::reverse(CuttingLine.begin(), CuttingLine.end());
			m_StartIndex_InMessageText = (int)Source.size() - (int)CuttingLine.size();
		}

		m_MessageBlock = CuttingLine;
		if (Source.empty() != true) m_MessageLines.push_back(Source);
	}

	void GenerateMessageBlockFrom(const std::list<std::wstring>& Source, DXTK_FONT* Font,
		SourceMessageListPopOrder SourcePopOrder = SourceMessageListPopOrder::FRONT,
		SetMessageBlockPosInMessageLines messageBlockPivot = SetMessageBlockPosInMessageLines::TOP)
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
				WND_MessageBlock::GenerateMessageLines(Message, m_MessageLines, Font, m_Cutting_Width);
			}
		}
		else if (SourcePopOrder == SourceMessageListPopOrder::BACK)
		{
			for (auto Message_iter = Source.rbegin(); Message_iter != Source.rend(); ++Message_iter)
			{
				auto& Message = (*Message_iter);
				WND_MessageBlock::GenerateMessageLines(Message, m_MessageLines, Font, m_Cutting_Width);
			}
		}

		if (messageBlockPivot == SetMessageBlockPosInMessageLines::TOP)
		{
			int LineStack = 0;
			for (int i = 0; i < (int)m_MessageLines.size(); ++i)
			{
				std::wstring MessageOneLine = m_MessageBlock + m_MessageLines[i];
				if (Font->GetStringSize(MessageOneLine).y > m_Cutting_Height)
				{
					m_StartIndex_InMessageLines = i - LineStack;
					break;
				}
				m_MessageBlock += MessageOneLine + L'\n';
				LineStack++;
			}
		}
		else if (messageBlockPivot == SetMessageBlockPosInMessageLines::BOTTOM)
		{
			std::list<std::wstring> MessageList;
			std::wstring MessageBlock;
			for (int i = (int)m_MessageLines.size() - 1; i > 0; --i)
			{
				std::wstring MessageOneLine = MessageBlock + m_MessageLines[i];
				if (Font->GetStringSize(MessageOneLine).y > m_Cutting_Height)
				{
					m_StartIndex_InMessageLines = i + 1;
					break;
				}
				MessageList.push_back(m_MessageLines[i]);
				MessageBlock += MessageOneLine + L'\n';
			}

			std::reverse(MessageList.begin(), MessageList.end());

			for (auto& MessageOneLine : MessageList)
				m_MessageBlock += MessageOneLine + L'\n';
		}

		if (0 < m_StartIndex_InMessageLines
			&& m_StartIndex_InMessageLines < m_MessageLines.size())
			m_MessageBlock.pop_back(); // erase '\n'
	}

	void AddMessageLine(const std::wstring& Source, DXTK_FONT* Font)
	{
		WND_MessageBlock::GenerateMessageLines(Source, m_MessageLines, Font, m_Cutting_Width);
	}

private:
	void GenerateMessageLines(
		const std::wstring& Source, std::vector<std::wstring>& Dest,
		DXTK_FONT* Font, 
		float CuttingLineWidth)
	{
		std::wstring CuttingLine;
		std::vector <std::wstring>& CuttingLines = Dest;

		if (Font->GetStringSize(Source).x <= CuttingLineWidth)
		{
			CuttingLines.push_back(Source);
			return;
		}

		for (auto& ch : Source)
		{
			CuttingLine.push_back(ch);
			if (ch == L'\n')
			{
				CuttingLine.pop_back();
				CuttingLines.push_back(CuttingLine);
				CuttingLine.clear();
				continue;
			}
			DirectX::XMFLOAT2 CuttingLineSize = Font->GetStringSize(CuttingLine);
			if (CuttingLineSize.x >= CuttingLineWidth)
			{
				CuttingLines.push_back(CuttingLine);
				CuttingLine.clear();
			}
		}
	}

private:
	std::wstring m_MessageBlock;
	std::vector<std::wstring> m_MessageLines; // 가장 오래된 메세지가 맨 앞 원소

	float m_Cutting_Width = 0.0f;
	float m_Cutting_Height = 0.0f;

	int m_StartIndex_InMessageLines = 0;
	int m_StartIndex_InMessageText = 0;
};