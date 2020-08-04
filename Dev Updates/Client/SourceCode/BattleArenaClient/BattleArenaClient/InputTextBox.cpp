#include "InputTextBox.h"

void InputTextBox::Init(
	float newMaxWidth, float newMaxHeight,
	DXTK_FONT* newFont,
	Object* newTextRenderObj, Object* newCaretRenderObj,
	const std::wstring& newStartInputForm,
	size_t newMaxInputText)
{
	m_MaxWidth = newMaxWidth;
	m_MaxHeight = newMaxHeight;
    m_RenderTextsBlock.init(m_MaxWidth, m_MaxHeight);

    m_StartInputForm = newStartInputForm;

	m_MaxInputText = newMaxInputText;

	m_RenderableTexts.clear();
	m_RenderableTexts.insert(m_RenderableTexts.size(), L"0123456789");
	m_RenderableTexts.insert(m_RenderableTexts.size(), L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	m_RenderableTexts.insert(m_RenderableTexts.size(), L"abcdefghijklmnopqrstuvwxyz");
	m_RenderableTexts.insert(m_RenderableTexts.size(), L" ");

	m_InputTexts.clear();
	m_InputTextsCaretPosIndex = m_RenderTextsCaretPosIndex = 0;

    m_IsHiding = false;
    m_HideCharacter = L'*';

	m_TextRenderObj = newTextRenderObj;
	m_CaretRenderObj = newCaretRenderObj;
	m_Font = newFont;
}

void InputTextBox::SetTextRenderObj(Object* obj)
{
	m_TextRenderObj = obj;
}

void InputTextBox::SetCaretRenderObj(Object* obj)
{
	m_CaretRenderObj = obj;
}

void InputTextBox::SetFont(DXTK_FONT* font)
{
	m_Font = font;
}

void InputTextBox::SetMaxInputText(size_t newMaxInputText)
{
	m_MaxInputText = newMaxInputText;
}

void InputTextBox::SetStartInputForm(const std::wstring& newStartInputForm)
{
	m_StartInputForm = newStartInputForm;
}

void InputTextBox::SetMaxSize(float newWidth, float newHeight)
{
	m_MaxWidth = newWidth;
	m_MaxHeight = newHeight;

    m_RenderTextsBlock.SetCuttingSize(m_MaxWidth, m_MaxHeight);
}

void InputTextBox::SetPosition(DirectX::XMFLOAT2 newPosition)
{
	if (m_TextRenderObj == nullptr) return;

	auto& TextRenderObjPos = m_TextRenderObj->m_Textinfo->m_TextPos;
	TextRenderObjPos = newPosition;
}

void InputTextBox::SetActivate(bool activate)
{
    Activate = activate;
}

void InputTextBox::SetHide(bool State, wchar_t hide_charcter)
{
    m_IsHiding = State;
    m_HideCharacter = hide_charcter;
}

DirectX::XMFLOAT2 InputTextBox::GetSize()
{
	if (m_Font == nullptr) return DirectX::XMFLOAT2(0.0f, 0.0f);
	if (m_TextRenderObj == nullptr) return DirectX::XMFLOAT2(0.0f, 0.0f);

	auto& ChattingLogRenderText = m_TextRenderObj->m_Textinfo->m_Text;
	return m_Font->GetStringSize(ChattingLogRenderText);
}

DirectX::XMFLOAT2 InputTextBox::GetPosition()
{
	if (m_TextRenderObj == nullptr) return DirectX::XMFLOAT2(0.0f, 0.0f);

	return m_TextRenderObj->m_Textinfo->m_TextPos;;
}

std::wstring InputTextBox::GetFullinputTexts()
{
    if (m_InputTexts.empty() == true) return std::wstring();
    return m_StartInputForm + m_InputTexts;
}

bool InputTextBox::IsEmpty()
{
    return m_InputTexts.empty();
}

void InputTextBox::AddText(const wchar_t& newText)
{
    std::wstring FullinputTexts = m_StartInputForm + m_InputTexts;
    if (FullinputTexts.size() == m_MaxInputText) return;

    auto RenderTextsInfo = m_TextRenderObj->m_Textinfo.get();
    auto& RenderTexts = RenderTextsInfo->m_Text;

    wchar_t AddText = newText;
    if (CapsLock == false && 0x41 <= (int)newText && (int)newText <= 0x5A)
        AddText = (wchar_t)((int)newText + 0x20);

    int InsertOffset = 0;
    if (m_StartInputForm.empty() == false) InsertOffset = 1;;

    int InsertIndex = m_InputTextsCaretPosIndex + InsertOffset;
    InsertIndex -= (int)m_StartInputForm.size();
    m_InputTexts.insert((size_t)InsertIndex, 1, AddText);

    if (m_IsHiding == true)
        AddText = m_HideCharacter;

    bool MessageCutted = false;
    m_RenderTextsBlock.AddMessageCharacter(AddText, m_Font, m_InputTextsCaretPosIndex + InsertOffset, MessageCutted);
    RenderTexts = m_RenderTextsBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageText);

    m_InputTextsCaretPosIndex++;
    m_RenderTextsCaretPosIndex
        = MathHelper::Clamp(m_RenderTextsCaretPosIndex + 1, -1, (int)RenderTexts.size() - 1);
}

std::wstring InputTextBox::PopText(int& InputTextsCaretPosIndex, bool PopCaretBack)
{
    auto RenderTextsInfo = m_TextRenderObj->m_Textinfo.get();
    auto& RenderTexts = RenderTextsInfo->m_Text;
    std::wstring FullinputTexts = m_StartInputForm + m_InputTexts;
    std::wstring retTexts;

    if (m_InputTexts.empty() == false)
    {
        int ErasePosIndex = InputTextsCaretPosIndex;

        if (PopCaretBack == true)
        {
            if (InputTextsCaretPosIndex > (int)m_StartInputForm.size())
            {
                int minPosIndex = MathHelper::Clamp((int)m_StartInputForm.size() - 1, 0, (int)m_StartInputForm.size() - 1);
                ErasePosIndex
                    = MathHelper::Clamp(InputTextsCaretPosIndex + 1, minPosIndex, (int)FullinputTexts.size() - 1);
            }
        }

        int ErasePrevStartIdex_inMessageText = m_RenderTextsBlock.GetStartIndex_InMessageText();
        int EraseAfterStartIndex_inMessageText = 0;
        if (ErasePosIndex >= m_StartInputForm.size())
        {
            if ((InputTextsCaretPosIndex != ((int)FullinputTexts.size() - 1)) || PopCaretBack != true)
            {
                bool MessageCutted = false;
                m_RenderTextsBlock.EraseMessageCharacter(ErasePosIndex, m_Font, MessageCutted);
                ErasePosIndex -= (int)m_StartInputForm.size();
                retTexts = m_InputTexts.erase(ErasePosIndex, 1);

                EraseAfterStartIndex_inMessageText = m_RenderTextsBlock.GetStartIndex_InMessageText();
            }
        }

        RenderTexts = m_RenderTextsBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageText);

        if (PopCaretBack == true)
        {
            if (ErasePosIndex == (int)FullinputTexts.size() - 1)
                InputTextsCaretPosIndex = (int)FullinputTexts.size() - 1;

            if (ErasePrevStartIdex_inMessageText != EraseAfterStartIndex_inMessageText)
            {
                m_RenderTextsCaretPosIndex
                    = MathHelper::Clamp(m_RenderTextsCaretPosIndex + 1, -1, (int)RenderTexts.size() - 1);
            }
        }
        else
        {
            FullinputTexts = m_StartInputForm + m_InputTexts;

            int minPosIndex = MathHelper::Clamp((int)m_StartInputForm.size() - 1, 0, (int)m_StartInputForm.size() - 1);

            InputTextsCaretPosIndex
                = MathHelper::Clamp(InputTextsCaretPosIndex - 1, minPosIndex, (int)FullinputTexts.size() - 1);

            if (ErasePrevStartIdex_inMessageText == EraseAfterStartIndex_inMessageText)
            {
                if (FullinputTexts.size() <= RenderTexts.size())
                {
                    m_RenderTextsCaretPosIndex
                        = MathHelper::Clamp(m_RenderTextsCaretPosIndex - 1, (int)m_StartInputForm.size() - 1, (int)RenderTexts.size() - 1);
                }
                else
                {
                    m_RenderTextsCaretPosIndex
                        = MathHelper::Clamp(m_RenderTextsCaretPosIndex - 1, -1, (int)RenderTexts.size() - 1);
                }
            }
        }
    }

    return retTexts;
}

void InputTextBox::InitTexts()
{
    if (m_TextRenderObj == nullptr) return;
    if (m_Font == nullptr) return;

    m_InputTexts.clear();

    auto& RenderTexts = m_TextRenderObj->m_Textinfo->m_Text;
    bool MessageCutted = false;
    m_RenderTextsBlock.GenerateMessageBlockFrom(m_StartInputForm, m_Font, MessageCutted, WND_MessageBlock::SetMessageBlockPosInMessageText::RIGHT);

    RenderTexts = m_RenderTextsBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageText);

    m_InputTextsCaretPosIndex = MathHelper::Clamp((int)m_StartInputForm.size() - 1, 0, (int)m_StartInputForm.size() - 1);
    m_RenderTextsCaretPosIndex = (int)m_StartInputForm.size() - 1;
}

void InputTextBox::MoveLeft_Caret()
{
    auto RenderTextsInfo = m_TextRenderObj->m_Textinfo.get();
    auto& RenderTexts = RenderTextsInfo->m_Text;
    std::wstring FullinputTexts = m_StartInputForm + m_InputTexts;

    if (m_RenderTextsCaretPosIndex == -1)
    {
        bool MessageCutted = false;
        m_RenderTextsBlock.MoveStartIndex_InMessageText(-1, m_Font, MessageCutted);
        RenderTexts = m_RenderTextsBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageText);
    }

    if (FullinputTexts.size() > RenderTexts.size())
    {
        if (m_InputTextsCaretPosIndex == (int)m_StartInputForm.size())
        {
            bool MessageCutted = false;
            m_RenderTextsBlock.MoveStartIndex_InMessageText(-(int)(m_StartInputForm.size()), m_Font, MessageCutted);
            RenderTexts = m_RenderTextsBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageText);

            m_RenderTextsCaretPosIndex = (int)m_StartInputForm.size() - 1;
        }
        else
        {
            m_RenderTextsCaretPosIndex
                = MathHelper::Clamp(m_RenderTextsCaretPosIndex - 1, -1, (int)RenderTexts.size() - 1);
        }
    }
    else
    {
        m_RenderTextsCaretPosIndex
            = MathHelper::Clamp(m_RenderTextsCaretPosIndex - 1, (int)m_StartInputForm.size() - 1, (int)RenderTexts.size() - 1);
    }

    m_InputTextsCaretPosIndex
        = MathHelper::Clamp(m_InputTextsCaretPosIndex - 1, (int)m_StartInputForm.size(), (int)FullinputTexts.size() - 1);
}

void InputTextBox::MoveRight_Caret()
{
    auto RenderTextsInfo = m_TextRenderObj->m_Textinfo.get();
    auto& RenderTexts = RenderTextsInfo->m_Text;
    std::wstring FullinputTexts = m_StartInputForm + m_InputTexts;

    if (m_RenderTextsCaretPosIndex == (int)RenderTexts.size() - 1)
    {
        bool MessageCutted = false;
        m_RenderTextsBlock.MoveStartIndex_InMessageText(+1, m_Font, MessageCutted);
        RenderTexts = m_RenderTextsBlock.GetMessageBlock(m_Font, WND_MessageBlock::MessageBlockFrom::MessageText);
    }

    m_InputTextsCaretPosIndex
        = MathHelper::Clamp(m_InputTextsCaretPosIndex + 1, 0, (int)FullinputTexts.size() - 1);
    m_RenderTextsCaretPosIndex
        = MathHelper::Clamp(m_RenderTextsCaretPosIndex + 1, -1, (int)RenderTexts.size() - 1);
}

void InputTextBox::Update(CTimer& gt, float ViewPortWidth, float ViewPortHeight)
{
    if (m_TextRenderObj == nullptr) return;
	if (m_CaretRenderObj == nullptr) return;
    if (m_Font == nullptr) return;

    auto RenderTextsInfo = m_TextRenderObj->m_Textinfo.get();
    auto& RenderTexts = RenderTextsInfo->m_Text;
    auto CaretTransform = m_CaretRenderObj->m_TransformInfo.get();

    // Update Caret
    {
        DirectX::XMFLOAT2 RenderTextsSize = { 0.0f, 0.0f };
        if (RenderTexts.empty() == false)
        {
            if (m_RenderTextsCaretPosIndex >= 0)
            {
                wchar_t SpacingWdithHelperCharacter = '.';
                int CaretPosAddOffset = 1;
                std::wstring RenderSubTexts = RenderTexts.substr(0, (size_t)m_RenderTextsCaretPosIndex + 1);
                if (RenderSubTexts.back() == L' ')
                {
                    RenderSubTexts.push_back(SpacingWdithHelperCharacter);
                    CaretPosAddOffset++;
                }

                RenderTextsSize = m_Font->GetStringSize(RenderSubTexts.substr(0, (size_t)m_RenderTextsCaretPosIndex + CaretPosAddOffset));
            }
        }

        DirectX::XMFLOAT2 RenderTextsPos = RenderTextsInfo->m_TextPos;
        DirectX::XMFLOAT3 newCaretPos = { 0.0f, 0.0f, 0.0f };

        RenderTextsPos.x -= ViewPortWidth / 2.0f;
        RenderTextsPos.y -= ViewPortHeight / 2.0f;
        RenderTextsPos.y *= -1.0f;
        newCaretPos.x = RenderTextsPos.x + RenderTextsSize.x;
        newCaretPos.y = RenderTextsPos.y;

        CaretTransform->SetWorldPosition(newCaretPos);
        CaretTransform->UpdateWorldTransform();

        if (Activate == true)
        {
            if (CaretBlinkTimeStack >= CaretBlinkInterval)
            {
                if (CaretTransform->m_TexAlpha == 1.0f)
                    CaretTransform->m_TexAlpha = 0.0f;
                else if (CaretTransform->m_TexAlpha == 0.0f)
                    CaretTransform->m_TexAlpha = 1.0f;
                CaretBlinkTimeStack = 0.0f;
            }
            else CaretBlinkTimeStack += gt.GetTimeElapsed();
        }
        else
        {
            CaretTransform->m_TexAlpha = 0.0f;
            CaretBlinkTimeStack = 0.0f;
        }
    }
}

void InputTextBox::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    // Insert Text
    {
        int key_code = 0;
        wchar_t key_down_text = '\0';
        bool RenderableText_AllKeyUp = true;

        for (auto& wchar : m_RenderableTexts)
        {
            key_code = (int)wchar;
            if (key_state[key_code] == true)
            {
                if (ContinouseInputIntervalAct == false)
                {
                    LastInputText = wchar;
                    InputTextBox::AddText(wchar);
                }

                key_down_text = wchar;
                if (key_down_text == LastInputText)
                    ContinouseInputIntervalAct = true;
                else
                    ContinouseInputIntervalAct = false;

                RenderableText_AllKeyUp = false;

                break;
            }
        }

        if (RenderableText_AllKeyUp == true)
            ContinouseInputIntervalAct = false;

        if (ContinouseInputIntervalAct == true)
        {
            if (ContinuousInputTimeStack >= ContinuousInputInterval)
            {
                if (CharacterInsertTimeStack >= CharacterInsertInterval)
                {
                    if (key_down_text != '\0')
                        InputTextBox::AddText(key_down_text);

                    CharacterInsertTimeStack = 0.0f;
                }
                else CharacterInsertTimeStack += gt.GetTimeElapsed();
            }
            else ContinuousInputTimeStack += gt.GetTimeElapsed();
        }
        else
        {
            CharacterInsertTimeStack = 0.0f;
            ContinuousInputTimeStack = 0.0f;
            ContinouseInputIntervalAct = false;
        }
    }

    // Activate CapsLock
    if (key_state[VK_CAPITAL] == true)
    {
        CapsLockAct = true;
    }
    else if (key_state[VK_CAPITAL] == false)
    {
        if (CapsLockAct == true)
        {
            CapsLock = !CapsLock;
            CapsLockAct = false;
        }
    }

    // Erase Text
    {
        if (key_state[VK_BACK] == true || key_state[VK_DELETE] == true)
        {
            if (ContinouseEraseIntervalAct == false)
            {
                InputTextBox::PopText(m_InputTextsCaretPosIndex, key_state[VK_DELETE]);

                ContinouseEraseIntervalAct = true;
            }

            if (ContinuousEraseTimeStack >= ContinuousEraseInterval)
            {
                if (CharacterEraseTimeStack >= CharacterEraseInterval)
                {
                    InputTextBox::PopText(m_InputTextsCaretPosIndex, key_state[VK_DELETE]);

                    CharacterEraseTimeStack = 0.0f;
                }
                else CharacterEraseTimeStack += gt.GetTimeElapsed();
            }
            else ContinuousEraseTimeStack += gt.GetTimeElapsed();
        }
        else
        {
            ContinuousEraseTimeStack = 0.0f;
            CharacterEraseTimeStack = 0.0f;
            ContinouseEraseIntervalAct = false;
        }
    }

    // Move Caret
    {
        if (key_state[VK_LEFT] == true || key_state[VK_RIGHT] == true)
        {
            if (InputBoxScrollingAct == false)
            {
                if (key_state[VK_LEFT] == true)
                    InputTextBox::MoveLeft_Caret();
                else if (key_state[VK_RIGHT] == true)
                    InputTextBox::MoveRight_Caret();

                InputBoxScrollingAct = true;
            }
            else
            {
                if (InputBoxScorllingTimeStack >= InputBoxScrollingInterval)
                {
                    if (key_state[VK_LEFT] == true)
                        InputTextBox::MoveLeft_Caret();
                    else if (key_state[VK_RIGHT] == true)
                        InputTextBox::MoveRight_Caret();

                    InputBoxScorllingTimeStack = 0.0f;
                }
                else InputBoxScorllingTimeStack += gt.GetTimeElapsed();
            }
        }
        else
        {
            InputBoxScorllingTimeStack = 0.0f;
            InputBoxScrollingAct = false;
        }
    }
}

