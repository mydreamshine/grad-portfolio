#include "stdafx.h"
#include "GameOverScene.h"


GameOverScene::~GameOverScene()
{
}

void GameOverScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    Scene::OnInit(device, commandList,
        objCB_index, skinnedCB_index, textBatch_index);
}

void GameOverScene::OnInitProperties(CTimer& gt)
{
    // Init Character Prop.
    {
        const float ConvertModelUnit = 85.0f;
        XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
        XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };

        for (auto& obj : m_CharacterObjects)
        {
            obj->RenderActivated = false;

            auto transformInfo = obj->m_TransformInfo.get();

            transformInfo->SetWorldTransform(WorldScale, WorldRotationEuler, WorldPosition);
            transformInfo->SetLocalRotationEuler(LocalRotationEuler);

            auto AnimInfo = obj->m_SkeletonInfo->m_AnimInfo.get();
            AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            AnimInfo->AnimStop(aiModelData::AnimActionType::Idle);
        }
    }

    m_LightRotationAngle = 0.0f;

    UserInfo_UserName.clear();
    UserInfo_UserRank = 0;

    GameMatchingResult_CountKill = 0;
    GameMatchingResult_CountDeath = 0;
    GameMatchingResult_CountAssistance = 0;
    GameMatchingResult_TotalDamage = 0;
    GameMatchingResult_TotalHeal = 0;
    GameMatchingResult_PlayedCharacterType = CHARACTER_TYPE::NON;

    ReturnLobyButtonPress = false;
    ReturnLobyButtonUp = true;

    OnceTryReturnLoby = false;

    ObjectManager ObjManager;
    Object* ReturnLobyButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_ReturnLobyButton");
    auto& AllRitems = *m_AllRitemsRef;
    auto Ritem_ButtonPress = AllRitems.find("UI_Layout_ReturnLobyButton")->second.get();
    ReturnLobyButton->m_RenderItems = { Ritem_ButtonPress };
}

void GameOverScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt,
    std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt, GeneratedEvents);
}

void GameOverScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;

    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;
    const UINT maxUILayOutObject = (UINT)Geometries["GameOverSceneUIGeo"]->DrawArgs.size();


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
        else if (Ritem_iter.first.find("UI") != std::string::npos)
        {
            if (Ritem_name.find("Press") != std::string::npos) continue;

            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, maxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            std::vector<RenderItem*> Ritems = { Ritem };

            objManager.SetObjectComponent(newObj, objName, Ritems);


            if (objName.find("GameOverInfo") == std::string::npos) continue;

            XMFLOAT3 UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            XMFLOAT3 UI_LayoutExtents = Ritem->Geo->DrawArgs[objName].Bounds.Extents;

            int TextObjCreatNum = 3;
            std::vector<Object*> additionalTextObjets;

            Object* newLayoutTextObj = nullptr;
            TextInfo* text_info = nullptr;
            for (int i = 0; i < TextObjCreatNum; ++i)
            {
                newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
                text_info = newLayoutTextObj->m_Textinfo.get();
                newLayoutTextObj->m_Name = "Text" + objName;
                text_info->m_FontName = L"¸¼Àº °íµñ";
                text_info->m_TextColor = DirectX::Colors::Yellow;
                text_info->m_TextPos.x = UI_LayoutPos.x;
                text_info->m_TextPos.y = UI_LayoutPos.y;
                text_info->m_TextPos.x += m_width / 2.0f; // offset
                text_info->m_TextPos.y = m_height / 2.0f - text_info->m_TextPos.y; // offset

                additionalTextObjets.push_back(newLayoutTextObj);
            }

            additionalTextObjets[0]->m_Name = "TextGameOverInfo_UserName";
            additionalTextObjets[0]->m_Textinfo->m_TextPos.y = UI_LayoutPos.y + UI_LayoutExtents.y - 21.0f;
            additionalTextObjets[0]->m_Textinfo->m_TextPos.y *= -1.0f;          // offset
            additionalTextObjets[0]->m_Textinfo->m_TextPos.y += m_height / 2.0f; // offset

            additionalTextObjets[1]->m_Name = "TextGameOverInfo_UserRank";
            additionalTextObjets[1]->m_Textinfo->m_TextPos.x = UI_LayoutPos.x + 15.0f;
            additionalTextObjets[1]->m_Textinfo->m_TextPos.x += m_width / 2.0f; // offset
            additionalTextObjets[1]->m_Textinfo->m_TextPos.y = UI_LayoutPos.y + UI_LayoutExtents.y - 72.0f;
            additionalTextObjets[1]->m_Textinfo->m_TextPos.y *= -1.0f;          // offset
            additionalTextObjets[1]->m_Textinfo->m_TextPos.y += m_height / 2.0f; // offset
            additionalTextObjets[1]->m_Textinfo->m_FontName = L"¸¼Àº °íµñ(16pt)";

            additionalTextObjets[2]->m_Name = "TextGameOverInfo_TotalScore";
            additionalTextObjets[2]->m_Textinfo->m_TextPos.y = UI_LayoutPos.y - 42.0f;
            additionalTextObjets[2]->m_Textinfo->m_TextPos.y *= -1.0f;          // offset
            additionalTextObjets[2]->m_Textinfo->m_TextPos.y += m_height / 2.0f; // offset
        }
    }

    // Create Character Object
    {
        const float ConvertModelUnit = 85.0f;
        const XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
        const XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
        std::string objName;
        aiModelData::aiSkeleton* CharacterSkeleton = nullptr;

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
                objName = "Warrior Instancing";
                CharacterSkeleton = ModelSkeletons["Male Knight 01"].get();
            }
            else if (i == (int)CHARACTER_TYPE::BERSERKER)
            {
                objName = "Berserker Instancing";
                CharacterSkeleton = ModelSkeletons["Male Warrior 01"].get();
            }
            else if (i == (int)CHARACTER_TYPE::ASSASSIN)
            {
                objName = "Assassin Instancing";
                CharacterSkeleton = ModelSkeletons["Female Warrior 01"].get();
            }
            else if (i == (int)CHARACTER_TYPE::PRIEST)
            {
                objName = "Priest Instancing";
                CharacterSkeleton = ModelSkeletons["Male Mage 01"].get();
            }

            objManager.SetObjectComponent(newCharacterObj, objName,
                m_CharacterRitems[i], CharacterSkeleton,
                nullptr, nullptr, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);

            newCharacterObj->RenderActivated = false;

            auto skeletonInfo = newCharacterObj->m_SkeletonInfo.get();
            auto AnimInfo = newCharacterObj->m_SkeletonInfo->m_AnimInfo.get();
            AnimInfo->Init();
            std::set<std::string> AnimNameList; skeletonInfo->m_Skeleton->GetAnimationList_Name(AnimNameList);
            AnimInfo->AutoApplyActionFromSkeleton(AnimNameList);
            if (i == (int)CHARACTER_TYPE::WARRIOR)
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

void GameOverScene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
    Scene::UpdateObjectCBs(objCB, gt);
}

void GameOverScene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    Scene::UpdateSkinnedCBs(skinnedCB, gt);
}

void GameOverScene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
    Scene::UpdateMaterialCBs(matCB, gt);
}

void GameOverScene::UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt)
{
    Scene::UpdateMainPassCB(passCB, gt);
}

void GameOverScene::UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::UpdateShadowPassCB(passCB, shadow_map, gt);
}

void GameOverScene::UpdateShadowTransform(CTimer& gt)
{
    Scene::UpdateShadowTransform(gt);
}

void GameOverScene::UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    ObjectManager ObjManager;
    Object* GameOverInfo_UserNameTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameOverInfo_UserName");
    Object* GameOverInfo_UserRankTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameOverInfo_UserRank");
    Object* GameOverInfo_TotalScoreTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameOverInfo_TotalScore");

    auto& UserNameRenderTexts = GameOverInfo_UserNameTextObject->m_Textinfo->m_Text;
    auto& UserRankRenderTexts = GameOverInfo_UserRankTextObject->m_Textinfo->m_Text;
    auto& TotalScoreRenderTexts = GameOverInfo_TotalScoreTextObject->m_Textinfo->m_Text;

    if (UserInfo_UserName.empty() == true)
        UserNameRenderTexts = L"Unknown";
    else
        UserNameRenderTexts = UserInfo_UserName;
    UserRankRenderTexts = std::to_wstring(UserInfo_UserRank);
    TotalScoreRenderTexts = L"Total Kill: " + std::to_wstring(GameMatchingResult_CountKill) + L"\n";
    TotalScoreRenderTexts += L"Total Death: " + std::to_wstring(GameMatchingResult_CountDeath) + L"\n";
    TotalScoreRenderTexts += L"Total Assistance: " + std::to_wstring(GameMatchingResult_CountAssistance) + L"\n";
    TotalScoreRenderTexts += L"Total Damage: " + std::to_wstring(GameMatchingResult_TotalDamage) + L"\n";
    TotalScoreRenderTexts += L"Total Heal: " + std::to_wstring(GameMatchingResult_TotalHeal);
}

void GameOverScene::AnimateLights(CTimer& gt)
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

void GameOverScene::AnimateSkeletons(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
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

void GameOverScene::AnimateCameras(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    static XMFLOAT3 LookAtPosition = { -100.0f, 60.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    float deg2rad = MathHelper::Pi / 180.0f;
    float camAngle = -75.0f * deg2rad;
    float phi = 70.0f * deg2rad;
    float rad = 340.0f;
    XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
    Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
    XMStoreFloat3(&EyePosition, Eye_Pos);

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}

void GameOverScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    // Process Return Loby Button
    {
        if (key_state[VK_LBUTTON] == true)
        {
            ObjectManager ObjManager;
            Object* ReturnLobyButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_ReturnLobyButton");

            auto& ReturnLobyButtonBound = ReturnLobyButton->m_TransformInfo->m_Bound;

            RECT ReturnLobyButtonAreainScreen =
            {
                (LONG)(ReturnLobyButtonBound.Center.x - ReturnLobyButtonBound.Extents.x), // left
                (LONG)(ReturnLobyButtonBound.Center.y + ReturnLobyButtonBound.Extents.y), // top
                (LONG)(ReturnLobyButtonBound.Center.x + ReturnLobyButtonBound.Extents.x), // right
                (LONG)(ReturnLobyButtonBound.Center.y - ReturnLobyButtonBound.Extents.y), // bottom
            };

            float CursorPos_x = oldCursorPos.x - m_width * 0.5f;
            float CursorPos_y = -(oldCursorPos.y - m_height * 0.5f);

            // Process SelectedCharacter
            if (PointInRect(CursorPos_x, CursorPos_y, ReturnLobyButtonAreainScreen) == true)
            {
                if (ReturnLobyButtonPress == false && ReturnLobyButtonUp == true)
                {
                    auto& AllRitems = *m_AllRitemsRef;
                    auto Ritem_ButtonPress = AllRitems.find("UI_Layout_ReturnLobyButton_Press")->second.get();
                    ReturnLobyButton->m_RenderItems = { Ritem_ButtonPress };

                    ReturnLobyButtonPress = true;
                    ReturnLobyButtonUp = false;
                }
            }
        }
        else
        {
            ObjectManager ObjManager;
            Object* ReturnLobyButton = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_ReturnLobyButton");

            if (ReturnLobyButtonPress == true && ReturnLobyButtonUp == false)
            {
                auto& AllRitems = *m_AllRitemsRef;
                auto Ritem_ButtonPress = AllRitems.find("UI_Layout_ReturnLobyButton")->second.get();
                ReturnLobyButton->m_RenderItems = { Ritem_ButtonPress };

                ReturnLobyButtonPress = false;
                ReturnLobyButtonUp = true;

                OnceTryReturnLoby = true;
            }
        }
    }

    if (OnceTryReturnLoby == true)
    {
        EventManager eventManger;
        //eventManger.ReservateEvent_TryReturnLoby(GeneratedEvents);
        eventManger.ReservateEvent_ChangeScene(GeneratedEvents, FEP_LOBY_SCENE);
        OnceTryReturnLoby = false;
    }
}

void GameOverScene::SetMatchStatisticInfo(std::wstring UserName, int UserRank,
    unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance,
    int TotalScore_Damage, int TotalScore_Heal,
    CHARACTER_TYPE PlayedCharacterType)
{
    UserInfo_UserName = UserName;
    UserInfo_UserRank = UserRank;

    GameMatchingResult_PlayedCharacterType = PlayedCharacterType;
    GameMatchingResult_CountKill = Count_Kill;
    GameMatchingResult_CountDeath = Count_Death;
    GameMatchingResult_CountAssistance = Count_Assistance;
    GameMatchingResult_TotalDamage = TotalScore_Damage;
    GameMatchingResult_TotalHeal = TotalScore_Heal;

    std::string ActivatedCharacterName;
    switch (GameMatchingResult_PlayedCharacterType)
    {
    case CHARACTER_TYPE::WARRIOR:   ActivatedCharacterName = "Warrior"; break;
    case CHARACTER_TYPE::BERSERKER: ActivatedCharacterName = "Berserker"; break;
    case CHARACTER_TYPE::ASSASSIN:  ActivatedCharacterName = "Assassin"; break;
    case CHARACTER_TYPE::PRIEST:    ActivatedCharacterName = "Priest"; break;
    case CHARACTER_TYPE::COUNT:
    case CHARACTER_TYPE::NON:
    default:
        throw std::invalid_argument("Unknown Character Type.");
        break;
    }

    for (auto& obj : m_CharacterObjects)
    {
        auto AnimInfo = obj->m_SkeletonInfo->m_AnimInfo.get();

        if (obj->m_Name.find(ActivatedCharacterName) != std::string::npos)
        {
            obj->RenderActivated = true;
            AnimInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            AnimInfo->AnimLoop (aiModelData::AnimActionType::Idle);
        }
        else
        {
            obj->RenderActivated = false;
            AnimInfo->AnimStop(aiModelData::AnimActionType::Idle);
            AnimInfo->AnimLoop(aiModelData::AnimActionType::Idle, false);
        }
    }
}
