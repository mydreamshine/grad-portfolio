#include "stdafx.h"
#include "LobyScene.h"

LobyScene::~LobyScene()
{
}

void LobyScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    Scene::OnInit(device, commandList,
        objCB_index, skinnedCB_index, textBatch_index);
}

void LobyScene::OnInitProperties(CTimer& gt)
{
    for (auto& obj : m_CharacterObjects)
    {
        if (obj->Activated != true) continue;

        if (obj->m_Name.find("Warrior") != std::string::npos)
        {
            auto transformInfo = obj->m_TransformInfo.get();
            float ConvertModelUnit = 125.0f;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
            transformInfo->SetWorldTransform(WorldScale, WorldRotationEuler, WorldPosition);
            transformInfo->SetLocalRotationEuler(LocalRotationEuler);
        }

        auto skeletonInfo = obj->m_SkeletonInfo.get();
        auto animInfo = skeletonInfo->m_AnimInfo.get();
        animInfo->Init();
        std::set<std::string> AnimNameList; skeletonInfo->m_Skeleton->GetAnimationList_Name(AnimNameList);
        animInfo->AutoApplyActionFromSkeleton(AnimNameList);
        animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
        animInfo->AnimLoop(aiModelData::AnimActionType::Idle);
    }

    m_LightRotationAngle = 0.0f;

    camAngle = -90.0f * (MathHelper::Pi / 180.0f);

    ChattingList.clear();
    inputChat.clear();
    UserInfo_UserName.clear();
    UserInfo_UserRank = 0;

    SelectedCharacterType = CHARACTER_TYPE::NON;

    ChattingMode = false;
    StartMatching = false;
    MouseLeftButtonClick = false;

    OnceGetUserInfo = true;
    OnceTryGameMatching = false;
    OnceSendChatLog = false;
}

void LobyScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt,
    std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt, GeneratedEvents);
}

void LobyScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;

    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;
    const UINT maxUILayOutObject = (UINT)Geometries["LobySceneUIGeo"]->DrawArgs.size();

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
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, maxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            std::vector<RenderItem*> Ritems = { Ritem };
            objManager.SetObjectComponent(newObj, objName, Ritems);

            if (objName.find("CharacterSelection") != std::string::npos) continue;
            if (objName.find("Background") != std::string::npos) continue;

            auto newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
            auto text_info = newLayoutTextObj->m_Textinfo.get();
            newLayoutTextObj->m_Name = "Text" + objName;
            text_info->m_FontName = L"¸¼Àº °íµñ";
            text_info->m_TextColor = DirectX::Colors::Blue;
            auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
            text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;

            if (objName == "UI_Layout_LobyChattingLog") text_info->m_Text = L"Chatting Log";
            else if (objName == "UI_Layout_MatchWaiting")      text_info->m_Text = L"Match Waiting\n      Info";
            else if (objName == "UI_Layout_GameStartButton")  text_info->m_Text = L"Game Start\n   Button";
            else if (objName == "UI_Layout_LobyCharacterName")  text_info->m_Text = L"Character Name";
            else if (objName == "UI_Layout_LobyUserInfo")  text_info->m_Text = L"NickName:\nRank:";
            else if (objName == "UI_Layout_LobyCharacterDescrition")  text_info->m_Text = L"Chracter Info\n(Name, HP, etc.)";
        }
    }

    // Create Character Object
    {
        if (!m_CharacterRitems[(int)CHARACTER_TYPE::WARRIOR].empty())
        {
            auto newObj = objManager.CreateCharacterObject(
                objCB_index++, skinnedCB_index++,
                m_AllObjects,
                m_CharacterObjects, m_MaxCharacterObject,
                m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);

            std::string objName = "Warrior Instancing";

            float ConvertModelUnit = 125.0f;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
            objManager.SetObjectComponent(newObj, objName, m_CharacterRitems[(int)CHARACTER_TYPE::WARRIOR],
                ModelSkeletons["Male Knight 01"].get(),
                nullptr, &LocalRotationEuler, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);

            auto skeletonInfo = newObj->m_SkeletonInfo.get();
            auto animInfo = skeletonInfo->m_AnimInfo.get();
            animInfo->Init();
            std::set<std::string> AnimNameList; skeletonInfo->m_Skeleton->GetAnimationList_Name(AnimNameList);
            animInfo->AutoApplyActionFromSkeleton(AnimNameList);
            animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            animInfo->AnimLoop(aiModelData::AnimActionType::Idle);
        }
    }

    // Create DeActive Objects
    {
        const UINT nDeAcativateCharacterObj = m_MaxCharacterObject - (UINT)m_CharacterObjects.size();
        for (UINT i = 0; i < nDeAcativateCharacterObj; ++i)
        {
            auto newObj = objManager.CreateCharacterObject(
                objCB_index++, skinnedCB_index++,
                m_AllObjects,
                m_CharacterObjects, m_MaxCharacterObject,
                m_WorldObjects, m_MaxWorldObject);
            //newObj->Activated = false;
            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
        }

        const UINT nDeAcativateWorldObj = m_MaxWorldObject - (UINT)m_WorldObjects.size();
        for (UINT i = 0; i < nDeAcativateWorldObj; ++i)
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);
        }

        for (UINT i = m_MaxCharacterObject - nDeAcativateCharacterObj; i < m_MaxCharacterObject; ++i)
            m_CharacterObjects[i]->Activated = false;
        for (UINT i = m_MaxWorldObject - nDeAcativateWorldObj; i < m_MaxWorldObject; ++i)
            m_WorldObjects[i]->Activated = false;
    }

    m_nObjCB = (UINT)m_WorldObjects.size() + (UINT)m_UILayOutObjects.size();
    m_nSKinnedCB = (UINT)m_CharacterObjects.size();
    m_nTextBatch = (UINT)m_TextObjects.size();
}

void LobyScene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
    Scene::UpdateObjectCBs(objCB, gt);
}

void LobyScene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    Scene::UpdateSkinnedCBs(skinnedCB, gt);
}

void LobyScene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
    Scene::UpdateMaterialCBs(matCB, gt);
}

void LobyScene::UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt)
{
    Scene::UpdateMainPassCB(passCB, gt);
}

void LobyScene::UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::UpdateShadowPassCB(passCB, shadow_map, gt);
}

void LobyScene::UpdateShadowTransform(CTimer& gt)
{
    Scene::UpdateShadowTransform(gt);
}

void LobyScene::UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    ObjectManager ObjManager;
    Object* ChattingLogObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyChattingLog");
    Object* UserInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyUserInfo");
    Object* CharacterInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyCharacterDescrition");
    Object* SelectedCharacterNameObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyCharacterName");
    Object* MatchWaitingInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_MatchWaiting");

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

    auto& UserInfo = UserInfoObject->m_Textinfo->m_Text;
    UserInfo = L"UserName: " + UserInfo_UserName + L'\n';
    UserInfo += L"UserRank: " + std::to_wstring(UserInfo_UserRank);

    auto& CharacterInfo = CharacterInfoObject->m_Textinfo->m_Text;
    auto& SelectedCharacterName = SelectedCharacterNameObject->m_Textinfo->m_Text;
    switch (SelectedCharacterType)
    {
    case CHARACTER_TYPE::WARRIOR:
        SelectedCharacterName = L"Warrior";
        CharacterInfo = L"Name: Warrior\n";
        CharacterInfo += L"HP: 100\n";
        CharacterInfo += L"Damage: 30\n";
        CharacterInfo += L"Movement Speed: 1.0\n";
        CharacterInfo += L"Attack Speed: 1.0\n";
        CharacterInfo += L"Skill Name: Sword Wave\n";
        CharacterInfo += L"Skill Desc: Shoot 3 SwordWaves ahead.\n";
        CharacterInfo += L"Skill CoolTime: 4 seconds.";
        break;
    case CHARACTER_TYPE::BERSERKER:
        SelectedCharacterName = L"Berserker";
        CharacterInfo  = L"Name: Berserker\n";
        CharacterInfo += L"HP: 150\n";
        CharacterInfo += L"Damage: 40\n";
        CharacterInfo += L"Movement Speed: 1.0\n";
        CharacterInfo += L"Attack Speed: 0.6\n";
        CharacterInfo += L"Skill Name: Fury Roar";
        CharacterInfo += L"Skill Desc: Movement & Attack speed\n  are multiplied(x2) for 8 seconds.";
        CharacterInfo += L"Skill CoolTime: 10 seconds.";
        break;
    case CHARACTER_TYPE::ASSASSIN:
        SelectedCharacterName = L"Assassin";
        CharacterInfo  = L"Name: Assassin\n";
        CharacterInfo += L"HP: 100\n";
        CharacterInfo += L"Damage: 30\n";
        CharacterInfo += L"Movement Speed: 1.4\n";
        CharacterInfo += L"Attack Speed: 1.2\n";
        CharacterInfo += L"Skill Name: Stealth";
        CharacterInfo += L"Skill Desc: Stealth for 7 seconds.";
        CharacterInfo += L"Skill CoolTime: 12 seconds.";
        break;
    case CHARACTER_TYPE::PRIEST:
        SelectedCharacterName = L"Priest";
        CharacterInfo  = L"Name: Priest\n";
        CharacterInfo += L"HP: 80\n";
        CharacterInfo += L"Damage: 15\n";
        CharacterInfo += L"Movement Speed: 0.8\n";
        CharacterInfo += L"Attack Speed: 0.8\n";
        CharacterInfo += L"Skill Name: Holy Area";
        CharacterInfo += L"Skill Desc: Creates a holy area\n  that restores 15 health every 1 sec\n  for 10 sec.\n";
        CharacterInfo += L"Skill CoolTime: 12 seconds.";
        break;
    case CHARACTER_TYPE::NON:
        SelectedCharacterName = L"Non";
        CharacterInfo  = L"Name: Non\n";
        CharacterInfo += L"HP: Non\n";
        CharacterInfo += L"Damage: Non\n";
        CharacterInfo += L"Movement Speed: Non\n";
        CharacterInfo += L"Attack Speed: Non\n";
        CharacterInfo += L"Skill Name: Non\n";
        CharacterInfo += L"Skill Desc: Non\n";
        CharacterInfo += L"Skill CoolTime: Non";
        break;
    }

    auto MatchWaitingInfo = MatchWaitingInfoObject->m_Textinfo->m_Text;
    if (StartMatching == true) MatchWaitingInfo = L"Game Matching...";
    else
    {
        if (MatchWaitingInfo.empty() != true) MatchWaitingInfo.clear();
    }

}

void LobyScene::AnimateLights(CTimer& gt)
{
    float deg2rad = MathHelper::Pi / 180.0f;
    m_LightRotationAngle += (10.0f * deg2rad) * gt.GetTimeElapsed();

    XMMATRIX R = XMMatrixRotationY(m_LightRotationAngle);
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR lightDir = XMLoadFloat3(&m_BaseLightDirections[i]);
        lightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&m_RotatedLightDirections[i], lightDir);
    }
}

void LobyScene::AnimateSkeletons(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
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
        }
    }
}

void LobyScene::AnimateCameras(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    static XMFLOAT3 LookAtPosition = { 0.0f, 30.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    float deg2rad = MathHelper::Pi / 180.0f;
    camAngle += (10.0f * deg2rad) * gt.GetTimeElapsed();
    if ((int)camAngle / 360 == 1) camAngle -= 360.0f;
    float phi = 70.0f * deg2rad;
    float rad = 500.0f;
    XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
    Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
    XMStoreFloat3(&EyePosition, Eye_Pos);

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}

void LobyScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (OnceGetUserInfo == true)
    {
        EventManager eventManager;
        eventManager.ReservateEvent_GetUserInfo(GeneratedEvents);
        OnceGetUserInfo = false;
    }

    // Process SelectedCharacter
    {

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

    //Try matchmaking. e -> enqueue, d -> dequeue.
    if (key_state['E'] == true && StartMatching == false && OnceTryGameMatching == false)
    {
        EventManager eventManager;
        eventManager.ReservateEvent_TryGameMatching(GeneratedEvents, SelectedCharacterType);
        OnceTryGameMatching = true;
    }
    if (key_state['D'] == true && StartMatching == true && OnceTryGameMatching == false)
    {
        EventManager eventManager;
        eventManager.ReservateEvent_TryGameMatching(GeneratedEvents, SelectedCharacterType);
        OnceTryGameMatching = true;
    }
    if (OnceAccessMatch == true) {
        EventManager eventManager;
        eventManager.ReservateEvent_TryMatchLogin(GeneratedEvents, UserInfo_UserName, (char)SelectedCharacterType);
        OnceAccessMatch = false;
    }

    //TestFunc. - Access battle directly.
    if (key_state['R'] == true && OnceAccessBattleDirectly == false) {
        EventManager eventManager;
        eventManager.ReservateEvent_TryMatchLogin(GeneratedEvents, UserInfo_UserName, (char)SelectedCharacterType);
        OnceAccessBattleDirectly = true;
    }
}



void LobyScene::SetUserInfo(std::wstring UserName, int UserRank)
{
    UserInfo_UserName = UserName;
    UserInfo_UserRank = UserRank;
}

void LobyScene::SetChatLog(std::wstring UserName, std::wstring Message)
{
    if (ChattingList.size() == MaxChatLog) ChattingList.pop_back();
    ChattingList.push_back(L"[" + UserName + L"]: " + Message);
}

void LobyScene::SetMatchStatus(bool status)
{
    StartMatching = status;
    OnceTryGameMatching = false;
}

void LobyScene::SetAccessMatch(bool status)
{
    OnceAccessMatch = status;
}
