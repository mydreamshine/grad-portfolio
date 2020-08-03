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

	void init(float CuttingWidth, float CuttingHeight)
	{
		m_MessageBlock.clear();
		m_MessageLines.clear();
		m_StartIndex_InMessageLines = 0;
		m_StartIndex_InMessageText = 0;
		m_Cutting_Width = CuttingWidth;
		m_Cutting_Height = CuttingHeight;
	}

	void SetCuttingSize(float width, float height)
	{
		m_Cutting_Width = width;
		m_Cutting_Height = height;
	}

	int GetStartIndex_InMessageLines() { return m_StartIndex_InMessageLines; }
	int GetStartIndex_InMessageText() { return m_StartIndex_InMessageText; }

	const std::wstring& GetMessageBlock(DXTK_FONT* Font, MessageBlockFrom messageBlockFrom, int FromIndexOffset = 0);

	void GenerateMessageBlockFrom(const std::wstring& Source, DXTK_FONT* Font, bool& MessageCutted,
		SetMessageBlockPosInMessageText messageBlockPivot = SetMessageBlockPosInMessageText::LEFT);

	void GenerateMessageBlockFrom(
		const std::list<std::wstring>& Source,
		wchar_t LineTabChar,
		DXTK_FONT* Font,
		SourceMessageListPopOrder SourcePopOrder = SourceMessageListPopOrder::FRONT,
		SetMessageBlockPosInMessageLines messageBlockPivot = SetMessageBlockPosInMessageLines::TOP);

	void AddMessageLine(const std::wstring& Source, DXTK_FONT* Font,
		wchar_t LineTabChar,
		SetMessageBlockPosInMessageLines messageBlockPivot = SetMessageBlockPosInMessageLines::TOP);
	void AddMessageCharacter(const wchar_t& Source, DXTK_FONT* Font, int AddPosIndex, bool& MessageCutted);

	void EraseMessageCharacter(int ErasePosIndex, DXTK_FONT* Font, bool& MessageCutted);

	void MoveStartIndex_InMessageText(int IndexOffset, DXTK_FONT* Font, bool& MessageCutted);
	void MoveStartIndex_InMessageLines(int IndexOffset, DXTK_FONT* Font);

private:
	void GenerateMessageLines(
		const std::wstring& Source,
		wchar_t LineTabChar,
		std::vector<std::wstring>& Dest,
		DXTK_FONT* Font,
		float CuttingLineWidth);

	void GenerateMessageBlock(
		const std::vector<std::wstring>& MessageLines,
		DXTK_FONT* Font,
		std::wstring& DestMessageBlock,
		SetMessageBlockPosInMessageLines messageBlockPivot = SetMessageBlockPosInMessageLines::TOP);

	void GenerateMessageBlock(
		const std::wstring& SourceMessage,
		DXTK_FONT* Font,
		std::wstring& DestMessageBlock,
		bool& MessageCutted,
		SetMessageBlockPosInMessageText messageBlockPivot = SetMessageBlockPosInMessageText::LEFT);

private:
	std::wstring m_MessageBlock;
	std::vector<std::wstring> m_MessageLines;

	float m_Cutting_Width = 0.0f;
	float m_Cutting_Height = 0.0f;

	int m_StartIndex_InMessageLines = 0;
	int m_StartIndex_InMessageText = 0;
};