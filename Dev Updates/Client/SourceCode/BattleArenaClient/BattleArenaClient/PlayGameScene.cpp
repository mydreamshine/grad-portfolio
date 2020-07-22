#include "stdafx.h"
#include "PlayGameScene.h"

PlayGameScene::~PlayGameScene()
{
}

void PlayGameScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    Scene::OnInit(device, commandList,
        objCB_index, skinnedCB_index, textBatch_index);
}

void PlayGameScene::OnInitProperties(CTimer& gt)
{
    for (auto& Player_iter : m_Players)
    {
        auto Player = Player_iter.second.get();
        Player->Init();
    }
    m_Players.clear();
    m_MainPlayer = nullptr;

    ObjectManager objManager;

    for (auto& obj : m_WorldObjects)
    {
        if (obj->m_Name.find("Instancing") != std::string::npos)
        {
            for (auto& attachedObj : obj->m_Childs)
                objManager.DeActivateObj(attachedObj);
            objManager.DeActivateObj(obj);
        }
    }

    for (auto& obj : m_UILayOutObjects)
    {
        if (obj->m_Name.find("Instancing") != std::string::npos)
        {
            ObjectManager objManager;
            for (auto& attachedObj : obj->m_Childs)
                objManager.DeActivateObj(attachedObj);
            objManager.DeActivateObj(obj);
        }
    }

    for (auto& obj : m_TextObjects)
    {
        if (obj->m_Name.find("Instancing") != std::string::npos)
        {
            ObjectManager objManager;
            for (auto& attachedObj : obj->m_Childs)
                objManager.DeActivateObj(attachedObj);
            objManager.DeActivateObj(obj);
        }
    }

    m_LightRotationAngle = 0.0f;

    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    static XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);

    GameInfo_CountKill = 0;
    GameInfo_CountDeath = 0;
    GameInfo_CountAssistance = 0;

    KillLogList.clear();
    ChattingList.clear();
    inputChat.clear();

    TimeLimit_Sec = 0;

    ChattingMode = false;

    DeActPoisonGasArea = { -3423, 4290, 4577, -3710 };
}

void PlayGameScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt,
    std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    PlayGameScene::AnimateWorldObjectsTransform(gt, GeneratedEvents);
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt, GeneratedEvents);
}

void PlayGameScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;
    UINT MaxUILayOutObject = (UINT)Geometries["PlayGameSceneUIGeo"]->DrawArgs.size();

    ObjectManager objManager;

    for (auto& Ritem_iter : AllRitems)
    {
        auto& Ritem_name = Ritem_iter.first;
        auto Ritem = Ritem_iter.second.get();

        if (Ritem_name.find("Male Knight 01") != std::string::npos)
        {
            m_CharacterRitems[(int)CHARACTER_TYPE::WARRIOR].push_back(Ritem);
        }
        else if (Ritem_name.find("Male Warrior 01") != std::string::npos)
        {
            m_CharacterRitems[(int)CHARACTER_TYPE::BERSERKER].push_back(Ritem);
        }
        else if (Ritem_name.find("Male Mage 01") != std::string::npos)
        {
            m_CharacterRitems[(int)CHARACTER_TYPE::PRIEST].push_back(Ritem);
        }
        else if (Ritem_name.find("Female Warrior 01") != std::string::npos)
        {
            m_CharacterRitems[(int)CHARACTER_TYPE::ASSASSIN].push_back(Ritem);
        }
        else if (Ritem_name.find("UI") != std::string::npos)
        {
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, MaxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            // UI 오브젝트 같은 경우에는 메쉬의 원점이 (0,0,0)이 아니라
            // 이미 버텍스 자체가 스크린좌표계 기준으로 지정되어 있기 때문에
            // UI 오브젝트의 Transform에 대해선 따로 지정하지 않아도 된다.
            std::string objName = Ritem_iter.first;
            std::vector<RenderItem*> Ritems = { Ritem };
            objManager.SetObjectComponent(newObj, objName, Ritems);

            if (objName == "UI_Layout_SkillList") continue;
            auto newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
            auto text_info = newLayoutTextObj->m_Textinfo.get();
            newLayoutTextObj->m_Name = "Text" + objName;
            text_info->m_FontName = L"맑은 고딕";
            text_info->m_TextColor = DirectX::Colors::Blue;
            auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
            text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;

            if (objName == "UI_Layout_GameTimeLimit") text_info->m_Text = L"Time Limit";
            else if (objName == "UI_Layout_KDA")      text_info->m_Text = L"K: 0  D: 0  A: 0";
            else if (objName == "UI_Layout_KillLog")  text_info->m_Text = L"Kill Log";
            else if (objName == "UI_Layout_ChattingLog")  text_info->m_Text = L"Chatting Log";
            else if (objName == "UI_Layout_Skill1")  text_info->m_Text = L"Skill1";
            else if (objName == "UI_Layout_Skill2")  text_info->m_Text = L"Skill2";
            else if (objName == "UI_Layout_Skill3")  text_info->m_Text = L"Skill3";
            else if (objName == "UI_Layout_Skill4")  text_info->m_Text = L"Skill4";
        }
        else if (Ritem_name.find("Effect") != std::string::npos)
        {
            continue;
        }
        else
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);
            std::string objName = Ritem_iter.first;

            if (objName.find("SpawnStageGround") != std::string::npos)
            {
                XMFLOAT3 WorldPosition = { 17.86f, 5.0f, 0.0f };
                std::vector<RenderItem*> Ritems = { Ritem };
                objManager.SetObjectComponent(newObj, objName, Ritems, nullptr,
                    nullptr, nullptr, nullptr,
                    nullptr, nullptr, &WorldPosition);
            }
            else // Environment Object
            {
                // 환경 사물 같은 경우에는 메쉬의 원점이 (0,0,0)이 아니라
                // 이미 파일 자체에서 메쉬가 배치되어 있는 상태라서
                // 환경 사물의 Transform에 대해선 따로 지정하지 않아도 된다.
                std::vector<RenderItem*> Ritems = { Ritem };
                objManager.SetObjectComponent(newObj, objName, Ritems);

                if(objName.find("Floor1") != std::string::npos)
                {
                    m_GroundObj = newObj;
                    auto& FloorBound = newObj->m_TransformInfo->m_Bound;
                    float width = FloorBound.Extents.x;
                    float depth = FloorBound.Extents.z;

                    m_SceneBounds.Center = FloorBound.Center;
                    // m_SceneBounds.Radius에 비례해서 ShadowMap이 맵핑되다보니
                    // ShadowMap의 해상도보다 m_SceneBounds.Radius가 크면
                    // BackBuffer에 맵핑되는 그림자의 해상도가 상대적으로 떨어지게 된다.
                    m_SceneBounds.Radius = (float)SIZE_ShadowMap*2;
                }
            }
        }
    }

    // Create DeActive Objects
    {
        // SkinnedCB를 지닌(Skeleton이 있는) 오브젝트를 생성하면
        // SkinnedCB뿐만 아니라 ObjCB도(WorldObject도) 잡히기 때문에
        // WorldObject보다 적은 SkinnedObject를 먼저 생성해준다.
        const UINT nDeAcativateCharacterObj = m_MaxCharacterObject - (UINT)m_CharacterObjects.size();
        for (UINT i = 0; i < nDeAcativateCharacterObj; ++i)
        {
            auto newObj = objManager.CreateCharacterObject(
                objCB_index++, skinnedCB_index++,
                m_AllObjects,
                m_CharacterObjects, m_MaxCharacterObject,
                m_WorldObjects, m_MaxWorldObject);
            // CreateCharacterObject내부적으로 FindDeactiveCharacterObject()를 통해
            // 새로운 오브젝트를 만들게 되는데
            // 이때 FindDeactiveCharacterObject()는
            // 현재 오브젝트 리스트 중에서 Active가 false인 오브젝트를 먼저 찾아서 반환하기 때문에
            // 이처럼 한꺼번에 많은 오브젝트를 만들 경우에는
            // 각 오브젝트의 Activate 여부를 오브젝트들을 다 만들고 나서 결정해줘야 한다.
            //newObj->Activated = false;
            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
        }

        const UINT nDeAcativateWorldObj = m_MaxWorldObject - (UINT)m_WorldObjects.size();
        for (UINT i = 0; i < nDeAcativateWorldObj; ++i)
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);
        }

        const UINT nDeActivateTextObj = m_MaxTextObject - (UINT)m_TextObjects.size();
        for (UINT i = 0; i < nDeActivateTextObj; ++i)
        {
            auto newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
        }

        for (UINT i = m_MaxCharacterObject - nDeAcativateCharacterObj; i < m_MaxCharacterObject; ++i)
            m_CharacterObjects[i]->Activated = false;
        for (UINT i = m_MaxWorldObject - nDeAcativateWorldObj; i < m_MaxWorldObject; ++i)
            m_WorldObjects[i]->Activated = false;
        for (UINT i = m_MaxTextObject - nDeActivateTextObj; i < m_MaxTextObject; ++i)
            m_TextObjects[i]->Activated = false;
    }

    m_nObjCB = (UINT)m_WorldObjects.size() + (UINT)m_UILayOutObjects.size();
    m_nSKinnedCB = (UINT)m_CharacterObjects.size();
    m_nTextBatch = (UINT)m_TextObjects.size();
}

void PlayGameScene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
    Scene::UpdateObjectCBs(objCB, gt);
}

void PlayGameScene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    Scene::UpdateSkinnedCBs(skinnedCB, gt);
}

void PlayGameScene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
    Scene::UpdateMaterialCBs(matCB, gt);
}

void PlayGameScene::UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt)
{
    Scene::UpdateMainPassCB(passCB, gt);
}

void PlayGameScene::UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::UpdateShadowPassCB(passCB, shadow_map, gt);
}

void PlayGameScene::UpdateShadowTransform(CTimer& gt)
{
    Scene::UpdateShadowTransform(gt);
}

void PlayGameScene::UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    ObjectManager ObjManager;
    Object* KillLogObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_KillLog");
    Object* ChattingLogObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_ChattingLog");
    Object* TimeLimitObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_GameTimeLimit");
    Object* KDA_ScoreObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_KDA");

    // Update KillLog Text
    {
        auto& KillLog = KillLogObject->m_Textinfo->m_Text;
        KillLog.clear();

        int EmptyKillLogLine = 6;
        for (auto& KillLogLine : KillLogList)
        {
            KillLog += KillLogLine;
            if (--EmptyKillLogLine == 0) break;
            else KillLog += L'\n';
        }
    }

    // Update Chatting Text
    {
        auto& ChattingLog = ChattingLogObject->m_Textinfo->m_Text;
        ChattingLog.clear();

        if (ChattingMode == true)
        {
            int EmptyChatLine = 10;
            for (auto& ChatLine : ChattingList)
            {
                ChattingLog += ChatLine;
                if (--EmptyChatLine == 0) break;
                else ChattingLog += L'\n';
            }
        }
    }

    // Update TimeLimit Text
    {
        auto& TimeLimitText = TimeLimitObject->m_Textinfo->m_Text;
        int minute = TimeLimit_Sec / 60;
        int sec = TimeLimit_Sec % 60;
        TimeLimitText = L"Time Limit\n  ";

        if (minute < 10) TimeLimitText += L"0";
        TimeLimitText += std::to_wstring(minute) + L":";
        if (sec < 10) TimeLimitText += L"0";
        TimeLimitText += std::to_wstring(sec);
    }

    // Update KDA Score Text
    {
        auto& KDA_ScoreText = KDA_ScoreObject->m_Textinfo->m_Text;
        KDA_ScoreText  = L"K: " + std::to_wstring(GameInfo_CountKill)  + L"  ";
        KDA_ScoreText += L"D: " + std::to_wstring(GameInfo_CountDeath) + L"  ";
        KDA_ScoreText += L"A: " + std::to_wstring(GameInfo_CountAssistance);
    }
}

void PlayGameScene::AnimateLights(CTimer& gt)
{
    float deg2rad = MathHelper::Pi / 180.0f;
    m_LightRotationAngle += (1.0f * deg2rad) * gt.GetTimeElapsed();

    XMMATRIX R = XMMatrixRotationY(m_LightRotationAngle);
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR lightDir = XMLoadFloat3(&m_BaseLightDirections[i]);
        lightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&m_RotatedLightDirections[i], lightDir);
    }
}

void PlayGameScene::AnimateSkeletons(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (m_MainPlayer != nullptr)
        m_MainPlayer->ProcessSkeletonAnimNotify(GeneratedEvents);

    for (auto& obj : m_CharacterObjects)
    {
        auto SkeletonInfo = obj->m_SkeletonInfo.get();
        auto AnimInfo = SkeletonInfo->m_AnimInfo.get();
        std::string& AnimName = AnimInfo->CurrPlayingAnimName;
        bool isSetted = false;
        if (AnimInfo->AnimIsPlaying(AnimName, isSetted) == true)
        {
            if (isSetted == false) continue;
            SkeletonInfo->m_Skeleton->UpdateAnimationTransforms(*(AnimInfo), gt.GetTimeElapsed());

            if (obj->m_Childs.empty() != true)
            {
                for (auto& child_obj : obj->m_Childs)
                {
                    auto parentObjInfo = obj->m_TransformInfo.get();
                    auto childObjInfo = child_obj->m_TransformInfo.get();
                    // child 오브젝트의 AttachingTargetBoneID를 기준으로 AnimatedTransform을 가져와
                    // child 오브젝트의 월드 트랜스폼을 대체한다.
                    int AttachedBoneID = childObjInfo->m_AttachingTargetBoneID;
                    if (AttachedBoneID != -1)
                    {
                        XMFLOAT4X4 AnimTransform = MathHelper::Identity4x4();
                        Scene::aiM2dxM(AnimTransform, AnimInfo->CurrAnimJointTransforms[AttachedBoneID]);

                        // UpdateSkinnedCBs()에서 skinConstants.BoneTransform에 대입되는
                        // AnimTransform은 전치되지 않은 본래의 Transform이다.
                        // 하지만 UpdateObjectCBs()에서 현재 AnimTransform이 적용된
                        // WorldTransform이 전치되어버리고 만다.
                        // 이를 미연의 방지하기 위해(AnimTransform의 전치를 상쇄하기 위해)
                        // Attaching을 통한 오브젝트의 WorldTransform을 지정할 때에는
                        // AnimTransform을 미리 전치시킨다.
                        XMStoreFloat4x4(&AnimTransform, XMMatrixTranspose(XMLoadFloat4x4(&AnimTransform)));
                        XMMATRIX AnimM = XMLoadFloat4x4(&AnimTransform);

                        // 오브젝트 메쉬 트랜스폼을 위한 행렬 곱 순서
                        // AnimTransform * LocalTransform * WorldTransform
                        // Attaching을 한 오브젝트는
                        // 해당 오브젝트가 Skeleton을 지니고 있는 것이 아니기 때문에
                        // VS쉐이더 단계에서 animTransform을 계산하는 과정이 생략된다
                        // 고로, Attaching 오브젝트의 애니메이션을 위해선
                        // Attaching 오브젝트의 LocalTransform * WorldTransform이
                        // AnimTransform과 LocalTransform * WorldTransform을 포함한 행렬이어야 한다.
                        // 이때 LocalTransform과 WorldTransform은 해당 오브젝트의 World/Local이 아닌
                        // Attaching을 당한 오브젝트(장비 오브젝트 기준으론 캐릭터 오브젝트)
                        // 의 Local/WorldTransform을 취해준다.
                        XMFLOAT4X4 WorldTransform = parentObjInfo->GetWorldTransform();
                        XMFLOAT4X4 LocalTransform = parentObjInfo->GetLocalTransform();
                        XMMATRIX WorldM = XMLoadFloat4x4(&WorldTransform);
                        XMMATRIX LocalM = XMLoadFloat4x4(&LocalTransform);
                        XMMATRIX AnimWorldM = (AnimM * LocalM) * WorldM;

                        XMStoreFloat4x4(&WorldTransform, AnimWorldM);
                        childObjInfo->SetWorldTransform(WorldTransform);
                    }
                }
            }
        }
    }
}

void PlayGameScene::AnimateCameras(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    static XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    float cam_move_factor = 3.0f * gt.GetTimeElapsed();

    if (m_MainPlayer != nullptr)
    {
        m_MainPlayer->UpdateCamera(gt, (float)m_width / m_height);

        XMVECTOR newEyePos = m_MainCamera.GetPosition();
        XMVECTOR newLookAtPos = m_MainCamera.GetLook();
        newEyePos += (m_MainPlayer->m_Camera.GetPosition() - m_MainCamera.GetPosition()) * cam_move_factor;
        newLookAtPos += (m_MainPlayer->m_Camera.GetLook() - m_MainCamera.GetLook()) * cam_move_factor;
        newLookAtPos += newEyePos;
        /*XMVECTOR newEyePos = m_MainPlayer->m_Camera.GetPosition();
        XMVECTOR newLookAtPos = m_MainPlayer->m_Camera.GetLook();*/

        XMStoreFloat3(&EyePosition, newEyePos);
        XMStoreFloat3(&LookAtPosition, newLookAtPos);
        UpDirection = m_MainPlayer->m_Camera.GetUp3f();
    }
    else
    {
        float deg2rad = MathHelper::Pi / 180.0f;
        static float camAngle = -90.0f * deg2rad;
        //camAngle += 5.0f * deg2rad;
        float phi = 30.0f * deg2rad;
        float rad = 3000.0f;
        XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
        Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
        XMStoreFloat3(&EyePosition, Eye_Pos);
    }

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 10000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}

void PlayGameScene::AnimateWorldObjectsTransform(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    for (auto& obj : m_WorldObjects)
    {
        if (obj->ProcessSelfDeActivate(gt) != true)
            obj->m_TransformInfo->Animate(gt);
    }
}

void PlayGameScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (m_MainPlayer != nullptr)
    {
        CD3DX12_VIEWPORT ViewPort((float)m_ClientRect.left, (float)m_ClientRect.top, (float)m_ClientRect.right, (float)m_ClientRect.bottom);
        m_MainPlayer->ProcessInput(key_state, oldCursorPos, ViewPort, gt, m_GroundObj, GeneratedEvents);
    }

    // Process Chat
    {

    }

    if (OnceSendChatLog == true)
    {
        EventManager eventManager;
        eventManager.ReservateEvent_SendChatLog(GeneratedEvents, FEP_LOBY_SCENE, inputChat);
        inputChat.clear();
        OnceSendChatLog = false;
    }
}

void PlayGameScene::SpawnPlayer(
    int New_CE_ID,
    std::wstring Name,
    CHARACTER_TYPE CharacterType,
    bool IsMainPlayer,
    OBJECT_PROPENSITY Propensity,
    XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    if (CharacterType == CHARACTER_TYPE::NON) return;

    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;
    auto& CharacterBoudingBoxes = *m_CharacterModelBoundingBoxesRef;

    ObjectManager objManager;
    Object* newCharacterObj = nullptr;
    newCharacterObj = objManager.FindDeactiveCharacterObject(
        m_AllObjects,
        m_CharacterObjects, m_MaxCharacterObject,
        m_WorldObjects, m_MaxWorldObject);

    if (newCharacterObj == nullptr)
    {
        MessageBox(NULL, L"No characters available in the character list.", L"Object Generate Warning", MB_OK);
        return;
    }

    // Set Character Prop
    {
        newCharacterObj->m_CE_ID = New_CE_ID;
        newCharacterObj->CharacterType = CharacterType;
        newCharacterObj->Propensity = Propensity;
        switch (CharacterType)
        {
        case CHARACTER_TYPE::WARRIOR:
        {
            // 스케일을 250으로 늘려줘야 거시적으로
            // 주변 사물과의 크기 비례가 어색하지 않게 된다.
            float ConvertModelUnit = 250.0f;
            XMFLOAT3 WorldScale = { Scale.x * ConvertModelUnit, Scale.y * ConvertModelUnit, Scale.z * ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = RotationEuler;
            XMFLOAT3 WorldPosition = Position;
            XMFLOAT3 LocalRotationEuler = { 0.0f, 180.0f, 0.0f };
            std::string objName = "Warrior - Instancing(ID:" + std::to_string(New_CE_ID) + ")";
            objManager.SetObjectComponent(newCharacterObj, objName,
                m_CharacterRitems[(int)CHARACTER_TYPE::WARRIOR],
                ModelSkeletons["Male Knight 01"].get(),
                nullptr, &LocalRotationEuler, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);
            
            auto skeletonInfo = newCharacterObj->m_SkeletonInfo.get();
            auto animInfo = skeletonInfo->m_AnimInfo.get();
            animInfo->SetAnimTimeLineNotify("Sword Slash Gen", AnimNotifyTime::Warrior_SwordSlashStart);
        }
            break;
        case CHARACTER_TYPE::BERSERKER:
            break;
        case CHARACTER_TYPE::ASSASSIN:
            break;
        case CHARACTER_TYPE::PRIEST:
            break;
        }

        newCharacterObj->m_TransformInfo->UpdateLocalTransform();
        newCharacterObj->m_TransformInfo->UpdateWorldTransform();

        auto skeletonInfo = newCharacterObj->m_SkeletonInfo.get();
        auto animInfo = skeletonInfo->m_AnimInfo.get();
        animInfo->Init();
        std::set<std::string> AnimNameList; skeletonInfo->m_Skeleton->GetAnimationList_Name(AnimNameList);
        animInfo->AutoApplyActionFromSkeleton(AnimNameList);
        animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
        animInfo->AnimLoop(aiModelData::AnimActionType::Idle);
    }

    // Create Player
    {
        auto newPlayer = std::make_unique<Player>();
        newPlayer->m_Name = Name;
        newPlayer->m_CharacterObjRef = newCharacterObj;
        newPlayer->m_NickNameObjRef = objManager.FindDeactiveTextObject(m_AllObjects, m_TextObjects, m_MaxTextObject);
        auto& Textinfo = newPlayer->m_NickNameObjRef->m_Textinfo;
        Textinfo->m_FontName = L"맑은 고딕";
        Textinfo->m_TextColor = DirectX::Colors::Blue;
        Textinfo->m_Text = Name;
        newPlayer->m_HP_BarObjRef[0] = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
        newPlayer->m_HP_BarObjRef[1] = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
        newPlayer->m_HP_BarObjRef[2] = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
        /*objManager.SetObjectComponent(newPlayer->m_HP_BarObjRef[0], "HP Bar Border - Instancing", AllRitems["HP Bar Border"].get());
        objManager.SetObjectComponent(newPlayer->m_HP_BarObjRef[1], "HP Bar Increase - Instancing", AllRitems["HP Bar Increase"].get());
        objManager.SetObjectComponent(newPlayer->m_HP_BarObjRef[2], "HP Bar Dest - Instancing", AllRitems["HP Bar Dest"].get());*/
        if (IsMainPlayer == true) m_MainPlayer = newPlayer.get();
        m_Players[newCharacterObj->m_CE_ID] = std::move(newPlayer);
    }
}

void PlayGameScene::SpawnNormalAttackObject(int New_CE_ID,
    CHARACTER_TYPE AttackOrder, OBJECT_PROPENSITY Propensity,
    XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    if (AttackOrder == CHARACTER_TYPE::NON) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;
    Object* newNormalAttackObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);

    if (newNormalAttackObj == nullptr)
    {
        MessageBox(NULL, L"No skill objects available in the world object list.", L"Object Generate Warning", MB_OK);
        return;
    }

    // Set Object Prop
    {
        newNormalAttackObj->m_CE_ID = New_CE_ID;
        newNormalAttackObj->Propensity = Propensity;
        XMFLOAT3 WorldScale = Scale;
        XMFLOAT3 WorldRotationEuler = RotationEuler;
        XMFLOAT3 WorldPosition = Position;

        std::string objName;
        std::vector<RenderItem*> Ritems;
        
        switch (AttackOrder)
        {
        case CHARACTER_TYPE::WARRIOR:
        {
            objName = objName = "SwordNormalAttack - Instancing" + std::to_string(New_CE_ID);
            Ritems.push_back(AllRitems["SkillEffect_SwordSlash_a"].get());
        }
            break;
        case CHARACTER_TYPE::BERSERKER:
        {
            objName = objName = "AxeNormalAttack - Instancing" + std::to_string(New_CE_ID);
            //Ritem = ;
        }
            break;
        case CHARACTER_TYPE::ASSASSIN:
        {
            objName = objName = "DaggerNormalAttack - Instancing" + std::to_string(New_CE_ID);
            //Ritem = ;
        }
            break;
        case CHARACTER_TYPE::PRIEST:
        {
            objName = objName = "MagicWandNormalAttack - Instancing" + std::to_string(New_CE_ID);
            //Ritem = ;
        }
            break;
        }

        objManager.SetObjectComponent(newNormalAttackObj, objName,
            Ritems, nullptr,
            nullptr, nullptr, nullptr,
            &WorldScale, &WorldRotationEuler, &WorldPosition);
        newNormalAttackObj->m_TransformInfo->UpdateWorldTransform();
        newNormalAttackObj->m_TransformInfo->m_nonShadowRender = true;
    }
}

void PlayGameScene::SpawnSkillObject(int New_CE_ID,
    SKILL_TYPE SkillType, OBJECT_PROPENSITY Propensity,
    XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    if (SkillType == SKILL_TYPE::NON) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;
    Object* newSkillObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);

    if (newSkillObj == nullptr)
    {
        MessageBox(NULL, L"No skill objects available in the world object list.", L"Object Generate Warning", MB_OK);
        return;
    }

    // Set Object Prop
    {
        newSkillObj->m_CE_ID = New_CE_ID;
        newSkillObj->Propensity = Propensity;
        XMFLOAT3 WorldScale = Scale;
        XMFLOAT3 WorldRotationEuler = RotationEuler;
        XMFLOAT3 WorldPosition = Position;

        std::string objName;
        std::vector<RenderItem*> Ritems;

        switch (SkillType)
        {
        case SKILL_TYPE::SWORD_WAVE:
        {
            std::string objName = "SwordSlash - Instancing" + std::to_string(New_CE_ID);
            Ritems.push_back(AllRitems["SkillEffect_SwordSlash_a"].get());
        }
        break;
        case SKILL_TYPE::HOLY_AREA:
        {
            std::string objName = "HolyArea - Instancing" + std::to_string(New_CE_ID);

            for (int i = 0; i < 10; ++i)
            {
                // create holy effect object
            }
        }
            break;
        case SKILL_TYPE::FURY_ROAR:
        {
            std::string objName = "FuryRoar - Instancing" + std::to_string(New_CE_ID);

        }
            break;
        case SKILL_TYPE::STEALTH:
        {
            std::string objName = "StealthFog - Instancing" + std::to_string(New_CE_ID);
            for (int i = 0; i < 4; ++i)
            {
                // create stealth effect object
            }
        }
            break;
        }

        objManager.SetObjectComponent(newSkillObj, objName,
            Ritems, nullptr,
            nullptr, nullptr, nullptr,
            &WorldScale, &WorldRotationEuler, &WorldPosition);
        newSkillObj->m_TransformInfo->UpdateWorldTransform();
        newSkillObj->m_TransformInfo->m_nonShadowRender = true;
    }
}

void PlayGameScene::SpawnEffectObjects(EFFECT_TYPE EffectType, XMFLOAT3 Position)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    if (EffectType == EFFECT_TYPE::NON) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;

    switch (EffectType)
    {
    case EFFECT_TYPE::HOLY_EFFECT:
        break;
    case EFFECT_TYPE::FURY_ROAR_EFFECT:
        break;
    case EFFECT_TYPE::STEALTH_EFFECT:
        break;
    case EFFECT_TYPE::PICKING_EFFECT:
    {
        auto newEffectObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
        if (newEffectObj == nullptr)
        {
            MessageBox(NULL, L"No equipment object available in the world object list.", L"Object Generate Warning", MB_OK);
            return;
        }

        std::string objName = "CrossTarget - Instancing" + std::to_string(++m_EffectInstancingNum);
        std::vector<RenderItem*> Ritems = { AllRitems["PickingEffect_CrossTarget"].get() };
        objManager.SetObjectComponent(newEffectObj, objName,
            Ritems, nullptr,
            nullptr, nullptr, nullptr,
            nullptr, nullptr, &Position);
        newEffectObj->m_TransformInfo->m_nonShadowRender = true;
        newEffectObj->DeActivatedTime = 1.0f;
        newEffectObj->DeActivatedDecrease = 1.0f;
        newEffectObj->SelfDeActivated = true;
        newEffectObj->DisappearForDeAcTime = true;
    }
        break;
    }
}

void PlayGameScene::SetObjectTransform(int CE_ID, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position)
{
    ObjectManager ObjManager;
    Object* ControledObj = ObjManager.FindObjectCE_ID(m_WorldObjects, CE_ID);
    if(ControledObj != nullptr)
        ControledObj->m_TransformInfo->SetWorldTransform(Scale, RotationEuler, Position);
}

void PlayGameScene::SetCharacterMotion(int CE_ID, MOTION_TYPE MotionType, SKILL_TYPE SkillType)
{
    if (m_Players.find(CE_ID) != m_Players.end())
    {
        m_Players[CE_ID]->PlayMotion(MotionType, SkillType);
    }
    else
    {
        ObjectManager ObjManager;
        Object* ControledObj = ObjManager.FindObjectCE_ID(m_CharacterObjects, CE_ID);
        if (ControledObj == nullptr) return;

        auto AnimInfo = ControledObj->m_SkeletonInfo->m_AnimInfo.get();
        aiModelData::AnimActionType CurrPlayingAction = AnimInfo->CurrPlayingAction;
        aiModelData::AnimActionType newActionType = (aiModelData::AnimActionType)MotionType;

        if (newActionType == aiModelData::AnimActionType::Non) return;

        AnimInfo->AnimStop(CurrPlayingAction);
        AnimInfo->AnimPlay(newActionType);
        if (newActionType == aiModelData::AnimActionType::Idle || newActionType == aiModelData::AnimActionType::Walk)
            AnimInfo->AnimLoop(newActionType);
    }
}

void PlayGameScene::SetPlayerState(int CE_ID, PLAYER_STATE PlayerState)
{
    auto Player_iter = m_Players.find(CE_ID);
    if (Player_iter == m_Players.end()) return;
    Player* ControledPlayer = Player_iter->second.get();

    ControledPlayer->SetState(PlayerState);
}

void PlayGameScene::UpdateDeActPoisonGasArea(RECT deActPoisonGasArea)
{
    DeActPoisonGasArea = deActPoisonGasArea;
}

void PlayGameScene::DeActivateObject(int CE_ID)
{
    if (m_Players.find(CE_ID) != m_Players.end())
    {
        m_Players[CE_ID]->Init();
        m_Players.erase(CE_ID);
    }
    else
    {
        ObjectManager ObjManager;
        Object* ControledObj = ObjManager.FindObjectCE_ID(m_WorldObjects, CE_ID);
        if (ControledObj != nullptr)
            ObjManager.DeActivateObj(ControledObj);
    }
}

void PlayGameScene::SetKDAScore(unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance)
{
    GameInfo_CountKill = Count_Kill;
    GameInfo_CountDeath = Count_Death;
    GameInfo_CountAssistance = Count_Assistance;
}

void PlayGameScene::SetKillLog(std::wstring Message)
{
    if (KillLogList.size() == MaxKillLog) KillLogList.pop_back();
    KillLogList.push_back(Message);
}

void PlayGameScene::SetChatLog(std::wstring Message)
{
    if (ChattingList.size() == MaxChatLog) ChattingList.pop_back();
    ChattingList.push_back(Message);
}

void PlayGameScene::SetGamePlayTimeLimit(unsigned int Sec)
{
    TimeLimit_Sec = Sec;
}

void PlayGameScene::SetPlayerHP(int CE_ID, int HP)
{
    auto Player_iter = m_Players.find(CE_ID);
    if (Player_iter == m_Players.end()) return;
    Player* ControledPlayer = Player_iter->second.get();

    ControledPlayer->SetHP(HP);
}
