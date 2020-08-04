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

class InputTextBox
{
public:
    void Init(
        float newMaxWidth = 0.0f, float newMaxHeight = 0.0f,
        DXTK_FONT* newFont = nullptr,
        Object* newTextRenderObj = nullptr,
        Object* newCaretRenderObj = nullptr,
        const std::wstring& newStartInputForm = L"",
        size_t newMaxInputText = 100);

    void SetTextRenderObj(Object* obj);
    void SetCaretRenderObj(Object* obj);
    void SetFont(DXTK_FONT* font);
    void SetMaxInputText(size_t newMaxInputText);
    void SetStartInputForm(const std::wstring& newStartInputForm);

    void SetMaxSize(float newWidth, float newHeight);
    void SetPosition(DirectX::XMFLOAT2 newPosition);

    void SetActivate(bool activate);
    void SetHide(bool State, wchar_t hide_charcter = L'*');

    DirectX::XMFLOAT2 GetSize();
    DirectX::XMFLOAT2 GetPosition();
    std::wstring GetFullinputTexts();

    bool IsEmpty();

    void AddText(const wchar_t& newText);
    std::wstring PopText(int& InputTextsCaretPosIndex, bool PopCaretBack = false);
    void InitTexts();

    void MoveLeft_Caret();
    void MoveRight_Caret();

    void Update(CTimer& gt, float ViewPortWidth, float ViewPortHeight);
    void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

private:
	float m_MaxWidth = 0.0f;
	float m_MaxHeight = 0.0f;
	WND_MessageBlock m_RenderTextsBlock;

private:
	std::wstring m_StartInputForm;
    size_t m_MaxInputText = 128;
    std::wstring m_RenderableTexts;

private:
    std::wstring m_InputTexts;
    int m_InputTextsCaretPosIndex = 0;
    int m_RenderTextsCaretPosIndex = 0;

private:
    bool m_IsHiding = false;
    wchar_t m_HideCharacter = L'*';

private:
    Object* m_TextRenderObj = nullptr;
    Object* m_CaretRenderObj = nullptr;
    DXTK_FONT* m_Font = nullptr;

private:
    wchar_t LastInputText = L'\0';
    bool CapsLock = false;
    bool Activate = true;
    bool CaretUpdate = false;

private:
    const float CharacterInsertInterval = 0.01f;
    const float ContinuousInputInterval = 0.3f;
    const float CharacterEraseInterval = 0.01f;
    const float ContinuousEraseInterval = 0.3f;
    const float CaretBlinkInterval = 0.5f;
    const float InputBoxScrollingInterval = 0.06f;
    float CharacterInsertTimeStack = 0.0f;
    float ContinuousInputTimeStack = 0.0f;
    float CharacterEraseTimeStack = 0.0f;
    float ContinuousEraseTimeStack = 0.0f;
    float CaretBlinkTimeStack = 0.0f;
    float InputBoxScorllingTimeStack = 0.0f;

    bool ContinouseInputIntervalAct = false;
    bool ContinouseEraseIntervalAct = false;
    bool InputBoxScrolling = false;
    bool InputBoxScrollingAct = false;
    bool CapsLockAct = false;
};