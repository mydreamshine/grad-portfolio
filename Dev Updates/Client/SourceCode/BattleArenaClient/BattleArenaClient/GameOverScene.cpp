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
    for (auto& obj : m_CharacterObjects)
    {
        if (obj->m_Name == "Meshtint Free Knight")
        {
            auto transformInfo = obj->m_TransformInfo.get();
            float ConvertModelUnit = ModelFileUnit::meter;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
            transformInfo->SetWorldTransform(WorldScale, WorldRotationEuler, WorldPosition);
            transformInfo->SetLocalRotationEuler(LocalRotationEuler);

            auto skeletonInfo = obj->m_SkeletonInfo.get();
            auto animInfo = skeletonInfo->m_AnimInfo.get();
            animInfo->Init();
            animInfo->AutoApplyActionFromSkeleton(skeletonInfo->m_Skeleton);
            animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            animInfo->AnimLoop(aiModelData::AnimActionType::Idle);
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
        auto Ritem = Ritem_iter.second.get();
        if (Ritem_iter.first == "Meshtint Free Knight")
        {
            auto newObj = objManager.CreateCharacterObject(
                objCB_index++, skinnedCB_index++,
                m_AllObjects,
                m_CharacterObjects, m_MaxCharacterObject,
                m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);

            std::string objName = Ritem_iter.first;

            float ConvertModelUnit = ModelFileUnit::meter;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
            objManager.SetObjectComponent(newObj, objName, Ritem,
                ModelSkeletons["Meshtint Free Knight"].get(),
                nullptr, &LocalRotationEuler, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);

            auto skeletonInfo = newObj->m_SkeletonInfo.get();
            auto animInfo = newObj->m_SkeletonInfo->m_AnimInfo.get();
            animInfo->Init();
            animInfo->AutoApplyActionFromSkeleton(skeletonInfo->m_Skeleton);
            animInfo->AnimPlay(aiModelData::AnimActionType::Idle);
            animInfo->AnimLoop(aiModelData::AnimActionType::Idle);
        }
        if (Ritem_iter.first.find("UI") != std::string::npos)
        {
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, maxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            objManager.SetObjectComponent(newObj, objName, Ritem);

            if (objName.find("Background") != std::string::npos) continue;

            auto newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
            auto text_info = newLayoutTextObj->m_Textinfo.get();
            newLayoutTextObj->m_Name = "Text" + objName;
            text_info->m_FontName = L"¸¼Àº °íµñ";
            text_info->m_TextColor = DirectX::Colors::Blue;
            auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
            text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;
            text_info->TextBatchIndex = textBatch_index++;

            if (objName == "UI_Layout_GameOverInfo")        text_info->m_Text = L"Game Over Info\n\nTotal Damage,\nTotal Kill,\nTotal Death,\n etc.";
            else if (objName == "UI_Layout_GameOverResult") text_info->m_Text = L"Game Over";
            else if (objName == "UI_Layout_ReturnLoby")     text_info->m_Text = L"Retrun Loby";
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
    Object* GameOverInfoObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_GameOverInfo");

    auto& GameOverInfo = GameOverInfoObject->m_Textinfo->m_Text;

    GameOverInfo  = L"UserName: " + UserInfo_UserName + L'\n';
    GameOverInfo += L"UserRank: " + std::to_wstring(UserInfo_UserRank);
    GameOverInfo += L"Total Kill: " + std::to_wstring(GameMatchingResult_CountKill);
    GameOverInfo += L"Total Death: " + std::to_wstring(GameMatchingResult_CountDeath);
    GameOverInfo += L"Total Assistance: " + std::to_wstring(GameMatchingResult_CountAssistance);
    GameOverInfo += L"Total Damage: " + std::to_wstring(GameMatchingResult_TotalDamage);
    GameOverInfo += L"Total Heal: " + std::to_wstring(GameMatchingResult_TotalHeal);
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
}
