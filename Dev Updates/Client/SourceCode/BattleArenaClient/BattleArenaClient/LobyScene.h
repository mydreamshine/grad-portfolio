#pragma once
#include "Scene.h"

class LobyScene : public Scene
{
public:
    LobyScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~LobyScene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    virtual void OnInitProperties(CTimer& gt);
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
        const bool key_state[], const POINT& oldCursorPos,
        const RECT& ClientRect,
        CTimer& gt,
        std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index);

public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);
    virtual void AnimateLights(CTimer& gt);
    virtual void AnimateSkeletons(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);
    virtual void AnimateCameras(CTimer& gt);
    void AnimateWorldObjectsTransform(CTimer& gt);

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    ///////////////////////////////////////////////////////////////////////////////// Processing Events /////////////////////////////////////////////////////////////////////////////////
    // UserName, UserRank
    virtual void SetUserInfo(std::wstring UserName, int UserRank);
    // UserName, Message
    virtual void SetChatLog(std::wstring UserName, std::wstring Message);
    // Set match status.
    virtual void SetMatchStatus(bool status);
    // Set match status.
    virtual void SetAccessMatch(bool status);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    const UINT m_MaxWorldObject = 4;
    const UINT m_MaxCharacterObject = 4;
    const UINT m_MaxTextObject = 7;

    std::vector<RenderItem*> m_CharacterRitems[(int)CHARACTER_TYPE::COUNT];

    float camAngle = -90.0f * (MathHelper::Pi / 180.0f);

    const size_t MaxChatLog = 20;
    std::list<std::wstring> ChattingList;
    std::wstring inputChat;

    std::wstring UserInfo_UserName;
    int UserInfo_UserRank = 0;

    CHARACTER_TYPE SelectedCharacterType = CHARACTER_TYPE::WARRIOR;
    float CharacterTurnTableYaw = 0.0f;
    float AmountTurnTableYaw = 0.0f;
    float savedTurnTableYaw = 0.0f;
    bool OnceSavedTurnTableYaw = false;

    bool ChattingMode = false;
    const float ChattingModeCoolTime = 0.3f;
    float ChattingModeTimeStack = 0.0f;
    bool ActivateChattingModeCoolTime = false;

    bool StartMatching = false;
    
    bool CharacterSelectionDone = true;
    bool CharacterSelectLeft = false;
    bool CharacterSelectRight = false;
    bool LeftButtonPress = false;
    bool RightButtonPress = false;
    bool LeftButtonUp = true;
    bool RightButtonUp = true;
    bool PlayButtonPress = false;
    bool PlayButtonUp = true;
    bool CancelButtonPress = false;
    bool CancelButtonUp = true;
    const float MatchingTextInfoInterval = 0.5f;
    float MatchingTextInfoTimeStack = 0.0f;
    int   MatchingTextInfoChangeStack = 0;
    bool CurrButtonIsPlayButton = true;
    bool ChangeButton = false;

    bool OnceGetUserInfo = true;
    bool OnceTryGameMatching = false;
    bool OnceSendChatLog = false;
    bool OnceAccessMatch = false;

    //TestFunc.
    bool OnceAccessBattleDirectly = false;
};