#pragma once
#include "Scene.h"
#include "WND_MessageBlock.h"
#include "ChattingListBox.h"
#include "InputTextBox.h"

class PlayGameScene : public Scene
{
public:
    PlayGameScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~PlayGameScene();

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
    void AnimateWorldObjectsTransform(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);
    void AnimateEffectObjectsTransform(CTimer& gt);
    void AnimateCharacterRenderEffect(CTimer& gt);
    void UpdateUITransformAs(CTimer& gt, Camera* MainCamera, std::unordered_map<int, std::unique_ptr<Player>>& Players);
    void RotateBillboardObjects(Camera* MainCamera, std::vector<Object*>& Objects);

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    ///////////////////////////////////////////////////////////////////////////////// Processing Events /////////////////////////////////////////////////////////////////////////////////
    // Control Element ID, Player NickName, Character Type, Propensity, Transform(Scale, RotationEuler, Position)
    virtual void SpawnPlayer(int New_CE_ID, std::wstring Name, CHARACTER_TYPE CharacterType, bool IsMainPlayer, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Control Element ID, Attack Order(Chracter Type), Propensity, Transform(Scale, RotationEuler, Position)
    virtual void SpawnNormalAttackObject(int New_CE_ID, CHARACTER_TYPE AttackOrder, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Control Element ID, Skill Type, Propensity, Transform(Scale, RotationEuler, Position)
    virtual void SpawnSkillObject(int New_CE_ID, SKILL_TYPE SkillType, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Effect Type, Transform(Position)
    virtual void SpawnEffectObjects(EFFECT_TYPE EffectType, XMFLOAT3 Position);
    // Control Element ID, Transform(Scale, RotationEuler, Position)
    virtual void SetObjectTransform(int CE_ID, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Control Element ID, MotionType, MotionSpeed, SkillType(스킬 모션일 경우에만 지정, 그 외의 경우에는 NON)
    virtual void SetCharacterMotion(int CE_ID, MOTION_TYPE MotionType, float MotionSpeed = 1.0f, SKILL_TYPE SkillType = SKILL_TYPE::NON);
    // Control Element ID, Player State
    virtual void SetPlayerState(int CE_ID, PLAYER_STATE PlayerState);
    // Deactivated Poison Gas Area
    virtual void UpdateDeActPoisonGasArea(RECT DeActPoisonGasArea);
    // Control Element ID
    virtual void DeActivateObject(int CE_ID);

    // Count Score(Kill, Death, Assistance)
    virtual void SetKDAScore(unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance);
    // Killer Object ID, Dead Object ID
    virtual void SetKillLog(short Kill_Player_id, short Death_Player_id);
    // Message ([Do_UserName]: Chat Message)
    virtual void SetChatLog(std::wstring Message);
    // Remaining Sec
    virtual void SetGamePlayTimeLimit(unsigned int Sec);
    // Remaining HP
    virtual void SetPlayerHP(int CE_ID, int HP);
    // In Game Team Score (Team's Total Kill)
    virtual void SetInGameTeamScore(unsigned char InGameScore_Team);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    const UINT m_MaxWorldObject = MAX_WORLD_OBJECT;
    const UINT m_MaxCharacterObject = MAX_CHARACTER_OBJECT;
    const UINT m_MaxTextObject = 30 + MAX_CHARACTER_OBJECT;
    UINT       m_MaxUILayoutObject = 30 + MAX_CHARACTER_OBJECT;
    std::uint64_t m_EffectInstancingNum = 0;

    std::vector<RenderItem*> m_CharacterRitems[(int)CHARACTER_TYPE::COUNT];

    Object* m_GroundObj = nullptr;
    Player* m_MainPlayer = nullptr;
    std::unordered_map<int, std::unique_ptr<Player>> m_Players;

    std::vector<Object*> m_BillboardObjects;
    std::vector<Object*> m_EffectObjects;

    int GameInfo_CountKill = 0;
    int GameInfo_CountDeath = 0;
    int GameInfo_CountAssistance = 0;
    int GameInfo_CountTeamScore = 0;
    const int MaxTeamScore = 5;

    int TimeLimit_Sec = 0;
    float TimeLimitIntervalTimeStack = 0.0f;

    RECT DeActPoisonGasArea = {-3423, 4290, 4577, -3710 };

private:
    const size_t MaxKillLog = 10;
    std::queue<std::wstring> KillLogList;
    bool KillLogSlidingStart = false;
    bool KillLogSlidingInit = false;

private:
    const float MaxChattingLineWidth = 180.0f;
    const float MaxChattingLineHeight = 210.0f;
    ChattingListBox ChattinglistBox;
    InputTextBox inputTextBox;

    bool ChattingLayerActivate = false;
    bool ChattingLayerSliding = false;
    const float ChattingLayerSlidingActionEndTime = 0.5f;

private:
    /// 런타임 중에 VK_LEFT, RIGHT, UP, DOWN 키와,
    // x,y,z키를 가지고 오브젝트의 LocalTransform을 
    // 지정해준다. 거시적으로 적당하게 오브젝트가 배치된 것
    // 같으면 중단점을 걸어 local_angle과 local_pos 값을 확인한다.
    void ControlSetObjectLocalScaleAndTranslation(Object* obj)
    {
        static XMFLOAT3 local_angle = { 0.0f, 0.0f, 0.0f };
        static XMFLOAT3 local_pos = { 0.0f, 0.0f, 0.0f };

        //x
        if (GetAsyncKeyState(0x58) && 0x8000)
        {
            if (GetAsyncKeyState(VK_UP) && 0x8000)
                local_angle.x += 1.0f;
            else if (GetAsyncKeyState(VK_DOWN) && 0x8000)
                local_angle.x -= 1.0f;

            if (GetAsyncKeyState(VK_LEFT) && 0x8000)
                local_pos.x -= 1.0f;
            else if (GetAsyncKeyState(VK_RIGHT) && 0x8000)
                local_pos.x += 1.0f;
        }
        //y
        if (GetAsyncKeyState(0x59) && 0x8000)
        {
            if (GetAsyncKeyState(VK_UP) && 0x8000)
                local_angle.y += 1.0f;
            else if (GetAsyncKeyState(VK_DOWN) && 0x8000)
                local_angle.y -= 1.0f;

            if (GetAsyncKeyState(VK_LEFT) && 0x8000)
                local_pos.y -= 1.0f;
            else if (GetAsyncKeyState(VK_RIGHT) && 0x8000)
                local_pos.y += 1.0f;
        }
        //z
        if (GetAsyncKeyState(0x5A) && 0x8000)
        {
            if (GetAsyncKeyState(VK_UP) && 0x8000)
                local_angle.z += 1.0f;
            else if (GetAsyncKeyState(VK_DOWN) && 0x8000)
                local_angle.z -= 1.0f;

            if (GetAsyncKeyState(VK_LEFT) && 0x8000)
                local_pos.z -= 1.0f;
            else if (GetAsyncKeyState(VK_RIGHT) && 0x8000)
                local_pos.z += 1.0f;
        }
        obj->m_TransformInfo->SetLocalRotationEuler(local_angle);
        obj->m_TransformInfo->SetLocalPosition(local_pos);
    }
};
