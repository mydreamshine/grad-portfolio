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
    float deg2rad = MathHelper::Pi / 180.0f;

    float Yaw = 0.0f;
    float texAlpha = 1.0f;
    XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
    bool IdleActionSet = true;
    for (auto& CharacterObj : m_CharacterObjects)
    {
        if (CharacterObj->m_Name.find("Warrior") != std::string::npos)
        {
            Yaw = 270.0f;
            texAlpha = 1.0f;
            IdleActionSet = true;
        }
        else if (CharacterObj->m_Name.find("Berserker") != std::string::npos)
        {
            Yaw = 180.0f;
            texAlpha = 0.2f;
            IdleActionSet = false;
        }
        else if (CharacterObj->m_Name.find("Assassin") != std::string::npos)
        {
            Yaw = 0.0f;
            texAlpha = 0.2f;
            IdleActionSet = false;
        }
        else if (CharacterObj->m_Name.find("Priest") != std::string::npos)
        {
            Yaw = 90.0f;
            texAlpha = 0.2f;
            IdleActionSet = false;
        }

        XMVECTOR CartesianPos = MathHelper::SphericalToCartesian(200.0f, Yaw * deg2rad, 90.0f * deg2rad);
        XMStoreFloat3(&WorldPosition, CartesianPos);

        CharacterObj->m_TransformInfo->SetWorldPosition(WorldPosition);
        CharacterObj->m_TransformInfo->UpdateWorldTransform();
        CharacterObj->m_TransformInfo->m_TexAlpha = texAlpha;

        auto AnimInfo = CharacterObj->m_SkeletonInfo->m_AnimInfo.get();
        if (IdleActionSet == true)
        {
            AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            AnimInfo->AnimLoop(aiModelData::AnimActionType::Idle);
        }
        else
        {
            AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            AnimInfo->AnimStop(aiModelData::AnimActionType::Idle);
        }
    }

    ObjectManager ObjManager;

    Object* SelectionLeftButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_CharacterSelection_LeftButton");
    Object* SelectionRightButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_CharacterSelection_RightButton");
    Object* PlayButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameStartButton");

    auto& AllRitems = *m_AllRitemsRef;
    auto Ritem_playButton = AllRitems.find("UI_Layout_GameStartButton")->second.get();
    auto Ritem_leftButton = AllRitems.find("UI_Layout_CharacterSelection_LeftButton")->second.get();
    auto Ritem_rightButton = AllRitems.find("UI_Layout_CharacterSelection_RightButton")->second.get();
    PlayButton->m_RenderItems = { Ritem_playButton };
    SelectionLeftButton->m_RenderItems = { Ritem_leftButton };
    SelectionRightButton->m_RenderItems = { Ritem_rightButton };

    m_SceneBounds.Radius = (float)SIZE_ShadowMap;


    camAngle = -90.0f * deg2rad;

    // Init Chatting Texts
    {
        Object* InputChatTextRenderObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyChattingInputBox");
        Object* ChattingListTextRenderObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyChattingLog");
        Object* InputChatCaretObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LobyChattingInputBoxCaret");

        inputTextBox.Init();
        inputTextBox.SetTextRenderObj(InputChatTextRenderObject);
        inputTextBox.SetCaretRenderObj(InputChatCaretObject);
        inputTextBox.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ"].get());
        inputTextBox.SetStartInputForm(L"Unknown: ");
        inputTextBox.SetMaxSize(MaxChattingLineWidth, 24.0f);
        inputTextBox.InitTexts();

        ChattinglistBox.Init();
        ChattinglistBox.SetTextRenderObj(ChattingListTextRenderObject);
        ChattinglistBox.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ"].get());
        ChattinglistBox.SetLineTabCharacter(L':');
        ChattinglistBox.SetMaxSize(MaxChattingLineWidth, MaxChattingLineHeight);
    }


    UserInfo_UserName.clear();
    UserInfo_UserRank = 0;

    SelectedCharacterType = CHARACTER_TYPE::WARRIOR;
    CharacterTurnTableYaw = 0.0f;
    AmountTurnTableYaw = 0.0f;
    savedTurnTableYaw = 0.0f;
    OnceSavedTurnTableYaw = false;

    CharacterSelectionDone = true;
    CharacterSelectLeft = false;
    CharacterSelectRight = false;
    LeftButtonPress = false;
    RightButtonPress = false;
    LeftButtonUp = true;
    RightButtonUp = true;
    PlayButtonPress = false;
    PlayButtonUp = true;
    CancelButtonPress = false;
    CancelButtonUp = true;
    MatchingTextInfoTimeStack = 0.0f;
    MatchingTextInfoChangeStack = 0;
    CurrButtonIsPlayButton = true;
    ChangeButton = false;

    ChattingModeTimeStack = 0.0f;
    ActivateChattingModeCoolTime = false;

    StartMatching = false;

    OnceGetUserInfo = true;
    OnceTryGameMatching = false;
    OnceSendChatLog = false;

    OnceAccessMatch = false;
    OnceAccessBattleDirectly = false;
}

void LobyScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt,
    std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    LobyScene::AnimateWorldObjectsTransform(gt);
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt, GeneratedEvents);
}

void LobyScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;

    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    XMFLOAT2 InputCaretPos = { 0.0f, 0.0f };

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
            if (Ritem_name.find("Press") != std::string::npos) continue;
            if (Ritem_name.find("Cancel") != std::string::npos) continue;
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, maxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            std::vector<RenderItem*> Ritems = { Ritem };
            objManager.SetObjectComponent(newObj, objName, Ritems);

            if (objName.find("CharacterSelection") != std::string::npos) continue;
            if (objName.find("Background") != std::string::npos) continue;
            if (objName.find("Caret") != std::string::npos) continue;

            auto newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
            auto text_info = newLayoutTextObj->m_Textinfo.get();
            newLayoutTextObj->m_Name = "Text" + objName;
            text_info->m_FontName = L"¸¼Àº °íµñ";
            text_info->m_TextColor = DirectX::Colors::Blue;
            auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            auto UI_LayoutExtents = Ritem->Geo->DrawArgs[objName].Bounds.Extents;
            text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
            text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;

            if (objName == "UI_Layout_LobyChattingLog")
            {
                text_info->m_TextPos.y = UI_LayoutPos.y - UI_LayoutExtents.y + 30.7f;
                text_info->m_TextPos.y += 26.8f;
                text_info->m_TextPos.x = (UI_LayoutPos.x - UI_LayoutExtents.x) + 13.0f;
                text_info->m_TextPos.y = m_height / 2.0f - text_info->m_TextPos.y; // offset
                text_info->m_TextPos.x += m_width / 2.0f;  // offset
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                text_info->m_TextColor = DirectX::Colors::GhostWhite;
            }
            else if (objName == "UI_Layout_LobyChattingInputBox")
            {
                text_info->m_TextPos.x = (UI_LayoutPos.x - UI_LayoutExtents.x) + 8.0f;
                text_info->m_TextPos.x += m_width / 2.0f;
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                text_info->m_TextColor = DirectX::Colors::GhostWhite;
                InputCaretPos = text_info->m_TextPos;
            }
            else if (objName == "UI_Layout_GameStartButton")
            {
                text_info->m_TextPos.x = (UI_LayoutPos.x - UI_LayoutExtents.x) + 36.0f;
                text_info->m_TextPos.x += m_width / 2.0f;
                text_info->m_TextPos.y = -(UI_LayoutPos.y + UI_LayoutExtents.y + 10.0f);
                text_info->m_TextPos.y += m_height / 2.0f;
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                text_info->m_TextColor = DirectX::Colors::GhostWhite;
            }
            else if (objName == "UI_Layout_LobyCharacterName")
            {
                text_info->m_Text = L"Character Name";
                text_info->m_TextColor = DirectX::Colors::GhostWhite;
            }
            else if (objName == "UI_Layout_LobyUserInfo")
            {
                text_info->m_Text = L"NickName:\nRank:";
                text_info->m_TextPos.x = (UI_LayoutPos.x - UI_LayoutExtents.x) + 8.0f;
                text_info->m_TextPos.x += m_width / 2.0f;
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                text_info->m_TextColor = DirectX::Colors::Blue;
            }
            else if (objName == "UI_Layout_LobyCharacterDescrition")
            {
                text_info->m_Text = L"Chracter Info\n(Name, HP, etc.)";
                text_info->m_TextColor = DirectX::Colors::Yellow;
            }
        }
    }

    // Init Chatting Texts
    {
        ObjectManager ObjManager;
        Object* InputChatTextRenderObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyChattingInputBox");
        Object* ChattingListTextRenderObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyChattingLog");
        Object* InputChatCaretObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LobyChattingInputBoxCaret");

        inputTextBox.Init();
        inputTextBox.SetTextRenderObj(InputChatTextRenderObject);
        inputTextBox.SetCaretRenderObj(InputChatCaretObject);
        inputTextBox.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ"].get());
        inputTextBox.SetStartInputForm(L"Unknown: ");
        inputTextBox.SetMaxSize(MaxChattingLineWidth, 24.0f);
        inputTextBox.InitTexts();

        ChattinglistBox.Init();
        ChattinglistBox.SetTextRenderObj(ChattingListTextRenderObject);
        ChattinglistBox.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ"].get());
        ChattinglistBox.SetLineTabCharacter(L':');
        ChattinglistBox.SetMaxSize(MaxChattingLineWidth, MaxChattingLineHeight);
    }

    // Create Character Object
    {
        const float ConvertModelUnit = 125.0f;
        const XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
        const XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
        float texAlpha = 1.0f;
        std::string objName;
        aiModelData::aiSkeleton* CharacterSkeleton = nullptr;
        float deg2rad = MathHelper::Pi / 180.0f;

        for (int i = 0; i < (int)CHARACTER_TYPE::COUNT; ++i)
        {
            if (m_CharacterRitems[i].empty() == true) continue;

            auto newCharacterObj = objManager.CreateCharacterObject(
                objCB_index++, skinnedCB_index++,
                m_AllObjects,
                m_CharacterObjects, m_MaxCharacterObject,
                m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newCharacterObj);

            if (i == (int)CHARACTER_TYPE::WARRIOR)
            {
                XMVECTOR CartesianPos = MathHelper::SphericalToCartesian(200.0f, 270.0f * deg2rad, 90.0f * deg2rad);
                XMStoreFloat3(&WorldPosition, CartesianPos);
                texAlpha = 1.0f;
                objName = "Warrior Instancing";
                CharacterSkeleton = ModelSkeletons["Male Knight 01"].get();
            }
            else if (i == (int)CHARACTER_TYPE::BERSERKER)
            {
                XMVECTOR CartesianPos = MathHelper::SphericalToCartesian(200.0f, 180.0f * deg2rad, 90.0f * deg2rad);
                XMStoreFloat3(&WorldPosition, CartesianPos);
                texAlpha = 0.2f;
                objName = "Berserker Instancing";
                CharacterSkeleton = ModelSkeletons["Male Warrior 01"].get();
            }
            else if (i == (int)CHARACTER_TYPE::ASSASSIN)
            {
                XMVECTOR CartesianPos = MathHelper::SphericalToCartesian(200.0f, 0.0f, 90.0f * deg2rad);
                XMStoreFloat3(&WorldPosition, CartesianPos);
                texAlpha = 0.2f;
                objName = "Assassin Instancing";
                CharacterSkeleton = ModelSkeletons["Female Warrior 01"].get();
            }
            else if (i == (int)CHARACTER_TYPE::PRIEST)
            {
                XMVECTOR CartesianPos = MathHelper::SphericalToCartesian(200.0f, 90.0f * deg2rad, 90.0f * deg2rad);
                XMStoreFloat3(&WorldPosition, CartesianPos);
                texAlpha = 0.2f;
                objName = "Priest Instancing";
                CharacterSkeleton = ModelSkeletons["Male Mage 01"].get();
            }

            objManager.SetObjectComponent(newCharacterObj, objName,
                m_CharacterRitems[i], CharacterSkeleton,
                nullptr, nullptr, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);

            newCharacterObj->m_TransformInfo->m_TexAlpha = texAlpha;

            auto skeletonInfo = newCharacterObj->m_SkeletonInfo.get();
            auto animInfo = skeletonInfo->m_AnimInfo.get();
            animInfo->Init();
            std::set<std::string> AnimNameList; skeletonInfo->m_Skeleton->GetAnimationList_Name(AnimNameList);
            animInfo->AutoApplyActionFromSkeleton(AnimNameList);
            if (i == (int)CHARACTER_TYPE::WARRIOR)
            {
                animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                animInfo->AnimLoop(aiModelData::AnimActionType::Idle);
            }
            else
            {
                animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                animInfo->AnimStop(aiModelData::AnimActionType::Idle);
            }
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

    m_SceneBounds.Radius = (float)SIZE_ShadowMap;

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
    Object* UserInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyUserInfo");
    Object* CharacterInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyCharacterDescrition");
    Object* SelectedCharacterNameObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_LobyCharacterName");

    // Update Input Box Caret
    inputTextBox.Update(gt, (float)m_width, (float)m_height);

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
        CharacterInfo += L"Attack Speed: 1.0\n";
        CharacterInfo += L"Movement Speed: 360\n";
        CharacterInfo += L"Skill Name: Sword Wave\n";
        CharacterInfo += L"Skill Desc: Shoot 3 Sword\nWaves ahead.\n";
        CharacterInfo += L"Skill CoolTime: 4 seconds.";
        break;
    case CHARACTER_TYPE::BERSERKER:
        SelectedCharacterName = L"Berserker";
        CharacterInfo  = L"Name: Berserker\n";
        CharacterInfo += L"HP: 150\n";
        CharacterInfo += L"Damage: 40\n";
        CharacterInfo += L"Attack Speed: 0.6\n";
        CharacterInfo += L"Movement Speed: 300\n";
        CharacterInfo += L"Skill Name: Fury Roar\n";
        CharacterInfo += L"Skill Desc: Movement \n& Attack speed are \nmultiplied(x2) for\n8 seconds.\n";
        CharacterInfo += L"Skill CoolTime: 10 seconds.";
        break;
    case CHARACTER_TYPE::ASSASSIN:
        SelectedCharacterName = L"Assassin";
        CharacterInfo  = L"Name: Assassin\n";
        CharacterInfo += L"HP: 100\n";
        CharacterInfo += L"Damage: 30\n";
        CharacterInfo += L"Attack Speed: 1.2\n";
        CharacterInfo += L"Movement Speed: 380\n";
        CharacterInfo += L"Skill Name: Stealth\n";
        CharacterInfo += L"Skill Desc: Stealth for 7\nseconds.\n";
        CharacterInfo += L"Skill CoolTime: 12 seconds.";
        break;
    case CHARACTER_TYPE::PRIEST:
        SelectedCharacterName = L"Priest";
        CharacterInfo  = L"Name: Priest\n";
        CharacterInfo += L"HP: 80\n";
        CharacterInfo += L"Damage: 15\n";
        CharacterInfo += L"Movement Speed: 1.0\n";
        CharacterInfo += L"Attack Speed: 360\n";
        CharacterInfo += L"Skill Name: Holy Area\n";
        CharacterInfo += L"Skill Desc: Creates a\nheal area that restores\n15 health every 1 sec\nfor 10 sec.\n";
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

}

void LobyScene::AnimateLights(CTimer& gt)
{
    float deg2rad = MathHelper::Pi / 180.0f;
    m_LightRotationAngle += (40.0f * deg2rad) * gt.GetTimeElapsed();

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
        SkeletonInfo->m_Skeleton->UpdateAnimationTransforms(*(AnimInfo), gt.GetTimeElapsed());
    }
}

void LobyScene::AnimateCameras(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    static XMFLOAT3 LookAtPosition = { 0.0f, 40.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    float deg2rad = MathHelper::Pi / 180.0f;
    float phi = 75.0f * deg2rad;
    float rad = 750.0f;
    XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
    Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
    XMStoreFloat3(&EyePosition, Eye_Pos);

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}

void LobyScene::AnimateWorldObjectsTransform(CTimer& gt)
{
    if (CharacterSelectionDone == false)
    {
        if (OnceSavedTurnTableYaw == false)
        {
            savedTurnTableYaw = ((int)CharacterTurnTableYaw / 10) * 10.0f;
            OnceSavedTurnTableYaw = true;

            for (auto& CharacterObj : m_CharacterObjects)
            {
                auto AnimInfo = CharacterObj->m_SkeletonInfo->m_AnimInfo.get();
                AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                AnimInfo->AnimStop(aiModelData::AnimActionType::Idle);
            }
        }

        if (CharacterSelectLeft == true)
        {
            CharacterTurnTableYaw -= 90.0f * gt.GetTimeElapsed();

            if (savedTurnTableYaw - CharacterTurnTableYaw >= 90.0f)
            {
                CharacterTurnTableYaw = savedTurnTableYaw - 90.0f;
                if (CharacterTurnTableYaw <= -360.0f) CharacterTurnTableYaw = 0.0f;
                CharacterSelectionDone = true;
                OnceSavedTurnTableYaw = false;
                CharacterSelectLeft = false;
            }
        }
        else if (CharacterSelectRight == true)
        {
            CharacterTurnTableYaw += 90.0f * gt.GetTimeElapsed();

            if (CharacterTurnTableYaw - savedTurnTableYaw >= 90.0f)
            {
                CharacterTurnTableYaw = savedTurnTableYaw + 90.0f;
                if (CharacterTurnTableYaw >= 360.0f) CharacterTurnTableYaw = 0.0f;
                CharacterSelectionDone = true;
                OnceSavedTurnTableYaw = false;
                CharacterSelectRight = false;
            }
        }

        float deg2rad = MathHelper::Pi / 180.0f;
        XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
        float Yaw = 0.0f;
        float texAlpha = 1.0f;

        for (auto& CharacterObj : m_CharacterObjects)
        {
            if (CharacterObj->m_Name.find("Warrior") != std::string::npos)
            {
                Yaw = CharacterTurnTableYaw + 270.0f;
                if (compareFloat(Yaw, 270.0f) || compareFloat(Yaw, -90.0f))
                {
                    SelectedCharacterType = CHARACTER_TYPE::WARRIOR;
                    texAlpha = 1.0f;
                    auto AnimInfo = CharacterObj->m_SkeletonInfo->m_AnimInfo.get();
                    AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                    AnimInfo->AnimLoop(aiModelData::AnimActionType::Idle);
                }
                else texAlpha = 0.2f;
            }
            else if (CharacterObj->m_Name.find("Berserker") != std::string::npos)
            {
                Yaw = CharacterTurnTableYaw + 180.0f;
                if (compareFloat(Yaw, 270.0f) || compareFloat(Yaw, -90.0f))
                {
                    SelectedCharacterType = CHARACTER_TYPE::BERSERKER;
                    texAlpha = 1.0f;
                    auto AnimInfo = CharacterObj->m_SkeletonInfo->m_AnimInfo.get();
                    AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                    AnimInfo->AnimLoop(aiModelData::AnimActionType::Idle);
                }
                else texAlpha = 0.2f;
            }
            else if (CharacterObj->m_Name.find("Assassin") != std::string::npos)
            {
                Yaw = CharacterTurnTableYaw + 0.0f;
                if (compareFloat(Yaw, 270.0f) || compareFloat(Yaw, -90.0f))
                {
                    SelectedCharacterType = CHARACTER_TYPE::ASSASSIN;
                    texAlpha = 1.0f;
                    auto AnimInfo = CharacterObj->m_SkeletonInfo->m_AnimInfo.get();
                    AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                    AnimInfo->AnimLoop(aiModelData::AnimActionType::Idle);
                }
                else texAlpha = 0.2f;
            }
            else if (CharacterObj->m_Name.find("Priest") != std::string::npos)
            {
                Yaw = CharacterTurnTableYaw + 90.0f;
                if (compareFloat(Yaw, 270.0f) || compareFloat(Yaw, -90.0f))
                {
                    SelectedCharacterType = CHARACTER_TYPE::PRIEST;
                    texAlpha = 1.0f;
                    auto AnimInfo = CharacterObj->m_SkeletonInfo->m_AnimInfo.get();
                    AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
                    AnimInfo->AnimLoop(aiModelData::AnimActionType::Idle);
                }
                else texAlpha = 0.2f;
            }

            XMVECTOR CartesianPos = MathHelper::SphericalToCartesian(200.0f, Yaw * deg2rad, 90.0f * deg2rad);
            XMStoreFloat3(&WorldPosition, CartesianPos);

            CharacterObj->m_TransformInfo->SetWorldPosition(WorldPosition);
            CharacterObj->m_TransformInfo->UpdateWorldTransform();
            CharacterObj->m_TransformInfo->m_TexAlpha = texAlpha;
        }
    }
}

void LobyScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (OnceGetUserInfo == true)
    {
        EventManager eventManager;
        eventManager.ReservateEvent_GetUserInfo(GeneratedEvents);
        OnceGetUserInfo = false;
    }

    if (key_state[VK_LBUTTON] == true)
    {
        ObjectManager ObjManager;
        Object* SelectionLeftButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_CharacterSelection_LeftButton");
        Object* SelectionRightButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_CharacterSelection_RightButton");
        Object* PlayButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameStartButton");

        auto& LeftButtonBound = SelectionLeftButton->m_TransformInfo->m_Bound;
        auto& RightButtonBound = SelectionRightButton->m_TransformInfo->m_Bound;
        auto& PlayButtonBound = PlayButton->m_TransformInfo->m_Bound;

        RECT LeftButtonAreainScreen =
        {
            (LONG)(LeftButtonBound.Center.x - LeftButtonBound.Extents.x), // left
            (LONG)(LeftButtonBound.Center.y + LeftButtonBound.Extents.y), // top
            (LONG)(LeftButtonBound.Center.x + LeftButtonBound.Extents.x), // right
            (LONG)(LeftButtonBound.Center.y - LeftButtonBound.Extents.y), // bottom
        };

        RECT RightButtonAreainScreen =
        {
            (LONG)(RightButtonBound.Center.x - RightButtonBound.Extents.x), // left
            (LONG)(RightButtonBound.Center.y + RightButtonBound.Extents.y), // top
            (LONG)(RightButtonBound.Center.x + RightButtonBound.Extents.x), // right
            (LONG)(RightButtonBound.Center.y - RightButtonBound.Extents.y), // bottom
        };

        RECT PlayButtonAreainScreen =
        {
            (LONG)(PlayButtonBound.Center.x - PlayButtonBound.Extents.x), // left
            (LONG)(PlayButtonBound.Center.y + PlayButtonBound.Extents.y), // top
            (LONG)(PlayButtonBound.Center.x + PlayButtonBound.Extents.x), // right
            (LONG)(PlayButtonBound.Center.y - PlayButtonBound.Extents.y), // bottom
        };

        float CursorPos_x = oldCursorPos.x - m_width * 0.5f;
        float CursorPos_y = -(oldCursorPos.y - m_height * 0.5f);

        // Process SelectedCharacter
        if (PointInRect(CursorPos_x, CursorPos_y, LeftButtonAreainScreen) == true)
        {
            if (CharacterSelectionDone == true)
            {
                CharacterSelectLeft = true;
                CharacterSelectionDone = false;
            }

            if (LeftButtonPress == false && LeftButtonUp == true)
            {
                auto& AllRitems = *m_AllRitemsRef;
                auto Ritem_leftButtonPress = AllRitems.find("UI_Layout_CharacterSelection_LeftButton_Press")->second.get();
                SelectionLeftButton->m_RenderItems = { Ritem_leftButtonPress };
                LeftButtonPress = true;
                LeftButtonUp = false;
            }
        }
        else if (PointInRect(CursorPos_x, CursorPos_y, RightButtonAreainScreen) == true)
        {
            if (CharacterSelectionDone == true)
            {
                CharacterSelectRight = true;
                CharacterSelectionDone = false;
            }

            if (RightButtonPress == false && RightButtonUp == true)
            {
                auto& AllRitems = *m_AllRitemsRef;
                auto Ritem_rightButtonPress = AllRitems.find("UI_Layout_CharacterSelection_RightButton_Press")->second.get();
                SelectionRightButton->m_RenderItems = { Ritem_rightButtonPress };
                RightButtonPress = true;
                RightButtonUp = false;
            }
        }
        // Process Play
        else if (PointInRect(CursorPos_x, CursorPos_y, PlayButtonAreainScreen) == true)
        {
            if (CurrButtonIsPlayButton == true)
            {
                if (PlayButtonPress == false && PlayButtonUp == true)
                {
                    auto& AllRitems = *m_AllRitemsRef;
                    auto Ritem_playButtonPress = AllRitems.find("UI_Layout_GameStartButton_Press")->second.get();
                    PlayButton->m_RenderItems = { Ritem_playButtonPress };
                    PlayButtonPress = true;
                    PlayButtonUp = false;
                }
            }
            else
            {
                if (CancelButtonPress == false && CancelButtonUp == true)
                {
                    auto& AllRitems = *m_AllRitemsRef;
                    auto Ritem_cancelButtonPress = AllRitems.find("UI_Layout_MatchCancelButton_Press")->second.get();
                    PlayButton->m_RenderItems = { Ritem_cancelButtonPress };
                    CancelButtonPress = true;
                    CancelButtonUp = false;
                }
            }
        }
    }
    else
    {
        ObjectManager ObjManager;
        Object* SelectionLeftButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_CharacterSelection_LeftButton");
        Object* SelectionRightButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_CharacterSelection_RightButton");
        Object* PlayButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameStartButton");

        if (LeftButtonPress == true && LeftButtonUp == false)
        {
            auto& AllRitems = *m_AllRitemsRef;
            auto Ritem_leftButton = AllRitems.find("UI_Layout_CharacterSelection_LeftButton")->second.get();
            SelectionLeftButton->m_RenderItems = { Ritem_leftButton };
            LeftButtonPress = false;
            LeftButtonUp = true;
        }
        else if (RightButtonPress == true && RightButtonUp == false)
        {
            auto& AllRitems = *m_AllRitemsRef;
            auto Ritem_rightButton = AllRitems.find("UI_Layout_CharacterSelection_RightButton")->second.get();
            SelectionRightButton->m_RenderItems = { Ritem_rightButton };
            RightButtonPress = false;
            RightButtonUp = true;
        }
        else if (PlayButtonPress == true && PlayButtonUp == false)
        {
            auto& AllRitems = *m_AllRitemsRef;
            auto Ritem_playButton = AllRitems.find("UI_Layout_GameStartButton")->second.get();
            PlayButton->m_RenderItems = { Ritem_playButton };
            PlayButtonPress = false;
            PlayButtonUp = true;

            ChangeButton = true;

            //Try matchmaking. enqueue
            if (StartMatching == false && OnceTryGameMatching == false)
            {
                EventManager eventManager;
                eventManager.ReservateEvent_TryGameMatching(GeneratedEvents, SelectedCharacterType);
                OnceTryGameMatching = true;
            }

            // Match Status Test
            /*StartMatching = true;
            OnceTryGameMatching = false;*/
        }
        else if (CancelButtonPress == true && CancelButtonUp == false)
        {
            auto& AllRitems = *m_AllRitemsRef;
            auto Ritem_cancelButton = AllRitems.find("UI_Layout_MatchCancelButton")->second.get();
            PlayButton->m_RenderItems = { Ritem_cancelButton };
            CancelButtonPress = false;
            CancelButtonUp = true;

            ChangeButton = true;

            //Try matchmaking. dequeue
            if (StartMatching == true && OnceTryGameMatching == false)
            {
                EventManager eventManager;
                eventManager.ReservateEvent_TryGameMatching(GeneratedEvents, SelectedCharacterType);
                OnceTryGameMatching = true;
            }

            // Match Status Test
            /*StartMatching = false;
            OnceTryGameMatching = false;*/
        }

        if (ChangeButton == true && StartMatching == true && OnceTryGameMatching == false)
        {
            auto& AllRitems = *m_AllRitemsRef;
            auto Ritem_cancelButton = AllRitems.find("UI_Layout_MatchCancelButton")->second.get();
            PlayButton->m_RenderItems = { Ritem_cancelButton };
            ChangeButton = false;
            CurrButtonIsPlayButton = false;
        }
        else if (ChangeButton == true && StartMatching == false && OnceTryGameMatching == false)
        {
            auto& AllRitems = *m_AllRitemsRef;
            auto Ritem_playButton = AllRitems.find("UI_Layout_GameStartButton")->second.get();
            PlayButton->m_RenderItems = { Ritem_playButton };

            Object* MatchingInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_GameStartButton");
            MatchingInfoObject->m_Textinfo->m_Text.clear();

            ChangeButton = false;
            CurrButtonIsPlayButton = true;
        }

        if (CurrButtonIsPlayButton != true)
        {
            Object* MatchingInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_GameStartButton");
            auto TextInfo = MatchingInfoObject->m_Textinfo.get();
            TextInfo->m_Text = L"Matching";
            for (int i = 0; i < MatchingTextInfoChangeStack; ++i)
                TextInfo->m_Text += L" /";

            MatchingTextInfoTimeStack += gt.GetTimeElapsed();
            if (MatchingTextInfoTimeStack >= MatchingTextInfoInterval)
            {
                MatchingTextInfoChangeStack = (MatchingTextInfoChangeStack + 1) % 4;
                MatchingTextInfoTimeStack = 0.0f;
            }
        }
    }

    // Process Chat
    {
        ChattinglistBox.ProcessInput(key_state, oldCursorPos, gt, GeneratedEvents);

        if (key_state[VK_RETURN] == true)
        {
            if (ActivateChattingModeCoolTime == false)
                ChattingModeTimeStack = ChattingModeCoolTime;
            else
                ChattingModeTimeStack += gt.GetTimeElapsed();
            if (ChattingModeTimeStack >= ChattingModeCoolTime)
            {
                std::wstring PlayerNameInputForm;
                if (UserInfo_UserName.empty() == false)
                    PlayerNameInputForm = UserInfo_UserName + L": ";
                else
                    PlayerNameInputForm = L"Unknown: ";

                inputTextBox.SetActivate(false);
                if (inputTextBox.IsEmpty() == false)
                {
                    std::wstring FullinputTexts = inputTextBox.GetFullinputTexts();
                    EventManager eventManager;
                    eventManager.ReservateEvent_SendChatLog(GeneratedEvents, FEP_LOBY_SCENE, FullinputTexts);

                    // Chatting List Test
                    //ChattinglistBox.AddMessage(FullinputTexts);
                    //if (ChattinglistBox.IsSizeChanged() == true)
                    //{
                    //    ObjectManager ObjManager;
                    //    Object* ChattingLogLayerObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LobyChattingLog");
                    //    auto ChattingLogLayerTransform = ChattingLogLayerObject->m_TransformInfo.get();
                    //    auto ChattingLogLayerPos = ChattingLogLayerTransform->GetWorldPosition();
                    //    auto ChattingLogLayerExtents = ChattingLogLayerTransform->m_Bound.Extents;
                    //    auto ChattingLogRenderTextSize = ChattinglistBox.GetSize();

                    //    XMFLOAT2 newChattingListBoxPos = ChattinglistBox.GetPosition();
                    //    newChattingListBoxPos.y = ChattingLogLayerPos.y - ChattingLogLayerExtents.y + 30.7f;
                    //    newChattingListBoxPos.y += 26.8f;
                    //    newChattingListBoxPos.y += ChattingLogRenderTextSize.y * 0.5f;
                    //    newChattingListBoxPos.y = m_height / 2.0f - (newChattingListBoxPos.y); // Coord Offset
                    //    ChattinglistBox.SetPosition(newChattingListBoxPos);
                    //}

                    inputTextBox.SetStartInputForm(PlayerNameInputForm);
                    inputTextBox.InitTexts();
                }

                ActivateChattingModeCoolTime = true;
                ChattingModeTimeStack = 0.0f;
            }
        }
        else
        {
            ActivateChattingModeCoolTime = false;
        }

        inputTextBox.ProcessInput(key_state, oldCursorPos, gt, GeneratedEvents);
    }

    //Try matchmaking. e -> enqueue, d -> dequeue.
    /*if (key_state['E'] == true && StartMatching == false && OnceTryGameMatching == false)
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
    }*/
    if (OnceAccessMatch == true) {
        EventManager eventManager;
        eventManager.ReservateEvent_TryMatchLogin(GeneratedEvents, UserInfo_UserName, (char)SelectedCharacterType);
        OnceAccessMatch = false;
    }

    //TestFunc. - Access battle directly.
    if (key_state['Q'] == true && OnceAccessBattleDirectly == false) {
        EventManager eventManager;
        eventManager.ReservateEvent_TryMatchLogin(GeneratedEvents, UserInfo_UserName, (char)SelectedCharacterType);
        OnceAccessBattleDirectly = true;
    }
}



void LobyScene::SetUserInfo(std::wstring UserName, int UserRank)
{
    UserInfo_UserName = UserName;
    UserInfo_UserRank = UserRank;

    std::wstring PlayerNameInputForm;
    if (UserInfo_UserName.empty() == false)
        PlayerNameInputForm = UserInfo_UserName + L": ";
    else
        PlayerNameInputForm = L"Unknown: ";

    inputTextBox.SetStartInputForm(PlayerNameInputForm);
    inputTextBox.InitTexts();
}

void LobyScene::SetChatLog(std::wstring Message)
{
    ObjectManager ObjManager;

    ChattinglistBox.AddMessage(Message);
    if (ChattinglistBox.IsSizeChanged() == true)
    {
        Object* ChattingLogLayerObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LobyChattingLog");
        auto ChattingLogLayerTransform = ChattingLogLayerObject->m_TransformInfo.get();
        auto ChattingLogLayerPos = ChattingLogLayerTransform->GetWorldPosition();
        auto ChattingLogLayerExtents = ChattingLogLayerTransform->m_Bound.Extents;
        auto ChattingLogRenderTextSize = ChattinglistBox.GetSize();

        XMFLOAT2 newChattingListBoxPos = ChattinglistBox.GetPosition();
        newChattingListBoxPos.y = ChattingLogLayerPos.y - ChattingLogLayerExtents.y + 30.7f;
        newChattingListBoxPos.y += 26.8f;
        newChattingListBoxPos.y += ChattingLogRenderTextSize.y * 0.5f;
        newChattingListBoxPos.y = m_height / 2.0f - (newChattingListBoxPos.y); // Coord Offset
        ChattinglistBox.SetPosition(newChattingListBoxPos);
    }
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
