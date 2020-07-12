#pragma once
#include "Scene.h"

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

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    ///////////////////////////////////////////////////////////////////////////////// Processing Events /////////////////////////////////////////////////////////////////////////////////
    // Control Element ID, Player NickName, Character Type, Propensity, Transform(Scale, RotationEuler, Position)
    void SpawnPlayer(int New_CE_ID, std::wstring Name, CHARACTER_TYPE CharacterType, bool IsMainPlayer, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Control Element ID, Attack Order(Chracter Type), Propensity, Transform(Scale, RotationEuler, Position)
    void SpawnNormalAttackObject(int New_CE_ID, CHARACTER_TYPE AttackOrder, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Control Element ID, Skill Type, Propensity, Transform(Scale, RotationEuler, Position)
    void SpawnSkillObject(int New_CE_ID, SKILL_TYPE SkillType, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Effect Type, Transform(Position)
    void SpawnEffectObjects(EFFECT_TYPE EffectType, XMFLOAT3 Position);
    // Control Element ID, Transform(Scale, RotationEuler, Position)
    void SetObjectTransform(int CE_ID, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position);
    // Control Element ID, MotionType, SkillType(스킬 모션일 경우에만 지정, 그 외의 경우에는 NON)
    void SetCharacterMotion(int CE_ID, MOTION_TYPE MotionType, SKILL_TYPE SkillType = SKILL_TYPE::NON);
    // Control Element ID, Player State
    void SetPlayerState(int CE_ID, PLAYER_STATE PlayerState);
    // Deactivated Poison Gas Area
    //void UpdateDeActPoisonGasArea(RECT DeActPoisonGasArea);
    // Control Element ID
    void DeActivateObject(int CE_ID);

    // Count Score(Kill, Death, Assistance)
    void SetKDAScore(unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance);
    // Do_UserName, Target_UserName
    void SetKillLog(std::wstring Do_UserName, std::wstring Target_UserName);
    // UserName, Message
    void SetChatLog(std::wstring UserName, std::wstring Message);
    // Remaining Sec
    void SetGamePlayTimeLimit(unsigned int Sec);
    // Remaining HP
    void SetPlayerHP(int CE_ID, int HP);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    const UINT m_MaxWorldObject = MAX_WORLD_OBJECT;
    const UINT m_MaxCharacterObject = MAX_CHARACTER_OBJECT;
    const UINT m_MaxTextObject = 9 + MAX_CHARACTER_OBJECT;
    UINT       m_MaxUILayOutObject = MAX_CHARACTER_OBJECT * 3; // Number of HP Bar(테두리 + 증감HP Bar + HP Bar)
    std::uint64_t m_EffectInstancingNum = 0;

    Object* m_GroundObj = nullptr;
    Player* m_MainPlayer = nullptr;
    std::unordered_map<int, std::unique_ptr<Player>> m_Players;

    int GameInfo_CountKill = 0;
    int GameInfo_CountDeath = 0;
    int GameInfo_CountAssistance = 0;

    const size_t MaxKillLog = 10;
    const size_t MaxChatLog = 20;
    std::list<std::wstring> KillLogList;
    std::list<std::wstring> ChattingList;

    unsigned int TimeLimit_Sec = 0;

    bool ChattingMode = false;

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
