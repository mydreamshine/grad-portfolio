#pragma once
#include "Scene.h"

class GameOverScene : public Scene
{
public:
    GameOverScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~GameOverScene();

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

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    ///////////////////////////////////////////////////////////////////////////////// Processing Events /////////////////////////////////////////////////////////////////////////////////
    // UserName, UserRank, Count Score(Kill, Death, Assistance, Total Damage, Total Heal), Played Character Type
    virtual void SetMatchStatisticInfo(std::wstring UserName, int UserRank,
        unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance,
        int TotalScore_Damage, int TotalScore_Heal,
        CHARACTER_TYPE PlayedCharacterType);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    const UINT m_MaxWorldObject = 4;
    const UINT m_MaxCharacterObject = 4;
    const UINT m_MaxTextObject = 3;

    std::vector<RenderItem*> m_CharacterRitems[(int)CHARACTER_TYPE::COUNT];

    std::wstring UserInfo_UserName;
    int UserInfo_UserRank = 0;

    int GameMatchingResult_CountKill = 0;
    int GameMatchingResult_CountDeath = 0;
    int GameMatchingResult_CountAssistance = 0;
    int GameMatchingResult_TotalDamage = 0;
    int GameMatchingResult_TotalHeal = 0;
    CHARACTER_TYPE GameMatchingResult_PlayedCharacterType = CHARACTER_TYPE::NON;

    bool ReturnLobyButtonPress = false;
    bool ReturnLobyButtonUp = true;

    bool OnceTryReturnLoby = false;
};