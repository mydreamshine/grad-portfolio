#include "stdafx.h"
#include "PlayGameScene.h"

#include <algorithm>

using namespace aiModelData;

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
    auto& AllRitems = *m_AllRitemsRef;

    for (auto& Player_iter : m_Players)
    {
        auto Player = Player_iter.second.get();
        Player->Init();
    }
    m_Players.clear();
    m_MainPlayer = nullptr;

    // BillboardObject나 EffectObject들은 아래 WorldObject이기도 하기에
    // WorldObject Instancing 비활성화 구문에 의해
    // Deactivate가 된다.
    // 고로, BillboardObject, EffectObject 리스트는 Object Deactivate 구문없이
    // 비워주기만 하면 된다.
    m_BillboardObjects.clear();
    m_EffectObjects.clear();

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


    // SpawnSkillObject()에서 HealAreaObject에 관한 FindDeactivateObject()를 호출하게 되는데
    // 이때 CheckRenderAct 파라미터에 의해 오브젝트의 RenderActivated를 기준으로
    // 비활성된 오브젝트를 색출하게 된다.
    // 그러나 HealAreaObject들은 HealAreaObject이기 이전에 WorldObject이기도 하기에
    // 위에 Instancing 비활성화 구문에 의해 RenderActivated가 true로 바뀌게 되어버린다.
    // RenderActivated를 기준으로 한 FindDeactivateObject()를 지키기 위해
    // 아래와 같이 HealAreaObject들에 한해서 RenderActivated를 false로 변경해준다.
    for (auto& obj : m_HealAreaObjects)
    {
        obj->Activated = true;
        obj->RenderActivated = false;
    }

    for (auto& obj : m_UILayOutObjects)
    {
        if (obj->m_Name.find("Instancing") != std::string::npos)
        {
            for (auto& attachedObj : obj->m_Childs)
                objManager.DeActivateObj(attachedObj);
            objManager.DeActivateObj(obj);
        }
    }

    for (auto& obj : m_TextObjects)
    {
        if (obj->m_Name.find("Instancing") != std::string::npos)
        {
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
    GameInfo_CountTeamScore = 0;

    TimeLimit_Sec = 0;
    TimeLimitIntervalTimeStack = 0.0f;

    DeActPoisonGasArea = { -3423, 4290, 4577, -3710 };
    PlayGameScene::InitPoisonFogObjects();

    while (KillLogList.empty() != true) KillLogList.pop();
    KillLogSlidingStart = false;
    KillLogSlidingInit = false;

    // Initialize Chatting Rel. 
    {
        ChattingLayerActivate = false;
        ChattingLayerSliding = false;

        ObjectManager ObjManager;
        Object* ChattingLogTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameChattingLog");
        Object* ChattingInputTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameChattingInput");
        Object* ChattingLogLayer = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameChattingLog");
        Object* ChattingLogPopUpButtonObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameChattingPopUpButton");
        Object* InputChatCaretObjet = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_InputBoxCaret");

        std::vector<RenderItem*> Ritems = { AllRitems["UI_Layout_GameChattingPopUpButton"].get() };
        ChattingLogPopUpButtonObject->m_RenderItems = Ritems;

        auto ChattingLogTextInfo = ChattingLogTextObject->m_Textinfo.get();
        auto ChattingInputTextInfo = ChattingInputTextObject->m_Textinfo.get();
        ChattingLogTextInfo->m_Text.clear();
        ChattingInputTextInfo->m_Text.clear();

        auto ChattingLogLayerTransform = ChattingLogLayer->m_TransformInfo.get();
        auto ChattingLogPopUpButtonTransform = ChattingLogPopUpButtonObject->m_TransformInfo.get();
        auto InputChatCaretTransform = InputChatCaretObjet->m_TransformInfo.get();
        auto& ChattingLogTextPosition = ChattingLogTextObject->m_Textinfo->m_TextPos;
        auto& ChattingLogInputTextPosition = ChattingInputTextObject->m_Textinfo->m_TextPos;

        auto& ChattingLogLayerBound = ChattingLogLayer->m_TransformInfo->m_Bound;

        const float SlidingDistance = -fabsf((-319.0f) - (-(m_width / 2.0f) - ChattingLogLayerBound.Extents.x * 2));

        XMFLOAT3 ChattingLogLayerPos = { -319.0f, 191.66f, 0.0f };
        ChattingLogLayerPos.x += SlidingDistance;
        XMFLOAT3 ChattingLogPopUpButtonPos = { -120.0f, 89.33f, 0.0f };
        ChattingLogPopUpButtonPos.x += SlidingDistance;
        XMFLOAT3 InputChatCaretPos = { 0.0f, 0.0f, 0.0f };
        InputChatCaretPos.x = ChattingLogLayerPos.x + 15.2f;

        ChattingLogLayerTransform->SetWorldPosition(ChattingLogLayerPos);
        ChattingLogLayerTransform->UpdateWorldTransform();
        ChattingLogPopUpButtonTransform->SetWorldPosition(ChattingLogPopUpButtonPos);
        ChattingLogPopUpButtonTransform->UpdateWorldTransform();
        ChattingLogTextPosition.x = ChattingLogLayerPos.x + 15.2f;
        ChattingLogTextPosition.x += m_width / 2.0f; // Coord Offset
        ChattingLogInputTextPosition.x = ChattingLogLayerPos.x + 15.2f;
        ChattingLogInputTextPosition.x += m_width / 2.0f; // Coord Offset
        InputChatCaretTransform->SetWorldPosition(InputChatCaretPos);
        InputChatCaretTransform->UpdateWorldTransform();

        ChattinglistBox.Init();
        ChattinglistBox.SetTextRenderObj(ChattingLogTextObject);
        ChattinglistBox.SetFont((*m_FontsRef)[L"맑은 고딕"].get());
        ChattinglistBox.SetLineTabCharacter(L':');
        ChattinglistBox.SetMaxSize(MaxChattingLineWidth, MaxChattingLineHeight);

        inputTextBox.Init();
        inputTextBox.SetTextRenderObj(ChattingInputTextObject);
        inputTextBox.SetCaretRenderObj(InputChatCaretObjet);
        inputTextBox.SetFont((*m_FontsRef)[L"맑은 고딕"].get());
        inputTextBox.SetStartInputForm(L"Unknown: ");
        inputTextBox.SetMaxSize(MaxChattingLineWidth, 24.0f);
        inputTextBox.InitTexts();
    }

    GameStart = true;
    GameOver = false;
    GameStartUI_ActTimeStack = 0.0f;
    GameOverUI_ActTimeStack = 0.0f;

    Object* TimeLimitObject = objManager.FindObjectName(m_TextObjects, "TextUI_Layout_GameTimeLimit");
    Object* GameStartInfoUIObject = objManager.FindObjectName(m_UILayOutObjects, "UI_Layout_FightTextLayer");
    Object* GameOverInfoUIObject = objManager.FindObjectName(m_UILayOutObjects, "UI_Layout_FinishTextLayer");
    TimeLimitObject->m_Textinfo->m_TextColor = DirectX::Colors::White;
    GameStartInfoUIObject->m_TransformInfo->m_TexAlpha = 0.0f;
    GameOverInfoUIObject->m_TransformInfo->m_TexAlpha = 0.0f;
    GameStartInfoUIObject->RenderActivated = false;
    GameOverInfoUIObject->RenderActivated = false;


    Object* CharacterInfoLayer_Skill_Icon_Fade = objManager.FindObjectName(m_UILayOutObjects, "UI_Layout_Skill_Icon_Fade");
    Object* CharacterInfoLayer_SkillCoolTime = objManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_SkillCoolTime");
    CharacterInfoLayer_Skill_Icon_Fade->m_TransformInfo->SetWorldScale({ 1.0f, 0.0f, 1.0f });
    CharacterInfoLayer_Skill_Icon_Fade->m_TransformInfo->UpdateWorldTransform();
    CharacterInfoLayer_SkillCoolTime->m_Textinfo->m_Text.clear();
}

void PlayGameScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt,
    std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    PlayGameScene::AnimateWorldObjectsTransform(gt, GeneratedEvents);
    PlayGameScene::AnimateEffectObjectsTransform(gt);
    PlayGameScene::AnimateCharacterRenderEffect(gt);
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt, GeneratedEvents);
    PlayGameScene::ProcessObjectCollision(gt, GeneratedEvents);
}

void PlayGameScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;
    m_MaxUILayoutObject = (UINT)Geometries["PlayGameSceneUIGeo"]->DrawArgs.size() + MAX_CHARACTER_OBJECT;

    bool Exist_Info_Layer_HP_Text = false;
    bool Exist_Character_Info_Layer = false;
    bool Exist_ChattingPopUpButton = false;

    Object* InputChatCaretObjet = nullptr;

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
            if (Ritem_name.find("InfoLayer") == std::string::npos
                && Ritem_name.find("HPBar") != std::string::npos) continue;

            if (Ritem_name.find("InfoLayer") != std::string::npos
                && Ritem_name.find("HPBar") == std::string::npos
                && Exist_Character_Info_Layer == true) continue;

            if (Ritem_name.find("PopUpButton") != std::string::npos
                && Exist_ChattingPopUpButton == true) continue;

            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, m_MaxUILayoutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            // UI 오브젝트 같은 경우에는 메쉬의 원점이 (0,0,0)이 아니라
            // 이미 버텍스 자체가 스크린좌표계 기준으로 지정되어 있기 때문에
            // UI 오브젝트의 Transform에 대해선 따로 지정하지 않아도 된다.
            std::string objName = Ritem_iter.first;
            std::vector<RenderItem*> Ritems = { Ritem };
            objManager.SetObjectComponent(newObj, objName, Ritems);

            XMFLOAT3 UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            XMFLOAT3 UI_LayoutExtents = Ritem->Geo->DrawArgs[objName].Bounds.Extents;

            int TextObjCreatNum = 1;

            if (Ritem_name.find("FightTextLayer") != std::string::npos
                || Ritem_name.find("FinishTextLayer") != std::string::npos)
            {
                continue;
            }
            if (Ritem_name.find("PopUpButton") != std::string::npos)
            {
                newObj->m_Name = "UI_Layout_GameChattingPopUpButton";
                newObj->m_TransformInfo->SetBound(Ritem->Geo->DrawArgs[objName].Bounds, TransformInfo::BoundPivot::Center);
                newObj->m_TransformInfo->SetWorldPosition({ -120.0f, 89.33f, 0.0f });
                newObj->m_TransformInfo->UpdateWorldTransform();
                UI_LayoutPos = newObj->m_TransformInfo->GetWorldPosition();
                Exist_ChattingPopUpButton = true;
                continue;
            }
            if (Ritem_name.find("InfoLayer_HPBar") != std::string::npos)
            {
                newObj->m_TransformInfo->SetBound(Ritem->Geo->DrawArgs[objName].Bounds, TransformInfo::BoundPivot::Center);
                newObj->m_TransformInfo->SetWorldPosition({ -UI_LayoutExtents.x, -200.167f, 0.0f });
                newObj->m_TransformInfo->UpdateWorldTransform();
                if (Exist_Info_Layer_HP_Text == true) continue;
            }
            if (Ritem_name.find("InfoLayer_HPBar") != std::string::npos) Exist_Info_Layer_HP_Text = true;
            else if (Ritem_name.find("GameChattingLog") != std::string::npos) TextObjCreatNum = 2;
            else if (Ritem_name.find("Skill_Icon_Fade") != std::string::npos) TextObjCreatNum = 2;
            else if (Ritem_name.find("Layout_Score") != std::string::npos) TextObjCreatNum = 2;
            else if (Ritem_name.find("InputBoxCaret") != std::wstring::npos) TextObjCreatNum = 0;
            else if (Ritem_name.find("InfoLayer") != std::string::npos)
            {
                newObj->m_Name = "CharacterInfoLayer";
                Exist_Character_Info_Layer = true;
                TextObjCreatNum = 3;
            }

            std::vector<Object*> additionalTextObjets;

            Object* newLayoutTextObj = nullptr;
            TextInfo* text_info = nullptr;

            for (int i = 0; i < TextObjCreatNum; ++i)
            {
                newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
                text_info = newLayoutTextObj->m_Textinfo.get();
                newLayoutTextObj->m_Name = "Text" + objName;
                text_info->m_FontName = L"맑은 고딕";
                text_info->m_TextColor = DirectX::Colors::Blue;
                text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
                text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;

                additionalTextObjets.push_back(newLayoutTextObj);
            }

            if (objName == "UI_Layout_GameTimeLimit")
            {
                text_info->m_Text = L"Time Limit";
                text_info->m_TextPos.x = 0.0f;
                text_info->m_TextPos.y = 210.0f;
                text_info->m_TextPos.x += m_width / 2.0f; // Coord Offset
                text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                text_info->m_TextColor = DirectX::Colors::White;
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::CENTER;
            }
            else if (objName == "UI_Layout_Score")
            {
                additionalTextObjets[0]->m_Name = "TextScoreLayer_KDA";
                additionalTextObjets[1]->m_Name = "TextScoreLayer_TeamGoalScore";
                additionalTextObjets[0]->m_Textinfo->m_Text = L"K:0  D:0  A:0";
                additionalTextObjets[1]->m_Textinfo->m_Text = L"0/" + std::to_wstring(MaxTeamScore);
                additionalTextObjets[1]->m_Textinfo->m_TextColor = DirectX::Colors::White;

                for (int i = 0; i < 2; ++i)
                {
                    text_info = additionalTextObjets[i]->m_Textinfo.get();
                    text_info->m_TextPos.y = UI_LayoutPos.y + UI_LayoutExtents.y - 10.7f;
                    text_info->m_TextPos.y -= i * 81.3f;
                    text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                }
            }
            else if (objName == "UI_Layout_KillLog")
            {
                newObj->m_TransformInfo->SetBound(Ritem->Geo->DrawArgs[objName].Bounds, TransformInfo::BoundPivot::Center);
                newObj->m_TransformInfo->SetWorldPosition({ -(m_width / 2.0f) - UI_LayoutExtents.x * 2.0f, m_height / 4.0f + UI_LayoutExtents.y, 0.0f });
                newObj->m_TransformInfo->UpdateWorldTransform();
                UI_LayoutPos = newObj->m_TransformInfo->GetWorldPosition();

                text_info->m_Text = L"Kill Log";
                text_info->m_FontName = L"맑은 고딕(16pt)";

                text_info->m_TextPos.x = UI_LayoutPos.x + UI_LayoutExtents.x;
                text_info->m_TextPos.y = UI_LayoutPos.y - UI_LayoutExtents.y;
                text_info->m_TextPos.x += m_width / 2.0f; // Coord Offset
                text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                text_info->m_TextColor = DirectX::Colors::Red;
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::CENTER;
            }
            else if (Ritem_name.find("GameChattingLog") != std::string::npos)
            {
                newObj->m_TransformInfo->SetBound(Ritem->Geo->DrawArgs[objName].Bounds, TransformInfo::BoundPivot::Center);
                newObj->m_TransformInfo->SetWorldPosition({ -319.0f, 191.66f, 0.0f });
                newObj->m_TransformInfo->UpdateWorldTransform();
                UI_LayoutPos = newObj->m_TransformInfo->GetWorldPosition();

                additionalTextObjets[0]->m_Name = "TextGameChattingInput";
                additionalTextObjets[1]->m_Name = "TextGameChattingLog";

                for (int i = 0; i < 2; ++i)
                {
                    text_info = additionalTextObjets[i]->m_Textinfo.get();
                    text_info->m_TextPos.x = UI_LayoutPos.x + 15.2f;
                    text_info->m_TextPos.y = UI_LayoutPos.y - UI_LayoutExtents.y * 2 + 30.7f;
                    text_info->m_TextPos.y += i * 30.3f;
                    text_info->m_TextPos.x += m_width / 2.0f; // Coord Offset
                    text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset

                    text_info->m_TextColor = XMVectorSet(254.0f / 255.0f, 216.0f / 255.0f, 109.0f / 255.0f, 1.0f);
                    text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                }
            }
            else if (Ritem_name.find("InputBoxCaret") != std::string::npos)
            {
                InputChatCaretObjet = newObj;
            }
            else if (objName.find("InfoLayer_HPBar") != std::string::npos && Exist_Info_Layer_HP_Text == true)
            {
                UI_LayoutPos = newObj->m_TransformInfo->GetWorldPosition();

                newLayoutTextObj->m_Name = "TextCharacterInfoLayer_HP";

                text_info->m_Text = L"100 / 100";
                text_info->m_TextPos.x = UI_LayoutPos.x + UI_LayoutExtents.x;
                text_info->m_TextPos.y = UI_LayoutPos.y - UI_LayoutExtents.y + 0.5f;
                text_info->m_TextPos.x += m_width / 2.0f; // Coord Offset
                text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                text_info->m_TextColor = XMVectorSet(59.0f / 255.0f, 56.0f / 255.0f, 56.0f / 255.0f, 1.0f);
                text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::CENTER;
            }
            else if (objName.find("InfoLayer") != std::string::npos && Exist_Character_Info_Layer == true)
            {
                additionalTextObjets[0]->m_Name = "TextCharacterInfoLayer_AttackDamage";
                additionalTextObjets[1]->m_Name = "TextCharacterInfoLayer_AttackSpeed";
                additionalTextObjets[2]->m_Name = "TextCharacterInfoLayer_MovingSpeed";
                additionalTextObjets[0]->m_Textinfo->m_Text = L"20";
                additionalTextObjets[1]->m_Textinfo->m_Text = L"1.2";
                additionalTextObjets[2]->m_Textinfo->m_Text = L"360";

                for (int i = 0; i < 3; ++i)
                {
                    text_info = additionalTextObjets[i]->m_Textinfo.get();
                    text_info->m_TextPos.x = 5.0f;
                    text_info->m_TextPos.y = -133.0f - i * 23.0f;
                    text_info->m_TextPos.x += m_width / 2.0f; // Coord Offset
                    text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                    text_info->m_TextColor = DirectX::Colors::Yellow;
                    text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                }
            }
            else if (Ritem_name.find("Skill_Icon_Fade") != std::string::npos)
            {
                newObj->m_TransformInfo->SetBound(Ritem->Geo->DrawArgs[objName].Bounds, TransformInfo::BoundPivot::Center);
                newObj->m_TransformInfo->SetWorldPosition({ 45.0f, -137.467f, 0.0f });
                newObj->m_TransformInfo->SetWorldScale({ 1.0f, 0.0f, 1.0f });
                newObj->m_TransformInfo->UpdateWorldTransform();
                UI_LayoutPos = newObj->m_TransformInfo->GetWorldPosition();

                additionalTextObjets[0]->m_Name = "TextCharacterInfoLayer_SkillName";
                additionalTextObjets[1]->m_Name = "TextCharacterInfoLayer_SkillCoolTime";
                additionalTextObjets[0]->m_Textinfo->m_Text = L"SkillName";
                additionalTextObjets[1]->m_Textinfo->m_Text = L"";

                for (int i = 0; i < 2; ++i)
                {
                    text_info = additionalTextObjets[i]->m_Textinfo.get();
                    text_info->m_TextPos.x = UI_LayoutPos.x + UI_LayoutExtents.x;
                    text_info->m_TextPos.y = UI_LayoutPos.y + 9.0f;
                    text_info->m_TextPos.x += m_width / 2.0f; // Coord Offset
                    text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                    if (i == 0) text_info->m_TextColor = DirectX::Colors::Yellow;
                    else
                    {
                        text_info->m_TextPos.y = UI_LayoutPos.y - UI_LayoutExtents.y;
                        text_info->m_TextPos.y = m_height / 2.0f - (text_info->m_TextPos.y); // Coord Offset
                        text_info->m_TextColor = XMVectorSet(9.0f / 255.0f, 12.0f / 255.0f, 7.0f / 255.0f, 1.0f);
                    }
                    text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::CENTER;
                }
            }
        }
        else if (Ritem_name.find("Effect") != std::string::npos)
        {
            continue;
        }
        else
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            if (Ritem_name.find("Grass") == std::string::npos)
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

                if (objName.find("Floor1") != std::string::npos)
                {
                    m_GroundObj = newObj;
                    auto& FloorBound = newObj->m_TransformInfo->m_Bound;
                    float width = FloorBound.Extents.x;
                    float depth = FloorBound.Extents.z;

                    m_SceneBounds.Center = FloorBound.Center;
                    // m_SceneBounds.Radius에 비례해서 ShadowMap이 맵핑되다보니
                    // ShadowMap의 해상도보다 m_SceneBounds.Radius가 크면
                    // BackBuffer에 맵핑되는 그림자의 해상도가 상대적으로 떨어지게 된다.
                    m_SceneBounds.Radius = (float)SIZE_ShadowMap * 2;
                }
                else if (objName.find("Grass") != std::string::npos)
                {
                    m_GrassObstacleObjects.push_back(newObj);
                    m_ObjRenderLayer[(int)RenderLayer::Transparent_depth].push_back(newObj);
                }
            }
        }
    }

    if (InputChatCaretObjet != nullptr)
    {
        Object* ChattingInputTextObject = objManager.FindObjectName(m_TextObjects, "TextGameChattingInput");

        auto ChattingInputTextInfo = ChattingInputTextObject->m_Textinfo.get();
        auto InputChatCaretTransform = InputChatCaretObjet->m_TransformInfo.get();
        auto Font = (*m_FontsRef)[ChattingInputTextInfo->m_FontName].get();

        XMFLOAT2 InputChatTextSize = Font->GetStringSize(ChattingInputTextInfo->m_Text);
        XMFLOAT2 InputChatTextPos = ChattingInputTextInfo->m_TextPos;
        XMFLOAT3 newInputChatCaretPos = { 0.0f, 0.0f, 0.0f };

        InputChatTextPos.x -= m_width / 2.0f;
        InputChatTextPos.y -= m_height / 2.0f;
        InputChatTextPos.y *= -1.0f;
        newInputChatCaretPos.x = InputChatTextPos.x + InputChatTextSize.x;
        newInputChatCaretPos.y = InputChatTextPos.y;

        InputChatCaretTransform->SetWorldPosition(newInputChatCaretPos);
        InputChatCaretTransform->UpdateWorldTransform();

        inputTextBox.Init();
        inputTextBox.SetTextRenderObj(ChattingInputTextObject);
        inputTextBox.SetCaretRenderObj(InputChatCaretObjet);
        inputTextBox.SetFont((*m_FontsRef)[L"맑은 고딕"].get());
        inputTextBox.SetStartInputForm(L"Unknown: ");
        inputTextBox.SetMaxSize(MaxChattingLineWidth, 24.0f);
    }

    Object* ChattingLogTextObject = objManager.FindObjectName(m_TextObjects, "TextGameChattingLog");
    ChattinglistBox.Init();
    ChattinglistBox.SetTextRenderObj(ChattingLogTextObject);
    ChattinglistBox.SetFont((*m_FontsRef)[L"맑은 고딕"].get());
    ChattinglistBox.SetLineTabCharacter(L':');
    ChattinglistBox.SetMaxSize(MaxChattingLineWidth, MaxChattingLineHeight);

    Object* GameStartInfoUIObject = objManager.FindObjectName(m_UILayOutObjects, "UI_Layout_FightTextLayer");
    Object* GameOverInfoUIObject = objManager.FindObjectName(m_UILayOutObjects, "UI_Layout_FinishTextLayer");
    GameStartInfoUIObject->m_TransformInfo->m_TexAlpha = 0.0f;
    GameOverInfoUIObject->m_TransformInfo->m_TexAlpha = 0.0f;


    // Depth enable Transparency Render Layer에 Heal Area Object를 미리 만들어준다
    for(UINT i = 0; i < m_MaxHealAreaObject; ++i)
    {
        auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
        m_ObjRenderLayer[(int)RenderLayer::Transparent_back].push_back(newObj);
        // Object Component(Transform, Ritems, ObjectName, etc.)는 Heal Area를 스폰할 때 지정해주고
        // 이 부분에선 RenderLayer만 지정해준다.
        // 단, Heal Area를 스폰할 때 FindObject를 WorldObjects가 아닌 HealAreaObjects에서 해준다.

        m_HealAreaObjects.push_back(newObj);
        // m_HealAreaObjects는 빌드할 때를 제외하고는 clear하지 않는다.
        // (각각의 오브젝트가 DeActivate 것과는 별개.)
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
            m_ObjRenderLayer[(int)RenderLayer::SkinnedTransparent].push_back(newObj);
        }

        const UINT nDeAcativateWorldObj = m_MaxWorldObject - (UINT)m_WorldObjects.size();
        for (UINT i = 0; i < nDeAcativateWorldObj; ++i)
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::Transparent].push_back(newObj);
        }

        const UINT nDeActivateUILayoutObj = m_MaxUILayoutObject - (UINT)m_UILayOutObjects.size();
        for (UINT i = 0; i < nDeActivateUILayoutObj; ++i)
        {
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, m_MaxUILayoutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);
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
        for (UINT i = m_MaxUILayoutObject - nDeActivateUILayoutObj; i < m_MaxUILayoutObject; ++i)
            m_UILayOutObjects[i]->Activated = false;
        for (UINT i = m_MaxTextObject - nDeActivateTextObj; i < m_MaxTextObject; ++i)
            m_TextObjects[i]->Activated = false;
    }

    // Render Deactivate HealAreaObjects
    {
        // Deactivate를 하면 FindDeactivateObject()에 의해 색출이 되기 때문에
        // RenderActivated만 비활성화 해준다.
        // (m_HealAreaObjects 중 비활성화된 오브젝트를 찾으려면
        //  FindDeactivateObject() 파라미터에 CheckRenderAct를 넣어주면
        //  오브젝트의 Activated가 아닌 RenderActivated를 기준으로 찾게 된다.
        for (UINT i = 0; i < m_MaxHealAreaObject; ++i)
            m_HealAreaObjects[i]->RenderActivated = false;
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
    Object* ChattingLogObject = ObjManager.FindObjectName(m_TextObjects, "TextGameChattingLog");
    Object* ChattingInputObject = ObjManager.FindObjectName(m_TextObjects, "TextGameChattingInput");
    Object* InputChatCaretObjet = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_InputBoxCaret");
    Object* TimeLimitObject = ObjManager.FindObjectName(m_TextObjects, "TextUI_Layout_GameTimeLimit");
    Object* KDA_ScoreObject = ObjManager.FindObjectName(m_TextObjects, "TextScoreLayer_KDA");
    Object* Team_ScoreObject = ObjManager.FindObjectName(m_TextObjects, "TextScoreLayer_TeamGoalScore");

    // Update KillLog Text
    {
        auto KillLogTextInfo = KillLogObject->m_Textinfo.get();
        auto& KillLogText = KillLogTextInfo->m_Text;
        XMFLOAT3 KillLogLayerScale = { 1.0f, 1.0f, 1.0f };

        if (KillLogList.empty() != true && KillLogSlidingStart == false)
        {
            KillLogSlidingStart = true;
            KillLogSlidingInit = true;
            KillLogText = KillLogList.front();
            KillLogList.pop();

            const float KillLogLayerDefaultWidth = 234.5f;
            const float KillLogTextRenderWidth = 204.5f;
            auto Font = (*m_FontsRef)[KillLogTextInfo->m_FontName].get();

            XMFLOAT2 KillLogRenderTextSize = Font->GetStringSize(KillLogText);

            if (KillLogRenderTextSize.x > KillLogTextRenderWidth)
                KillLogLayerScale.x += (KillLogRenderTextSize.x / 2.0f) / KillLogLayerDefaultWidth;

            Object* KillLogLayoutObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_KillLog");
            KillLogLayoutObject->m_TransformInfo->SetWorldScale(KillLogLayerScale);
            KillLogLayoutObject->m_TransformInfo->UpdateWorldTransform();
        }

        if (KillLogSlidingInit == true)
        {
            auto& KillLogTextPos = KillLogTextInfo->m_TextPos;

            Object* KillLogLayoutObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_KillLog");
            auto KillLogLayoutTransformInfo = KillLogLayoutObject->m_TransformInfo.get();
            auto KillLogLayoutBound = KillLogLayoutTransformInfo->m_Bound;

            XMFLOAT3 KillLogLayoutPos = { -(m_width / 2.0f) - KillLogLayoutBound.Extents.x * 2.0f, m_height / 4.0f + KillLogLayoutBound.Extents.y, 0.0f };
            KillLogLayoutObject->m_TransformInfo->SetWorldPosition(KillLogLayoutPos);
            KillLogLayoutObject->m_TransformInfo->SetWorldScale(KillLogLayerScale);
            KillLogLayoutObject->m_TransformInfo->UpdateWorldTransform();

            KillLogLayoutPos = KillLogLayoutTransformInfo->GetWorldPosition();

            KillLogTextPos.x = KillLogLayoutPos.x + KillLogLayoutBound.Extents.x;
            KillLogTextPos.y = KillLogLayoutPos.y - KillLogLayoutBound.Extents.y + 4.0f;
            KillLogTextPos.x += m_width / 2.0f; // Coord Offset
            KillLogTextPos.y = m_height / 2.0f - (KillLogTextPos.y); // Coord Offset

            KillLogSlidingInit = false;
        }

        if (KillLogSlidingStart == true)
        {
            XMFLOAT2 WndCenter = { m_width / 2.0f, m_height / 2.0f };

            auto FontInfo = (*m_FontsRef)[KillLogTextInfo->m_FontName].get();
            XMFLOAT2 KillLogTextSize = FontInfo->GetStringSize(KillLogText);
            auto& KillLogTextPos = KillLogTextInfo->m_TextPos;

            Object* KillLogLayoutObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_KillLog");
            auto KillLogLayoutTransformInfo = KillLogLayoutObject->m_TransformInfo.get();
            XMFLOAT3 KillLogLayoutPos = KillLogLayoutTransformInfo->GetWorldPosition();

            const float SlidingHalfTime = 0.303f;
            const float MaxSlidingHalfDistance = m_width / 2.0f;
            float KillLogSlidingSpeed = MaxSlidingHalfDistance * gt.GetTimeElapsed() / SlidingHalfTime;

            if (WndCenter.x - KillLogTextSize.x * 0.7f <= KillLogTextPos.x
                && KillLogTextPos.x <= WndCenter.x + KillLogTextSize.x * 0.23f)
                KillLogSlidingSpeed *= 0.1f;

            KillLogTextPos.x += KillLogSlidingSpeed;
            KillLogLayoutPos.x += KillLogSlidingSpeed;

            KillLogLayoutTransformInfo->SetWorldPosition(KillLogLayoutPos);
            KillLogLayoutTransformInfo->UpdateWorldTransform();

            auto KillLogLayoutBound = KillLogLayoutTransformInfo->m_Bound;
            float KillLogtLayoutLeft = KillLogLayoutPos.x - KillLogLayoutBound.Extents.x;
            if (KillLogtLayoutLeft >= WndCenter.x + m_width / 2.0f)
            {
                KillLogSlidingStart = false;
                KillLogText.clear();
            }
        }
    }

    // Update Character Info Layer
    {
        Object* CharacterInfoLayer_HPText = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_HP");
        Object* CharacterInfoLayer_AttackDamage = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_AttackDamage");
        Object* CharacterInfoLayer_AttackSpeed = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_AttackSpeed");
        Object* CharacterInfoLayer_MovingSpeed = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_MovingSpeed");
        Object* CharacterInfoLayer_SkillCoolTime = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_SkillCoolTime");

        Object* CharacterInfoLayer_HPBar_Dest = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_InfoLayer_HPBarDest");
        Object* CharacterInfoLayer_HPBar_Increase = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_InfoLayer_HPBarIncrease");
        Object* CharacterInfoLayer_Skill_Icon_Fade = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_Skill_Icon_Fade");

        if (m_MainPlayer != nullptr)
        {
            auto& HP_text = CharacterInfoLayer_HPText->m_Textinfo->m_Text;
            auto& AttackDamage_text = CharacterInfoLayer_AttackDamage->m_Textinfo->m_Text;
            auto& AttackSpeed_text = CharacterInfoLayer_AttackSpeed->m_Textinfo->m_Text;
            auto& MovingSpeed_text = CharacterInfoLayer_MovingSpeed->m_Textinfo->m_Text;
            auto& SkillCoolTime_text = CharacterInfoLayer_SkillCoolTime->m_Textinfo->m_Text;


            float CharacterCurrHP = (float)m_MainPlayer->m_CharacterObjRef->HP;
            float CharacterOldHP = (float)m_MainPlayer->m_oldHP;
            int CharacterMaxHP = 0;
            int AttackDamage = 0;
            float AttackSpeed = 0.0f;
            int MovingSpeed = 0;
            CHARACTER_TYPE CharacterType = m_MainPlayer->m_CharacterObjRef->CharacterType;
            switch (CharacterType)
            {
            case CHARACTER_TYPE::WARRIOR:
                CharacterMaxHP = 100;
                AttackDamage = 30;
                AttackSpeed = 1.0f;
                MovingSpeed = 360;
                break;
            case CHARACTER_TYPE::BERSERKER:
                CharacterMaxHP = 150;
                AttackDamage = 40;
                AttackSpeed = 0.6f;
                MovingSpeed = 300;
                break;
            case CHARACTER_TYPE::ASSASSIN:
                CharacterMaxHP = 100;
                AttackDamage = 30;
                AttackSpeed = 1.2f;
                MovingSpeed = 380;
                break;
            case CHARACTER_TYPE::PRIEST:
                CharacterMaxHP = 80;
                AttackDamage = 15;
                AttackSpeed = 1.0f;
                MovingSpeed = 360;
                break;
            }

            // Update Character Info Text(HP, AttackDamage, AttackSpeed, MovingSpeed, etc.)
            {
                if (m_MainPlayer->m_CurrUseSkill == SKILL_TYPE::FURY_ROAR)
                {
                    AttackSpeed *= 2.0f;
                    MovingSpeed *= 2;
                }

                HP_text = std::to_wstring((int)CharacterCurrHP) + L" / " + std::to_wstring(CharacterMaxHP);
                AttackDamage_text = std::to_wstring(AttackDamage);
                AttackSpeed_text = std::to_wstring(AttackSpeed);
                MovingSpeed_text = std::to_wstring(MovingSpeed);

                size_t FloatingPointAct = AttackSpeed_text.find(L'.') + 2;
                AttackSpeed_text.erase(FloatingPointAct, AttackSpeed_text.size() - (FloatingPointAct + 1));
            }


            // Update HP Bar
            {
                float HP_BarScale = CharacterCurrHP / (float)CharacterMaxHP;
                bool HP_Increase = (CharacterCurrHP - CharacterOldHP > 0.0f);
                float HP_IncreaseFactor = 6.0f * gt.GetTimeElapsed();

                std::vector<Object*> HP_BarObjRef = { CharacterInfoLayer_HPBar_Dest, CharacterInfoLayer_HPBar_Increase };
                for (auto& HP_bar_e : HP_BarObjRef)
                {
                    auto TransformInfo = HP_bar_e->m_TransformInfo.get();

                    // Set HP Bar Size
                    float CurrScale = TransformInfo->GetWorldScale().x;
                    if (HP_bar_e->m_Name.find("Dest") != std::string::npos)
                    {
                        float IncreaseFactor = (HP_Increase) ? HP_IncreaseFactor * 0.3f : HP_IncreaseFactor;
                        CurrScale += (HP_BarScale - CurrScale) * IncreaseFactor;
                        TransformInfo->SetWorldScale({ CurrScale, 1.0f, 1.0f });
                    }
                    else if (HP_bar_e->m_Name.find("Increase") != std::string::npos)
                    {
                        float IncreaseFactor = (HP_Increase) ? HP_IncreaseFactor : HP_IncreaseFactor * 0.3f;
                        CurrScale += (HP_BarScale - CurrScale) * IncreaseFactor;
                        TransformInfo->SetWorldScale({ CurrScale, 1.0f, 1.0f });
                    }

                    TransformInfo->UpdateWorldTransform();
                }
            }


            // Update Skill Cool Time
            {
                if (m_MainPlayer->m_CurrUseSkill != SKILL_TYPE::NON)
                {
                    float MaxCoolTime = 0.0f;
                    switch (CharacterType)
                    {
                    case CHARACTER_TYPE::WARRIOR:   MaxCoolTime = 5.0f; break;
                    case CHARACTER_TYPE::BERSERKER: MaxCoolTime = 11.0f; break;
                    case CHARACTER_TYPE::ASSASSIN:  MaxCoolTime = 12.0f; break;
                    case CHARACTER_TYPE::PRIEST:    MaxCoolTime = 13.0f; break;
                    }
                    MaxCoolTime -= 1.0f;

                    m_MainPlayer->m_SkillCoolTimeStack += gt.GetTimeElapsed();
                    SkillCoolTime_text = std::to_wstring(MaxCoolTime - m_MainPlayer->m_SkillCoolTimeStack);
                    size_t FloatingPointAct = SkillCoolTime_text.find(L'.') + 2;
                    SkillCoolTime_text.erase(FloatingPointAct, SkillCoolTime_text.size() - (FloatingPointAct + 1));

                    float ScaleFactor = MathHelper::Clamp((MaxCoolTime - m_MainPlayer->m_SkillCoolTimeStack) / MaxCoolTime, 0.0f, 1.0f);
                    XMFLOAT3 newSkill_Icon_Scale = { 1.0f, ScaleFactor, 1.0f };
                    CharacterInfoLayer_Skill_Icon_Fade->m_TransformInfo->SetWorldScale(newSkill_Icon_Scale);
                    CharacterInfoLayer_Skill_Icon_Fade->m_TransformInfo->UpdateWorldTransform();

                    if (m_MainPlayer->m_SkillCoolTimeStack >= MaxCoolTime)
                    {
                        m_MainPlayer->m_CurrUseSkill = SKILL_TYPE::NON;
                        m_MainPlayer->m_SkillCoolTimeStack = 0.0f;
                        SkillCoolTime_text.clear();
                    }
                }
            }
        }
    }

    // Update Input Box Caret
    inputTextBox.Update(gt, (float)m_width, (float)m_height);

    // Update TimeLimit Text
    {
        auto TimeLimitTextInfo = TimeLimitObject->m_Textinfo.get();
        auto& TimeLimitText = TimeLimitTextInfo->m_Text;

        TimeLimitIntervalTimeStack += gt.GetTimeElapsed();
        if (TimeLimitIntervalTimeStack >= 1.0f)
        {
            TimeLimitIntervalTimeStack = 0.0f;
            TimeLimit_Sec = (TimeLimit_Sec - 1 > 0) ? TimeLimit_Sec - 1 : 0;

            if (TimeLimit_Sec <= 10)
            {
                XMFLOAT3 Colors3f; XMStoreFloat3(&Colors3f, TimeLimitTextInfo->m_TextColor);
                if (Colors3f.y == 1.0f)
                    TimeLimitTextInfo->m_TextColor = DirectX::Colors::Red;
                else
                    TimeLimitTextInfo->m_TextColor = DirectX::Colors::White;
            }
        }

        int minute = TimeLimit_Sec / 60;
        int sec = TimeLimit_Sec % 60;
        TimeLimitText.clear();

        if (minute < 10) TimeLimitText += L"0";
        TimeLimitText += std::to_wstring(minute) + L":";
        if (sec < 10) TimeLimitText += L"0";
        TimeLimitText += std::to_wstring(sec);
    }

    // Update Score Text
    {
        auto& KDA_ScoreText = KDA_ScoreObject->m_Textinfo->m_Text;
        auto& Team_ScoreText = Team_ScoreObject->m_Textinfo->m_Text;
        KDA_ScoreText = L"K:" + std::to_wstring(GameInfo_CountKill) + L"  ";
        KDA_ScoreText += L"D:" + std::to_wstring(GameInfo_CountDeath) + L"  ";
        KDA_ScoreText += L"A:" + std::to_wstring(GameInfo_CountAssistance);

        Team_ScoreText = std::to_wstring(GameInfo_CountTeamScore) + L"/";
        Team_ScoreText += std::to_wstring(MaxTeamScore);
    }

    // Update GameStart Text UI
    if (GameStart == true)
    {
        Object* GameStartInfoUIObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_FightTextLayer");

        if (GameStartUI_ActTimeStack >= GameStartUI_ActTimeLimit)
        {
            GameStart = false;
            GameStartInfoUIObject->RenderActivated = false;
        }
        else
        {
            GameStartInfoUIObject->RenderActivated = true;
            GameStartUI_ActTimeStack += gt.GetTimeElapsed();

            if (GameStartUI_ActTimeStack <= 0.6f)
                GameStartInfoUIObject->m_TransformInfo->m_TexAlpha = GameStartUI_ActTimeStack / 0.6f;
            else if (0.6f <= GameStartUI_ActTimeStack && GameStartUI_ActTimeStack <= 1.2f)
                GameStartInfoUIObject->m_TransformInfo->m_TexAlpha = 1.0f;
            else if (1.2f <= GameStartUI_ActTimeStack && GameStartUI_ActTimeStack <= GameStartUI_ActTimeLimit)
                GameStartInfoUIObject->m_TransformInfo->m_TexAlpha = (0.6f - (GameStartUI_ActTimeStack - 1.2f)) / (GameStartUI_ActTimeLimit - 1.2f);
        }
    }

    // Update GameOver Text UI
    if (GameOver == true)
    {
        Object* GameOverInfoUIObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_FinishTextLayer");
        GameOverInfoUIObject->m_TransformInfo->m_TexAlpha = 0.0f;

        if (GameOverUI_ActTimeStack >= GameOverUI_ActTimeLimit)
        {
            EventManager eventManager;
            eventManager.ReservateEvent_ChangeScene(GeneratedEvents, FEP_GAMEOVER_SCENE);
            GameOverInfoUIObject->RenderActivated = false;
        }
        else
        {
            GameOverInfoUIObject->RenderActivated = true;
            GameOverUI_ActTimeStack += gt.GetTimeElapsed();

            if (GameOverUI_ActTimeStack <= 1.0f)
                GameOverInfoUIObject->m_TransformInfo->m_TexAlpha = GameOverUI_ActTimeStack / 1.0f;
            else if (1.0f <= GameOverUI_ActTimeStack && GameOverUI_ActTimeStack <= 2.0f)
                GameOverInfoUIObject->m_TransformInfo->m_TexAlpha = 1.0f;
            else if (2.0f <= GameOverUI_ActTimeStack && GameOverUI_ActTimeStack <= GameOverUI_ActTimeLimit)
                GameOverInfoUIObject->m_TransformInfo->m_TexAlpha = (1.0f - (GameOverUI_ActTimeStack - 2.0f)) / (GameOverUI_ActTimeLimit - 2.0f);
        }
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
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
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

    PlayGameScene::RotateBillboardObjects(&m_MainCamera, m_BillboardObjects);
    PlayGameScene::UpdateUITransformAs(gt, &m_MainCamera, m_Players);
}

void PlayGameScene::AnimateWorldObjectsTransform(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    for (auto& obj : m_WorldObjects)
    {
        if (obj->ProcessSelfDeActivate(gt) != true)
            obj->m_TransformInfo->Animate(gt);

        if (obj->m_TransformInfo->WorldPosIsInterpolation == true)
            obj->m_TransformInfo->InterpolateTransformWorldPosition(gt);

        if (obj->m_TransformInfo->SetedSpritedTexture == true)
            obj->m_TransformInfo->UpdateClipedTexture(obj->DeActivatedTime - obj->DeActivatedDecrease, obj->DeActivatedTime, gt);
    }
}

void PlayGameScene::AnimateEffectObjectsTransform(CTimer& gt)
{
    for (auto& obj : m_EffectObjects)
    {
        if (obj->m_Name.find("HolyCross") != std::string::npos)
        {
            XMFLOAT3 PivotPosition = obj->m_Parent->m_TransformInfo->GetWorldPosition();
            auto Transforminfo = obj->m_TransformInfo.get();
            XMFLOAT3 WorldPosition = Transforminfo->GetWorldPosition();

            const float HealAreaRad = 1000.0f;
            float MaxSpawnHeight = HealAreaRad * 0.7f - 50.0f;
            float CurrSpawnHeight = WorldPosition.y - (PivotPosition.y + 50.0f);

            float texAlpha = 1.0f - CurrSpawnHeight / MaxSpawnHeight;
            float newScaleScala = MathHelper::Clamp(CurrSpawnHeight + MaxSpawnHeight * 0.5f, 0.0f, MaxSpawnHeight) / MaxSpawnHeight;

            const float Move_speed = MaxSpawnHeight * gt.GetTimeElapsed() / 3.0f;
            XMFLOAT3 newWorldPosition = WorldPosition;
            newWorldPosition.y += Move_speed;

            if (newWorldPosition.y >= PivotPosition.y + HealAreaRad * 0.7f)
            {
                newWorldPosition = {
                    MathHelper::RandF(PivotPosition.x - HealAreaRad * 0.6f, PivotPosition.x + HealAreaRad * 0.6f),
                    PivotPosition.y + 50.0f,
                MathHelper::RandF(PivotPosition.z - HealAreaRad * 0.6f, PivotPosition.z + HealAreaRad * 0.6f) };
            }

            Transforminfo->SetWorldScale({ newScaleScala, newScaleScala, newScaleScala });
            Transforminfo->SetWorldPosition(newWorldPosition);
            Transforminfo->m_TexAlpha = texAlpha;
            Transforminfo->UpdateWorldTransform();
        }
        else if (obj->m_Name.find("FuryRoar") != std::string::npos)
        {
            auto Transforminfo = obj->m_TransformInfo.get();
            const float MaxScaleScala = 4.0f;
            const float MaxScaleTiming = 0.5f;
            const float ScaleExtSpeed = MaxScaleScala * gt.GetTimeElapsed() / MaxScaleTiming;
            const float CompleteDisappearTiming = 1.8f;
            const float AlphaZeroSpeed = gt.GetTimeElapsed() / CompleteDisappearTiming;

            float CurrScaleScala = Transforminfo->GetWorldScale().x;
            float newScaleScala = MathHelper::Clamp(CurrScaleScala + ScaleExtSpeed, 0.0f, MaxScaleScala);

            float CurrTexAlpha = Transforminfo->m_TexAlpha;
            float texAlpha = MathHelper::Clamp(CurrTexAlpha - AlphaZeroSpeed, 0.0f, 1.0f);

            Transforminfo->SetWorldScale({ newScaleScala, newScaleScala, newScaleScala });
            Transforminfo->m_TexAlpha = texAlpha;
            if (texAlpha <= 0.07f)
                obj->RenderActivated = false;
        }
        else if (obj->m_Name.find("StealthFog") != std::string::npos)
        {
            auto Transforminfo = obj->m_TransformInfo.get();

            const float MaxScaleScala = 2.5f;
            const float MaxScaleTiming = 2.1f;
            const float ScaleExtSpeed = MaxScaleScala * gt.GetTimeElapsed() / MaxScaleTiming;

            float CurrScaleScala = Transforminfo->GetWorldScale().x;
            float newScaleScala = MathHelper::Clamp(CurrScaleScala + ScaleExtSpeed, 0.0f, MaxScaleScala);

            float texAlpha = 1.0f;
            if (newScaleScala >= MaxScaleScala * 0.5f)
                texAlpha = MathHelper::Clamp(1.0f - (newScaleScala - MaxScaleScala * 0.5f) / (MaxScaleScala * 0.5f), 0.0f, 1.0f);

            Transforminfo->SetWorldScale({ newScaleScala, newScaleScala, newScaleScala });
            Transforminfo->m_TexAlpha = texAlpha;
            if (texAlpha <= 0.1f)
                obj->RenderActivated = false;
        }
        else if (obj->m_Name.find("PoisonFog") != std::string::npos)
        {
            if (obj->RenderActivated == false) continue;

            auto Transforminfo = obj->m_TransformInfo.get();

            const float DefaultPoisonGasSize = 200.0f;
            const float BoundaryPoisonGasSize = 1000.0f;
            const float InnerPoisonGasSize = 400.0f;

            float MaxScaleScala = InnerPoisonGasSize / DefaultPoisonGasSize;
            if (obj->m_Name.find("Boundary") != std::string::npos)
                MaxScaleScala = BoundaryPoisonGasSize / DefaultPoisonGasSize;
            const float MaxScaleTiming = 0.8f;
            float ScaleExtIncreaseSign = obj->DeActivatedDecrease; // Scale Increase Sign 대용
            const float ScaleExtSpeed = ScaleExtIncreaseSign * MaxScaleScala * gt.GetTimeElapsed() / MaxScaleTiming;

            float CurrScaleScala = Transforminfo->GetWorldScale().x;
            float newScaleScala = MathHelper::Clamp(CurrScaleScala + ScaleExtSpeed, 0.0f, MaxScaleScala);

            float texAlpha = MathHelper::Clamp(newScaleScala / MaxScaleScala, 0.0f, 1.0f);

            Transforminfo->SetWorldScale({ newScaleScala, newScaleScala, newScaleScala });
            Transforminfo->m_TexAlpha = texAlpha;

            if (newScaleScala >= MaxScaleScala)
                obj->DeActivatedDecrease = -1.0f; // Scale Increase Sign 대용
            else if (newScaleScala <= 0.0f)
                obj->DeActivatedDecrease = 1.0f; // Scale Increase Sign 대용
        }
    }
}

void PlayGameScene::AnimateCharacterRenderEffect(CTimer& gt)
{
    for (auto& player_iter : m_Players)
    {
        auto player = player_iter.second.get();
        player->UpdateRenderEffect(gt);
    }
}

void PlayGameScene::UpdateUITransformAs(CTimer& gt, Camera* MainCamera, std::unordered_map<int, std::unique_ptr<Player>>& Players)
{
    CD3DX12_VIEWPORT ViewPort((float)m_ClientRect.left, (float)m_ClientRect.top, (float)m_ClientRect.right, (float)m_ClientRect.bottom);

    for (auto& Player_iter : Players)
    {
        auto player = Player_iter.second.get();
        if (m_MainPlayer != nullptr)
        {
            if(player != m_MainPlayer)
            {
                ObjectManager objManager;
                Object* CharacterInfoLayer = objManager.FindObjectName(m_UILayOutObjects, "CharacterInfoLayer");
                auto& MainPlayerCharacterInfoLayerBound = CharacterInfoLayer->m_TransformInfo->m_Bound;
                auto& PlayerHP_BarBound = player->m_HP_BarObjRef[0]->m_TransformInfo->m_Bound;
                RECT MainPlayerCharacterInfoLayerRect =
                {
                    (LONG)(MainPlayerCharacterInfoLayerBound.Center.x - MainPlayerCharacterInfoLayerBound.Extents.x), // left
                    (LONG)(MainPlayerCharacterInfoLayerBound.Center.y + MainPlayerCharacterInfoLayerBound.Extents.y), // top
                    (LONG)(MainPlayerCharacterInfoLayerBound.Center.x + MainPlayerCharacterInfoLayerBound.Extents.x), // right
                    (LONG)(MainPlayerCharacterInfoLayerBound.Center.y - MainPlayerCharacterInfoLayerBound.Extents.y), // bottom
                };
                RECT PlayerHP_BarRect =
                {
                    (LONG)(PlayerHP_BarBound.Center.x - PlayerHP_BarBound.Extents.x), // left
                    (LONG)(PlayerHP_BarBound.Center.y + PlayerHP_BarBound.Extents.y), // top
                    (LONG)(PlayerHP_BarBound.Center.x + PlayerHP_BarBound.Extents.x), // right
                    (LONG)(PlayerHP_BarBound.Center.y - PlayerHP_BarBound.Extents.y), // bottom
                };
                if (IntersectRect_(MainPlayerCharacterInfoLayerRect, PlayerHP_BarRect) == true)
                    player->SetHP_BarActivate(false);
                else
                {
                    if(player->m_CharacterObjRef->PlayerState != PLAYER_STATE::ACT_STEALTH)
                        player->SetHP_BarActivate(true);
                }
            }
        }
        player->UpdateUITransform(MainCamera, ViewPort, gt);
    }
}

void PlayGameScene::RotateBillboardObjects(Camera* MainCamera, std::vector<Object*>& Objects)
{
    for (auto& obj : Objects)
    {
        auto TransformInfo = obj->m_TransformInfo.get();
        XMFLOAT3 WorldPos = TransformInfo->GetWorldPosition();
        XMFLOAT3 WorldScale = TransformInfo->GetWorldScale();
        XMMATRIX VIEW = MainCamera->GetView();
        VIEW.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMMATRIX INV_VIEW = XMMatrixInverse(nullptr, VIEW);
        XMFLOAT4X4 View; XMStoreFloat4x4(&View, INV_VIEW);
        TransformInfo->SetWorldTransform(View);
        TransformInfo->SetWorldPosition(WorldPos);
        TransformInfo->SetWorldScale(WorldScale);
    }
}

void PlayGameScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (GameOver == true) return;

    bool UI_Click = false;

    // Process Chat
    {
        ObjectManager ObjManager;
        Object* CharacterInfoLayer = ObjManager.FindObjectName(m_UILayOutObjects, "CharacterInfoLayer");
        Object* TeamGoalScoreLayer = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_Score");

        Object* ChattingLogLayer = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameChattingLog");
        Object* ChattingLogPopUpButtonObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameChattingPopUpButton");
        Object* InputChatCaretObjet = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_InputBoxCaret");
        Object* ChattingLogTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameChattingLog");
        Object* ChattingLogInputTextObject = ObjManager.FindObjectName(m_TextObjects, "TextGameChattingInput");

        auto& CharacterInfoLayerBound = CharacterInfoLayer->m_TransformInfo->m_Bound;
        auto& TeamGoalScoreLayerBound = TeamGoalScoreLayer->m_TransformInfo->m_Bound;
        auto& ChattingLogLayerBound = ChattingLogLayer->m_TransformInfo->m_Bound;
        auto& ChattingPopUpButtonBound = ChattingLogPopUpButtonObject->m_TransformInfo->m_Bound;

        if (key_state[VK_LBUTTON] == true) // Mouse Left Button
        {
            RECT CharacterInfoLayerAreainScreen =
            {
                (LONG)(CharacterInfoLayerBound.Center.x - CharacterInfoLayerBound.Extents.x), // left
                (LONG)(CharacterInfoLayerBound.Center.y + CharacterInfoLayerBound.Extents.y), // top
                (LONG)(CharacterInfoLayerBound.Center.x + CharacterInfoLayerBound.Extents.x), // right
                (LONG)(CharacterInfoLayerBound.Center.y - CharacterInfoLayerBound.Extents.y), // bottom
            };

            RECT TeamGoalScoreLayerAreainScreen =
            {
                (LONG)(TeamGoalScoreLayerBound.Center.x - TeamGoalScoreLayerBound.Extents.x), // left
                (LONG)(TeamGoalScoreLayerBound.Center.y + TeamGoalScoreLayerBound.Extents.y), // top
                (LONG)(TeamGoalScoreLayerBound.Center.x + TeamGoalScoreLayerBound.Extents.x), // right
                (LONG)(TeamGoalScoreLayerBound.Center.y - TeamGoalScoreLayerBound.Extents.y), // bottom
            };

            RECT ChattingLogLayerAreainScreen =
            {
                (LONG)(ChattingLogLayerBound.Center.x - ChattingLogLayerBound.Extents.x), // left
                (LONG)(ChattingLogLayerBound.Center.y + ChattingLogLayerBound.Extents.y), // top
                (LONG)(ChattingLogLayerBound.Center.x + ChattingLogLayerBound.Extents.x), // right
                (LONG)(ChattingLogLayerBound.Center.y - ChattingLogLayerBound.Extents.y), // bottom
            };

            RECT ChattingPopUpButtonAreainScreen =
            {
                (LONG)(ChattingPopUpButtonBound.Center.x - ChattingPopUpButtonBound.Extents.x), // left
                (LONG)(ChattingPopUpButtonBound.Center.y + ChattingPopUpButtonBound.Extents.y), // top
                (LONG)(ChattingPopUpButtonBound.Center.x + ChattingPopUpButtonBound.Extents.x), // right
                (LONG)(ChattingPopUpButtonBound.Center.y - ChattingPopUpButtonBound.Extents.y), // bottom
            };

            float CursorPos_x = oldCursorPos.x - m_width * 0.5f;
            float CursorPos_y = -(oldCursorPos.y - m_height * 0.5f);

            if (PointInRect(CursorPos_x, CursorPos_y, CharacterInfoLayerAreainScreen) == true)
                UI_Click = true;
            else if (PointInRect(CursorPos_x, CursorPos_y, TeamGoalScoreLayerAreainScreen) == true)
                UI_Click = true;
            else if (PointInRect(CursorPos_x, CursorPos_y, ChattingLogLayerAreainScreen) == true)
                UI_Click = true;

            // Process Chatting Pop-Up
            if (PointInRect(CursorPos_x, CursorPos_y, ChattingPopUpButtonAreainScreen) == true)
            {
                UI_Click = true;

                if (ChattingLayerSliding == false)
                {
                    ChattingLayerActivate = !ChattingLayerActivate;
                    if (ChattingLayerActivate == true)
                    {
                        auto& AllRitems = *m_AllRitemsRef;
                        std::vector<RenderItem*> Ritems = { AllRitems["UI_Layout_GameChattingPopUpButton"].get() };
                        ChattingLogPopUpButtonObject->m_RenderItems = Ritems;

                        if (inputTextBox.IsEmpty() == true)
                        {
                            std::wstring PlayerNameInputForm;
                            if (m_MainPlayer != nullptr)
                                PlayerNameInputForm = m_MainPlayer->m_Name + L": ";
                            else
                                PlayerNameInputForm = L"Unknown: ";

                            inputTextBox.SetStartInputForm(PlayerNameInputForm);
                            inputTextBox.InitTexts();
                        }
                    }
                    inputTextBox.SetActivate(ChattingLayerActivate);

                    ChattingLayerSliding = true;
                }
            }
        }

        if (key_state[VK_RETURN] == true) // Enter Key
        {
            if (ChattingLayerSliding == false)
            {
                auto& AllRitems = *m_AllRitemsRef;
                std::vector<RenderItem*> Ritems = { AllRitems["UI_Layout_GameChattingPopUpButton"].get() };
                ChattingLogPopUpButtonObject->m_RenderItems = Ritems;
                auto ChattingLogTextInfo = ChattingLogInputTextObject->m_Textinfo.get();
                auto ChattingInputTextInfo = ChattingLogInputTextObject->m_Textinfo.get();
                auto& ChattingLogRenderText = ChattingLogTextInfo->m_Text;
                auto& ChattingInputRenderText = ChattingInputTextInfo->m_Text;

                std::wstring PlayerNameInputForm;
                if (m_MainPlayer != nullptr)
                    PlayerNameInputForm = m_MainPlayer->m_Name + L": ";
                else
                    PlayerNameInputForm = L"Unknown: ";

                std::wstring FullinputTexts = inputTextBox.GetFullinputTexts();

                if (ChattingLayerActivate == false)
                {
                    if (inputTextBox.IsEmpty() == true)
                    {
                        std::wstring PlayerNameInputForm;
                        if (m_MainPlayer != nullptr)
                            PlayerNameInputForm = m_MainPlayer->m_Name + L": ";
                        else
                            PlayerNameInputForm = L"Unknown: ";

                        inputTextBox.SetStartInputForm(PlayerNameInputForm);
                        inputTextBox.InitTexts();
                    }

                    inputTextBox.SetActivate(true);

                    ChattingLayerActivate = true;
                    ChattingLayerSliding = true;
                }

                if (ChattingLayerSliding == false && inputTextBox.IsEmpty() == false)
                {
                    EventManager eventManager;
                    eventManager.ReservateEvent_SendChatLog(GeneratedEvents, FEP_PLAYGMAE_SCENE, FullinputTexts);

                    // ChattingList Message Add Test
                    //{
                    //    ChattinglistBox.AddMessage(FullinputTexts);
                    //    if (ChattinglistBox.IsSizeChanged() == true)
                    //    {
                    //        auto ChattingLogLayerTransform = ChattingLogLayer->m_TransformInfo.get();
                    //        auto ChattingLogLayerPos = ChattingLogLayerTransform->GetWorldPosition();
                    //        auto ChattingLogLayerExtents = ChattingLogLayerTransform->m_Bound.Extents;
                    //        auto ChattingLogRenderTextSize = ChattinglistBox.GetSize();

                    //        XMFLOAT2 newChattingListBoxPos = ChattinglistBox.GetPosition();
                    //        newChattingListBoxPos.y = ChattingLogLayerPos.y - ChattingLogLayerExtents.y * 2 + 30.7f;
                    //        newChattingListBoxPos.y += 26.8f;
                    //        newChattingListBoxPos.y += ChattingLogRenderTextSize.y * 0.5f;
                    //        newChattingListBoxPos.y = m_height / 2.0f - (newChattingListBoxPos.y); // Coord Offset
                    //        ChattinglistBox.SetPosition(newChattingListBoxPos);
                    //    }
                    //}

                    std::wstring PlayerNameInputForm;
                    if (m_MainPlayer != nullptr)
                        PlayerNameInputForm = m_MainPlayer->m_Name + L": ";
                    else
                        PlayerNameInputForm = L"Unknown: ";

                    inputTextBox.SetStartInputForm(PlayerNameInputForm);
                    inputTextBox.InitTexts();
                }
            }
        }

        ChattinglistBox.ProcessInput(key_state, oldCursorPos, gt, GeneratedEvents);

        if (ChattingLayerActivate == true)
            inputTextBox.ProcessInput(key_state, oldCursorPos, gt, GeneratedEvents);

        if (ChattingLayerSliding == true)
        {
            auto ChattingLogLayerTransform = ChattingLogLayer->m_TransformInfo.get();
            auto ChattingLogPopUpButtonTransform = ChattingLogPopUpButtonObject->m_TransformInfo.get();
            auto& ChattingLogTextPosition = ChattingLogTextObject->m_Textinfo->m_TextPos;
            auto& ChattingLogInputTextPosition = ChattingLogInputTextObject->m_Textinfo->m_TextPos;

            const float SlidingDistance = fabsf((-319.0f) - (-(m_width / 2.0f) - ChattingLogLayerBound.Extents.x * 2));
            float SlidingVelocity = 0.0f;
            float ChattingLayer_DestPos_x = 0.0f;
            if (ChattingLayerActivate == true)
            {
                ChattingLayer_DestPos_x = -319.0f;
                SlidingVelocity = SlidingDistance * gt.GetTimeElapsed() / ChattingLayerSlidingActionEndTime;
            }
            else
            {
                ChattingLayer_DestPos_x = -(m_width / 2.0f) - ChattingLogLayerBound.Extents.x * 2;
                SlidingVelocity = -SlidingDistance * gt.GetTimeElapsed() / ChattingLayerSlidingActionEndTime;
            }

            XMFLOAT3 ChattingLogLayerPos = ChattingLogLayerTransform->GetWorldPosition();
            ChattingLogLayerPos.x += SlidingVelocity;
            XMFLOAT3 ChattingLogPopUpButtonPos = ChattingLogPopUpButtonTransform->GetWorldPosition();
            ChattingLogPopUpButtonPos.x += SlidingVelocity;

            ChattingLogLayerTransform->SetWorldPosition(ChattingLogLayerPos);
            ChattingLogLayerTransform->UpdateWorldTransform();
            ChattingLogPopUpButtonTransform->SetWorldPosition(ChattingLogPopUpButtonPos);
            ChattingLogPopUpButtonTransform->UpdateWorldTransform();
            ChattingLogTextPosition.x += SlidingVelocity;
            ChattingLogInputTextPosition.x += SlidingVelocity;

            if ((ChattingLayerActivate == true && ChattingLogLayerPos.x >= ChattingLayer_DestPos_x)
                || (ChattingLayerActivate == false && ChattingLogLayerPos.x <= ChattingLayer_DestPos_x))
            {
                ChattingLayerSliding = false;

                float offset = -(ChattingLogLayerPos.x - ChattingLayer_DestPos_x);
                ChattingLogLayerPos.x += offset;
                ChattingLogPopUpButtonPos.x += offset;

                ChattingLogLayerTransform->SetWorldPosition(ChattingLogLayerPos);
                ChattingLogLayerTransform->UpdateWorldTransform();
                ChattingLogPopUpButtonTransform->SetWorldPosition(ChattingLogPopUpButtonPos);
                ChattingLogPopUpButtonTransform->UpdateWorldTransform();

                ChattingLogTextPosition.x += offset;
                ChattingLogInputTextPosition.x += offset;
            }

            ChattingLogLayerPos = ChattingLogLayerTransform->GetWorldPosition();
            auto ChattingLogLayerExtents = ChattingLogLayerTransform->m_Bound.Extents;
            auto ChattingLogRenderTextSize = ChattinglistBox.GetSize();

            XMFLOAT2 newChattingListBoxPos = ChattinglistBox.GetPosition();
            newChattingListBoxPos.y = ChattingLogLayerPos.y - ChattingLogLayerExtents.y * 2 + 30.7f;
            newChattingListBoxPos.y += 26.8f;
            newChattingListBoxPos.y += ChattingLogRenderTextSize.y * 0.5f;
            newChattingListBoxPos.y = m_height / 2.0f - (newChattingListBoxPos.y); // Coord Offset
            ChattinglistBox.SetPosition(newChattingListBoxPos);
        }
    }

    if (m_MainPlayer != nullptr && UI_Click == false && ChattingLayerActivate == false)
    {
        CD3DX12_VIEWPORT ViewPort((float)m_ClientRect.left, (float)m_ClientRect.top, (float)m_ClientRect.right, (float)m_ClientRect.bottom);
        m_MainPlayer->ProcessInput(key_state, oldCursorPos, ViewPort, gt, m_GroundObj, GeneratedEvents);
    }
}

void PlayGameScene::ProcessObjectCollision(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    if (m_MainPlayer == nullptr) return;
    Player* TeamPlayer = nullptr;

    for (auto& player_iter : m_Players)
    {
        auto player = player_iter.second.get();
        if (player == m_MainPlayer) continue;
        if (m_MainPlayer->m_CharacterObjRef->Propensity == player->m_CharacterObjRef->Propensity)
        {
            TeamPlayer = player;
            break;
        }
    }

    auto MainPlayerCharacterBound = m_MainPlayer->m_CharacterObjRef->m_TransformInfo->m_Bound;
    MainPlayerCharacterBound.Extents.x += 200.0f;
    MainPlayerCharacterBound.Extents.z += 200.0f;

    BoundingBox TeamPlayerCharacterBound;
    if (TeamPlayer != nullptr)
    {
        TeamPlayerCharacterBound = (TeamPlayer->m_CharacterObjRef->m_TransformInfo->m_Bound);
        TeamPlayerCharacterBound.Extents.x += 200.0f;
        TeamPlayerCharacterBound.Extents.z += 200.0f;
    }

    float newTexAlpha = 1.0f;
    for (auto obj : m_GrassObstacleObjects)
    {
        auto GrassObstacleTransformInfo = obj->m_TransformInfo.get();
        auto& GrassObstacleBound = GrassObstacleTransformInfo->m_Bound;

        newTexAlpha = 1.0f;
        if (MainPlayerCharacterBound.Intersects(GrassObstacleBound) == true)
            newTexAlpha = 0.3f;
        else if (TeamPlayer != nullptr)
        {
            if (TeamPlayerCharacterBound.Intersects(GrassObstacleBound) == true)
                newTexAlpha = 0.3f;
        }

        GrassObstacleTransformInfo->m_TexAlpha = newTexAlpha;
    }


    for (auto& player_iter : m_Players)
    {
        auto otherPlayer = player_iter.second.get();
        if (m_MainPlayer->m_CharacterObjRef->Propensity == otherPlayer->m_CharacterObjRef->Propensity)
            continue;

        auto otherPlayerTransformInfo = otherPlayer->m_CharacterObjRef->m_TransformInfo.get();
        auto& otherPlayerBound = otherPlayerTransformInfo->m_Bound;

        if (otherPlayer->m_CharacterObjRef->PlayerState == PLAYER_STATE::ACT_STEALTH)
        {
            if (MainPlayerCharacterBound.Intersects(otherPlayerBound) == true)
            {
                otherPlayer->SetHP_BarActivate(true);
                
                otherPlayerTransformInfo->m_TexAlpha = otherPlayer->StealthAlpha;
                otherPlayerTransformInfo->m_nonShadowRender = false;
                continue;
            }
            else if (TeamPlayer != nullptr)
            {
                if (TeamPlayerCharacterBound.Intersects(otherPlayerBound) == true)
                {
                    otherPlayer->SetHP_BarActivate(true);
                    otherPlayerTransformInfo->m_TexAlpha = otherPlayer->StealthAlpha;
                    otherPlayerTransformInfo->m_nonShadowRender = false;
                    continue;
                }
            }

            otherPlayer->SetHP_BarActivate(false);
            otherPlayerTransformInfo->m_TexAlpha = 0.0f;
            otherPlayerTransformInfo->m_nonShadowRender = true;
        }
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

        std::string objName;
        const std::vector<RenderItem*>* CharacterRitems = nullptr;
        aiSkeleton* CharacterSkeleton = nullptr;

        std::unordered_map<std::string, float> CharacterAnimNotifys[(int)AnimActionType::Count];

        switch (CharacterType)
        {
        case CHARACTER_TYPE::WARRIOR:
        {
            objName = "Warrior - Instancing(ID:" + std::to_string(New_CE_ID) + ")";
            CharacterRitems = &(m_CharacterRitems[(int)CHARACTER_TYPE::WARRIOR]);
            CharacterSkeleton = ModelSkeletons["Male Knight 01"].get();
            newCharacterObj->m_TransformInfo->SetBound((*m_CharacterModelBoundingBoxesRef)["Male Knight 01"], TransformInfo::BoundPivot::Bottom);
            CharacterAnimNotifys[(int)AnimActionType::Attack]["Warrior_NormalAttackObjGenTiming"] = AnimNotifyTime::Warrior_NormalAttackObjGenTiming;
            CharacterAnimNotifys[(int)AnimActionType::SkillPose]["Warrior_SkillObjGenTiming"] = AnimNotifyTime::Warrior_SkillObjGenTiming;
        }
        break;
        case CHARACTER_TYPE::BERSERKER:
        {
            objName = "Berserker - Instancing(ID:" + std::to_string(New_CE_ID) + ")";
            CharacterRitems = &(m_CharacterRitems[(int)CHARACTER_TYPE::BERSERKER]);
            CharacterSkeleton = ModelSkeletons["Male Warrior 01"].get();
            newCharacterObj->m_TransformInfo->SetBound((*m_CharacterModelBoundingBoxesRef)["Male Warrior 01"], TransformInfo::BoundPivot::Bottom);
            CharacterAnimNotifys[(int)AnimActionType::Attack]["Berserker_NormalAttackObjGenTiming"] = AnimNotifyTime::Berserker_NormalAttackObjGenTiming;
            CharacterAnimNotifys[(int)AnimActionType::SkillPose]["Berserker_SkillObjGenTiming"] = AnimNotifyTime::Berserker_SkillObjGenTiming;
        }
        break;
        case CHARACTER_TYPE::ASSASSIN:
        {
            objName = "Assassin - Instancing(ID:" + std::to_string(New_CE_ID) + ")";
            CharacterRitems = &(m_CharacterRitems[(int)CHARACTER_TYPE::ASSASSIN]);
            CharacterSkeleton = ModelSkeletons["Female Warrior 01"].get();
            newCharacterObj->m_TransformInfo->SetBound((*m_CharacterModelBoundingBoxesRef)["Female Warrior 01"], TransformInfo::BoundPivot::Bottom);
            CharacterAnimNotifys[(int)AnimActionType::Attack]["Assassin_NormalAttackObjGenTiming"] = AnimNotifyTime::Assassin_NormalAttackObjGenTiming;
            CharacterAnimNotifys[(int)AnimActionType::SkillPose]["Assassin_SkillObjGenTiming"] = AnimNotifyTime::Assassin_SkillObjGenTiming;
        }
        break;
        case CHARACTER_TYPE::PRIEST:
        {
            objName = "Priest - Instancing(ID:" + std::to_string(New_CE_ID) + ")";
            CharacterRitems = &(m_CharacterRitems[(int)CHARACTER_TYPE::PRIEST]);
            CharacterSkeleton = ModelSkeletons["Male Mage 01"].get();
            newCharacterObj->m_TransformInfo->SetBound((*m_CharacterModelBoundingBoxesRef)["Male Mage 01"], TransformInfo::BoundPivot::Bottom);
            CharacterAnimNotifys[(int)AnimActionType::Attack]["Priest_NormalAttackObjGenTiming"] = AnimNotifyTime::Priest_NormalAttackObjGenTiming;
            CharacterAnimNotifys[(int)AnimActionType::SkillPose]["Priest_SkillObjGenTiming"] = AnimNotifyTime::Priest_SkillObjGenTiming;
        }
        break;
        }

        // Set Ritem, Skeleton, LocalTransform
        {
            XMFLOAT3 LocalRotationEuler = { 0.0f, 180.0f, 0.0f };
            objManager.SetObjectComponent(newCharacterObj, objName,
                (*CharacterRitems), CharacterSkeleton,
                nullptr, &LocalRotationEuler, nullptr);
            newCharacterObj->m_TransformInfo->UpdateLocalTransform();
        }

        // Set Animations
        {
            auto skeletonInfo = newCharacterObj->m_SkeletonInfo.get();
            auto animInfo = skeletonInfo->m_AnimInfo.get();
            animInfo->Init();

            // Set AnimNotifys
            for (int i = 0; i < (int)AnimActionType::Count; ++i)
            {
                for (auto& AnimNotify : CharacterAnimNotifys[i])
                    animInfo->SetAnimTimeLineNotify((AnimActionType)i, AnimNotify.first, AnimNotify.second);
            }

            // Set Actions
            std::set<std::string> AnimNameList; skeletonInfo->m_Skeleton->GetAnimationList_Name(AnimNameList);
            animInfo->AutoApplyActionFromSkeleton(AnimNameList);
            animInfo->AnimPlay(AnimActionType::Idle);
            animInfo->AnimLoop(AnimActionType::Idle);
        }
    }

    // Create Player
    {
        auto newPlayer = std::make_unique<Player>();
        if (Name.empty() == true)
            newPlayer->m_Name = L"Unknown";
        else
            newPlayer->m_Name = Name;
        newPlayer->m_CharacterObjRef = newCharacterObj;

        if (m_MainPlayer != nullptr)
            newPlayer->MainPlayerPropensity = m_MainPlayer->m_CharacterObjRef->Propensity;

        // Set Character Info Text(Name, HP, etc.)
        {
            newPlayer->m_CharacterInfoTextObjRef = objManager.FindDeactiveTextObject(m_AllObjects, m_TextObjects, m_MaxTextObject);
            newPlayer->m_CharacterInfoTextObjRef->m_Name = "CharacterInfoText Instancing";
            auto& Textinfo = newPlayer->m_CharacterInfoTextObjRef->m_Textinfo;
            Textinfo->m_FontName = L"맑은 고딕";
            Textinfo->m_TextColor = DirectX::Colors::Black;
        }

        // Set HP Bar
        {
            newPlayer->m_HP_BarObjRef[0] = objManager.FindDeactiveUILayOutObject(m_AllObjects, m_UILayOutObjects, m_MaxUILayoutObject);
            newPlayer->m_HP_BarObjRef[1] = objManager.FindDeactiveUILayOutObject(m_AllObjects, m_UILayOutObjects, m_MaxUILayoutObject);
            newPlayer->m_HP_BarObjRef[2] = objManager.FindDeactiveUILayOutObject(m_AllObjects, m_UILayOutObjects, m_MaxUILayoutObject);
            std::vector<RenderItem*> Ritems;
            if (IsMainPlayer == true)
            {
                Ritems = { AllRitems["UI_Layout_HPBarDest_Allies"].get() };

                // Set Other Player HP Bar Ritem (피아식별용)
                OBJECT_PROPENSITY mainPlayerPropensity = Propensity;
                for (auto& Player_iter : m_Players)
                {
                    auto otherPlayer = Player_iter.second.get();
                    auto otherPlayerPropensity = otherPlayer->m_CharacterObjRef->Propensity;
                    auto& otherPlayerHPBarRitems = otherPlayer->m_HP_BarObjRef[0]->m_RenderItems;
                    auto& otherPlayerMainPropensity = otherPlayer->MainPlayerPropensity;
                    if (otherPlayerPropensity == mainPlayerPropensity)
                        otherPlayerHPBarRitems = { AllRitems["UI_Layout_HPBarDest_Allies"].get() };
                    else
                        otherPlayerHPBarRitems = { AllRitems["UI_Layout_HPBarDest_Enemy"].get() };
                    otherPlayerMainPropensity = mainPlayerPropensity;
                }
            }
            else
            {
                if (m_MainPlayer != nullptr)
                {
                    // Set Other Player HP Bar Ritem (피아식별용)
                    OBJECT_PROPENSITY mainPlayerPropensity = m_MainPlayer->m_CharacterObjRef->Propensity;
                    if (Propensity == mainPlayerPropensity)
                        Ritems = { AllRitems["UI_Layout_HPBarDest_Allies"].get() };
                    else
                        Ritems = { AllRitems["UI_Layout_HPBarDest_Enemy"].get() };
                }
                else
                    Ritems = { AllRitems["UI_Layout_HPBarDest_Enemy"].get() };
            }
            objManager.SetObjectComponent(newPlayer->m_HP_BarObjRef[0], "HP Bar Dest - Instancing", Ritems);
            Ritems = { AllRitems["UI_Layout_HPBarIncrease"].get() };
            objManager.SetObjectComponent(newPlayer->m_HP_BarObjRef[1], "HP Bar Increase - Instancing", Ritems);
            Ritems = { AllRitems["UI_Layout_HPBarBack"].get() };
            objManager.SetObjectComponent(newPlayer->m_HP_BarObjRef[2], "HP Bar Back - Instancing", Ritems);

            int maxHP = 0;
            switch (CharacterType)
            {
            case CHARACTER_TYPE::WARRIOR:   maxHP = 100; break;
            case CHARACTER_TYPE::BERSERKER: maxHP = 150; break;
            case CHARACTER_TYPE::ASSASSIN:  maxHP = 100; break;
            case CHARACTER_TYPE::PRIEST:    maxHP = 80; break;
            }
            newPlayer->SetHP(maxHP);
        }

        // Set Transform
        {
            // 스케일을 250으로 늘려줘야 거시적으로
            // 주변 사물과의 크기 비례가 어색하지 않게 된다.
            float ConvertModelUnit = 250.0f;
            XMFLOAT3 WorldScale = { Scale.x * ConvertModelUnit, Scale.y * ConvertModelUnit, Scale.z * ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = RotationEuler;
            XMFLOAT3 WorldPosition = Position;
            newPlayer->SetTransform(WorldScale, WorldRotationEuler, WorldPosition);
        }

        if (IsMainPlayer == true)
        {
            m_MainPlayer = newPlayer.get();

            ObjectManager ObjManager;
            Object* CharacterInfoLayer_SkillName = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_SkillName");
            Object* CharacterInfoLayer_SkillCoolTime = ObjManager.FindObjectName(m_TextObjects, "TextCharacterInfoLayer_SkillCoolTime");
            Object* CharacterInfoLayer = ObjManager.FindObjectName(m_UILayOutObjects, "CharacterInfoLayer");

            auto& CharacterInfoLayer_SkillNameText = CharacterInfoLayer_SkillName->m_Textinfo->m_Text;
            auto& CharacterInfoLayer_SkillCoolTimeText = CharacterInfoLayer_SkillCoolTime->m_Textinfo->m_Text;

            std::vector<RenderItem*> InfoLayer_Ritems;
            switch (CharacterType)
            {
            case CHARACTER_TYPE::WARRIOR:
                CharacterInfoLayer_SkillNameText = L"SwordWave";
                InfoLayer_Ritems = { AllRitems["UI_Layout_Warrior_InfoLayer"].get() };
                break;
            case CHARACTER_TYPE::BERSERKER:
                CharacterInfoLayer_SkillNameText = L"FuryRoar";
                InfoLayer_Ritems = { AllRitems["UI_Layout_Berserker_InfoLayer"].get() };
                break;
            case CHARACTER_TYPE::ASSASSIN:
                CharacterInfoLayer_SkillNameText = L"Stealth";
                InfoLayer_Ritems = { AllRitems["UI_Layout_Assassin_InfoLayer"].get() };
                break;
            case CHARACTER_TYPE::PRIEST:
                CharacterInfoLayer_SkillNameText = L"HealArea";
                InfoLayer_Ritems = { AllRitems["UI_Layout_Priest_InfoLayer"].get() };
                break;
            }
            CharacterInfoLayer->m_RenderItems = InfoLayer_Ritems;
        }
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
        MessageBox(NULL, L"No attack objects available in the world object list.", L"Object Generate Warning", MB_OK);
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
            objName = "SwordNormalAttack - Instancing";
            Ritems = { AllRitems["SkillEffect_SwordSlash_a"].get() };
        }
        break;
        case CHARACTER_TYPE::BERSERKER:
        {
            objName = "AxeNormalAttack - Instancing";
            Ritems = { AllRitems["SkillEffect_Sword_Wave_RedLight"].get() };
        }
        break;
        case CHARACTER_TYPE::ASSASSIN:
        {
            objName = "DaggerNormalAttack - Instancing";
            Ritems = { AllRitems["SkillEffect_Sting_Wave_BlueLight"].get() };
        }
        break;
        case CHARACTER_TYPE::PRIEST:
        {
            objName = "MagicWandNormalAttack - Instancing";
            Ritems = { AllRitems["SkillEffect_Fire_Wave_YellowLight"].get() };
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
    Object* newSkillObj = nullptr;
    if (SkillType == SKILL_TYPE::HOLY_AREA)
    {
        newSkillObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_HealAreaObjects, m_MaxHealAreaObject, true);
        newSkillObj->Activated = true;
        newSkillObj->RenderActivated = true;
    }
    else
        newSkillObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);

    if (newSkillObj == nullptr)
    {
        MessageBox(NULL, L"No skill objects available in the world object list.", L"Object Generate Error", MB_OK);
        throw std::invalid_argument("Object Generate Error");
        return;
    }

    // Set Object Prop
    {
        newSkillObj->m_CE_ID = New_CE_ID;
        newSkillObj->Propensity = Propensity;
        XMFLOAT3 WorldScale = Scale;
        XMFLOAT3 WorldRotationEuler = RotationEuler;
        XMFLOAT3 WorldPosition = Position;
        XMFLOAT3 LocalScale = { 1.0f, 1.0f, 1.0f };
        XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 LocalPosition = { 0.0f, 0.0f, 0.0f };

        std::string objName;
        std::vector<RenderItem*> Ritems;

        switch (SkillType)
        {
        case SKILL_TYPE::SWORD_WAVE:
        {
            objName = "SwordSlash - Instancing";
            Ritems = { AllRitems["SkillEffect_SwordSlash_a"].get() };
        }
        break;
        case SKILL_TYPE::HOLY_AREA:
        {
            XMFLOAT3 PivotScale = WorldScale;
            XMFLOAT3 PivotRotationEuler = WorldRotationEuler;
            WorldPosition.y += 10.0f;
            XMFLOAT3 PivotPosition = WorldPosition;

            const float HealAreaRad = 1000.0f;
            // create holy effect object
            for (int i = 0; i < 20; ++i)
            {
                Object* newSkillEffectObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
                //newSkillEffectObj->m_CE_ID = New_CE_ID;
                newSkillEffectObj->m_Sub_CE_ID = New_CE_ID; // Controlled Object의 CE_ID 중복 방지

                objName = "HolyCross - Instancing";
                Ritems = { AllRitems["SkillEffect_Heal_Cross_GreenLight"].get() };

                XMFLOAT3 newWorldScale = PivotScale;
                XMFLOAT3 newWorldRotationEuler = { 0.0f, 0.0f, 0.0f };
                XMFLOAT3 newWorldPosition = {
                    MathHelper::RandF(PivotPosition.x - HealAreaRad * 0.6f, PivotPosition.x + HealAreaRad * 0.6f),
                    MathHelper::RandF(PivotPosition.y + 50.0f, PivotPosition.y + HealAreaRad * 0.7f),
                MathHelper::RandF(PivotPosition.z - HealAreaRad * 0.6f, PivotPosition.z + HealAreaRad * 0.6f) };
                // Billboard를 감안하여 x-z좌표계평면에 수직이 되도록 한다.
                XMFLOAT3 newLocalRotationEuler = { -90.0f, 0.0f, 0.0f };

                objManager.SetObjectComponent(newSkillEffectObj, objName,
                    Ritems, nullptr,
                    nullptr, &newLocalRotationEuler, nullptr,
                    &newWorldScale, &newWorldRotationEuler, &newWorldPosition);
                newSkillObj->m_TransformInfo->UpdateWorldTransform();
                newSkillEffectObj->m_TransformInfo->m_nonShadowRender = true;
                newSkillEffectObj->RenderActivated = true;

                newSkillObj->m_Childs.push_back(newSkillEffectObj);
                newSkillEffectObj->m_Parent = newSkillObj;

                newSkillEffectObj->SelfDeActivated = true;
                newSkillEffectObj->DeActivatedDecrease = 14.0f; // HolyArea CoolTime: 13.0f

                m_BillboardObjects.push_back(newSkillEffectObj);
                m_EffectObjects.push_back(newSkillEffectObj);
            }

            objName = "HolyArea - Instancing";
            Ritems = { AllRitems["SkillEffect_Heal_Area_GreenLight"].get() };
        }
        break;
        case SKILL_TYPE::FURY_ROAR:
        {
            objName = "FuryRoar - Instancing";
            Ritems = { AllRitems["SkillEffect_Roar_Bear_RedLight"].get() };
            WorldPosition.y += 440.0f;
            WorldPosition.z -= 100.0f;
            // Billboard를 감안하여 x-z좌표계평면에 수직이 되도록 한다.
            LocalRotationEuler = { -90.0f, 0.0f, 0.0f };

            m_BillboardObjects.push_back(newSkillObj);
            m_EffectObjects.push_back(newSkillObj);
        }
        break;
        case SKILL_TYPE::STEALTH:
        {
            const float RandScale = MathHelper::RandF();
            WorldScale = { RandScale, RandScale, RandScale };
            XMFLOAT3 PivotScale = WorldScale;
            XMFLOAT3 PivotRotationEuler = WorldRotationEuler;
            WorldPosition.y += 440.0f;
            WorldPosition.z -= 100.0f;
            XMFLOAT3 PivotPosition = WorldPosition;

            // create stealth effect object
            for (int i = 0; i < 2; ++i)
            {
                Object* newSkillEffectObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
                //newSkillEffectObj->m_CE_ID = New_CE_ID;
                newSkillEffectObj->m_Sub_CE_ID = New_CE_ID; // Controlled Object의 CE_ID 중복 방지

                objName = "StealthFog - Instancing";
                Ritems = { AllRitems["SkillEffect_Smoke_BlueLight"].get() };

                const float randScale = MathHelper::RandF();
                XMFLOAT3 newWorldScale = { randScale, randScale, randScale };
                XMFLOAT3 newWorldRotationEuler = { 0.0f, 0.0f, 0.0f };
                XMFLOAT3 newWorldPosition = PivotPosition;
                newWorldPosition.y += (i + 1) * 10.0f;
                XMVECTOR NEW_WORLD_POS = XMLoadFloat3(&newWorldPosition);
                XMVECTOR STRAFE = m_MainCamera.GetRight();
                STRAFE *= (i > 0) ? -150.0f : 150.0f;
                NEW_WORLD_POS += STRAFE;
                XMStoreFloat3(&newWorldPosition, NEW_WORLD_POS);
                // Billboard를 감안하여 x-z좌표계평면에 수직이 되도록 한다.
                XMFLOAT3 newLocalRotationEuler = { -90.0f, 0.0f, 0.0f };

                objManager.SetObjectComponent(newSkillEffectObj, objName,
                    Ritems, nullptr,
                    nullptr, &newLocalRotationEuler, nullptr,
                    &newWorldScale, &newWorldRotationEuler, &newWorldPosition);
                newSkillObj->m_TransformInfo->UpdateWorldTransform();
                newSkillEffectObj->m_TransformInfo->m_nonShadowRender = true;

                newSkillObj->m_Childs.push_back(newSkillEffectObj);
                newSkillEffectObj->m_Parent = newSkillObj;

                m_BillboardObjects.push_back(newSkillEffectObj);
                m_EffectObjects.push_back(newSkillEffectObj);
            }

            objName = "StealthFog - Instancing";
            Ritems = { AllRitems["SkillEffect_Smoke_BlueLight"].get() };
            // Billboard를 감안하여 x-z좌표계평면에 수직이 되도록 한다.
            LocalRotationEuler = { -90.0f, 0.0f, 0.0f };

            m_BillboardObjects.push_back(newSkillObj);
            m_EffectObjects.push_back(newSkillObj);
        }
        break;
        }

        objManager.SetObjectComponent(newSkillObj, objName,
            Ritems, nullptr,
            &LocalScale, &LocalRotationEuler, &LocalPosition,
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
    XMFLOAT3 LocalScale = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 LocalPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 WorldScale = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 WorldPosition = Position;

    switch (EffectType)
    {
        // SpawnSkillObject()에서 대체한다.
        /*case EFFECT_TYPE::HOLY_EFFECT:
            break;
        case EFFECT_TYPE::FURY_ROAR_EFFECT:
            break;
        case EFFECT_TYPE::STEALTH_EFFECT:
            break;*/
    case EFFECT_TYPE::PICKING_EFFECT:
    {
        auto newEffectObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
        if (newEffectObj == nullptr)
        {
            MessageBox(NULL, L"No effect object available in the world object list.", L"Object Generate Warning", MB_OK);
            return;
        }

        std::string objName = "CrossTarget - Instancing" + std::to_string(++m_EffectInstancingNum);
        std::vector<RenderItem*> Ritems = { AllRitems["PickingEffect_CrossTarget"].get() };
        objManager.SetObjectComponent(newEffectObj, objName,
            Ritems, nullptr,
            &LocalScale, &LocalRotationEuler, &LocalPosition,
            &WorldScale, &WorldRotationEuler, &WorldPosition);
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
    Object* ControledObj = ObjManager.FindObjectCE_ID(m_WorldObjects, CE_ID);;

    auto Player_iter = m_Players.find(CE_ID);
    if (m_Players.find(CE_ID) != m_Players.end())
    {
        float ConvertModelUnit = 250.0f;
        Scale = { Scale.x * ConvertModelUnit, Scale.y * ConvertModelUnit, Scale.z * ConvertModelUnit };
    }

    if (ControledObj != nullptr)
    {
        auto TransformInfo = ControledObj->m_TransformInfo.get();
        TransformInfo->SetWorldScale(Scale);
        TransformInfo->SetWorldRotationEuler(RotationEuler);
        TransformInfo->SetInterpolatedDestPosition(Position);
        //TransformInfo->UpdateWorldTransform();
    }
}

void PlayGameScene::SetCharacterMotion(int CE_ID, MOTION_TYPE MotionType, float MotionSpeed, SKILL_TYPE SkillType)
{
    if (m_Players.find(CE_ID) != m_Players.end())
    {
        m_Players[CE_ID]->PlayMotion(MotionType, MotionSpeed, SkillType);
    }
    else
    {
        ObjectManager ObjManager;
        Object* ControledObj = ObjManager.FindObjectCE_ID(m_CharacterObjects, CE_ID);
        if (ControledObj == nullptr) return;

        auto AnimInfo = ControledObj->m_SkeletonInfo->m_AnimInfo.get();
        AnimActionType CurrPlayingAction = AnimInfo->CurrPlayingAction;
        AnimActionType newActionType = (AnimActionType)MotionType;

        if (newActionType == AnimActionType::Non) return;

        AnimInfo->SetPlaySpeed(MotionSpeed);
        if (CurrPlayingAction != newActionType)
        {
            AnimInfo->AnimStop(CurrPlayingAction);
            AnimInfo->AnimPlay(newActionType);
            AnimInfo->AnimLoop(newActionType, false);
        }
        if (newActionType == AnimActionType::Idle || newActionType == AnimActionType::Walk)
            AnimInfo->AnimLoop(newActionType);
    }
}

void PlayGameScene::SetPlayerState(int CE_ID, PLAYER_STATE PlayerState)
{
    auto Player_iter = m_Players.find(CE_ID);
    if (Player_iter == m_Players.end()) return;
    Player* ControledPlayer = Player_iter->second.get();

    bool HP_UI_AsStealthAct = true;
    if (m_MainPlayer != nullptr)
    {
        if (m_MainPlayer->m_CharacterObjRef->m_CE_ID != CE_ID)
        {
            if (ControledPlayer->m_CharacterObjRef->Propensity != m_MainPlayer->m_CharacterObjRef->Propensity
                && PlayerState == PLAYER_STATE::ACT_STEALTH)
                HP_UI_AsStealthAct = false;
        }
    }
    ControledPlayer->SetHP_BarActivate(HP_UI_AsStealthAct);
    ControledPlayer->SetState(PlayerState);
}

void PlayGameScene::UpdateDeActPoisonGasArea(RECT deActPoisonGasArea)
{
    DeActPoisonGasArea = deActPoisonGasArea;
    PlayGameScene::SpawnPoisonFogObject(DeActPoisonGasArea);
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
        while (m_BillboardObjects.size() > 0)
        {
            auto exceptObj = std::find_if(m_BillboardObjects.begin(), m_BillboardObjects.end(), [&](Object* obj)
                {
                    return (obj->m_Sub_CE_ID == CE_ID || obj->m_CE_ID == CE_ID);
                });
            if (exceptObj != m_BillboardObjects.end()) m_BillboardObjects.erase(exceptObj);
            else break;
        }

        while (m_EffectObjects.size() > 0)
        {
            auto exceptObj = std::find_if(m_EffectObjects.begin(), m_EffectObjects.end(), [&](Object* obj)
                {
                    return (obj->m_Sub_CE_ID == CE_ID); // Controlled Object의 CE_ID 중복 방지를 위해 HP로 대체
                });
            if (exceptObj != m_EffectObjects.end()) m_EffectObjects.erase(exceptObj);
            else break;
        }

        ObjectManager ObjManager;
        Object* ControledObj = ObjManager.FindObjectCE_ID(m_HealAreaObjects, CE_ID);
        if (ControledObj == nullptr)
            ControledObj = ObjManager.FindObjectCE_ID(m_WorldObjects, CE_ID);

        if (ControledObj == nullptr) return;

        //while (ControledObj != nullptr)
        {
            if (ControledObj->m_Parent != nullptr)
            {
                Object* DeactivatedChild = ControledObj;
                ControledObj = ControledObj->m_Parent;
                ObjManager.DeActivateObj(DeactivatedChild);
            }

            if (ControledObj == nullptr)
                std::invalid_argument("ControledObj missed ref");

            if (ControledObj != nullptr)
            {
                std::string ControlledobjName = ControledObj->m_Name;
                XMFLOAT3 ControlledWorldPosition = ControledObj->m_TransformInfo->GetWorldPosition();
                XMFLOAT3 ControlledWorldRotationEuler = ControledObj->m_TransformInfo->GetWorldEuler();
                std::string ControlledObjRitemName = ControledObj->m_RenderItems[0]->Name;
                XMFLOAT3 ControlledObjExtents = ControledObj->m_RenderItems[0]->Geo->DrawArgs[ControlledObjRitemName].Bounds.Extents;

                for (auto& child_obj : ControledObj->m_Childs)
                    ObjManager.DeActivateObj(child_obj);
                ObjManager.DeActivateObj(ControledObj);

                // HealArea에 한해서 FindDeactivateObject 기준이 RenderActivated이므로
                // HealArea의 RenderActivated를 다음과 같이 false인 상태로 바꿔 놓는다.
                if (ControlledobjName.find("Holy") != std::string::npos)
                {
                    // 다른 Spawn 메소드에서 FindDeactivateObject()에 의해 색출되지 않도록
                    // 아래와 같이 Activated는 true, RenderActivated는 false로 지정해준다.
                    ControledObj->Activated = true;
                    ControledObj->RenderActivated = false;
                }
                // NormalAttackObject나 SkillObject(SwordSlash)에 한해서
                // 비활성화가 되는 경우는 무언가에 부딪혔을 때이므로
                // 해당 "부딪힘"을 표현하기 위해 아래와 같이 ExplodingCloud Effect 오브젝트를 스폰해준다.
                else if (ControlledobjName.find("NormalAttack") != std::string::npos || ControlledobjName.find("SwordSlash") != std::string::npos)
                {
                    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
                    auto& AllRitems = *m_AllRitemsRef;

                    ObjectManager objManager;

                    auto newEffectObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
                    if (newEffectObj == nullptr)
                    {
                        MessageBox(NULL, L"No effect object available in the world object list.", L"Object Generate Warning", MB_OK);
                        return;
                    }

                    RenderItem* ExplosionRitem = AllRitems["CollisionEffect_Cloud_Explosion_Orange"].get();
                    XMFLOAT3 newEffectObjExtents = ExplosionRitem->Geo->DrawArgs["CollisionEffect_Cloud_Explosion_Orange"].Bounds.Extents;

                    // EffectObj SpawnPos Offset
                    XMFLOAT3 Offset;
                    {
                        float deg2rad = MathHelper::Pi / 180.0f;

                        XMVECTOR FRONT_VEC = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
                        XMMATRIX RotM = DirectX::XMMatrixRotationRollPitchYaw(ControlledWorldRotationEuler.x * deg2rad, ControlledWorldRotationEuler.y * deg2rad, ControlledWorldRotationEuler.z * deg2rad);

                        XMVECTOR ROTATE_VEC = XMVector3TransformNormal(FRONT_VEC, RotM);

                        float Offset_Distance = ControlledObjExtents.z * 2.0f;
                        XMVECTOR OFFSET_VEC = ROTATE_VEC * Offset_Distance;

                        XMStoreFloat3(&Offset, OFFSET_VEC);
                    }

                    std::string objName = "ExplodingCloud - Instancing" + std::to_string(++m_EffectInstancingNum);
                    std::vector<RenderItem*> Ritems = { ExplosionRitem };
                    XMFLOAT3 newLocalScale = { 1.0f, 1.0f, 1.0f };
                    XMFLOAT3 newLocalRotationEuler = { 0.0f, 0.0f, 0.0f };
                    XMFLOAT3 newLocalPosition = { 0.0f, 0.0f, 0.0f };
                    XMFLOAT3 newWorldScale = { 1.0f, 1.0f, 1.0f };
                    XMFLOAT3 newWorldRotationEuler = { 0.0f, 0.0f, 0.0f };
                    XMFLOAT3 newWorldPosition = ControlledWorldPosition;
                    newWorldPosition.x += Offset.x;
                    newWorldPosition.y += Offset.y;
                    newWorldPosition.z += Offset.z;

                    objManager.SetObjectComponent(newEffectObj, objName,
                        Ritems, nullptr,
                        &newLocalScale, &newLocalRotationEuler, &newLocalPosition,
                        &newWorldScale, &newWorldRotationEuler, &newWorldPosition);
                    newEffectObj->m_TransformInfo->m_nonShadowRender = true;
                    newEffectObj->DeActivatedTime = 0.3f;
                    newEffectObj->DeActivatedDecrease = 0.3f;
                    newEffectObj->SelfDeActivated = true;
                    newEffectObj->DisappearForDeAcTime = true;

                    newEffectObj->m_TransformInfo->SetedSpritedTexture = true;
                    newEffectObj->m_TransformInfo->SetSpritedTextureSize({ 1000.0f, 1000.0f });
                    newEffectObj->m_TransformInfo->SetClipedTextureSize({ 1000.0f / 3.0f, 1000.0f / 3.0f});
                }
            }

            //ControledObj = ObjManager.FindObjectCE_ID(m_WorldObjects, CE_ID);
        }
    }
}

void PlayGameScene::SetKDAScore(unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance)
{
    GameInfo_CountKill = Count_Kill;
    GameInfo_CountDeath = Count_Death;
    GameInfo_CountAssistance = Count_Assistance;
}

void PlayGameScene::SetKillLog(short Kill_Player_id, short Death_Player_id)
{
    wstring KillerName, DeadName;

    for (auto& Player_iter : m_Players)
    {
        auto Player = Player_iter.second.get();
        int PlayerObjID = Player->m_CharacterObjRef->m_CE_ID;
        if (PlayerObjID == Kill_Player_id) KillerName = Player->m_Name;
        else if (PlayerObjID == Death_Player_id) DeadName = Player->m_Name;
    }

    if (KillerName.empty() || DeadName.empty()) return;

    wstring Message = L"[" + KillerName + L"] Killed [" + DeadName + L"]";
    KillLogList.push(Message);
}

void PlayGameScene::SetChatLog(std::wstring Message)
{
    ObjectManager ObjManager;
    Object* ChattingLogPopUpButtonObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameChattingPopUpButton");
    Object* ChattingLogLayer = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_GameChattingLog");

    ChattinglistBox.AddMessage(Message);
    if (ChattinglistBox.IsSizeChanged() == true)
    {
        auto ChattingLogLayerTransform = ChattingLogLayer->m_TransformInfo.get();
        auto ChattingLogLayerPos = ChattingLogLayerTransform->GetWorldPosition();
        auto ChattingLogLayerExtents = ChattingLogLayerTransform->m_Bound.Extents;
        auto ChattingLogRenderTextSize = ChattinglistBox.GetSize();

        XMFLOAT2 newChattingListBoxPos = ChattinglistBox.GetPosition();
        newChattingListBoxPos.y = ChattingLogLayerPos.y - ChattingLogLayerExtents.y * 2 + 30.7f;
        newChattingListBoxPos.y += 26.8f;
        newChattingListBoxPos.y += ChattingLogRenderTextSize.y * 0.5f;
        newChattingListBoxPos.y = m_height / 2.0f - (newChattingListBoxPos.y); // Coord Offset
        ChattinglistBox.SetPosition(newChattingListBoxPos);
    }

    if (ChattingLayerActivate == false)
    {
        auto& AllRitems = *m_AllRitemsRef;
        std::vector<RenderItem*> Ritems = { AllRitems["UI_Layout_GameChattingPopUpButton_alarm"].get() };
        ChattingLogPopUpButtonObject->m_RenderItems = Ritems;
    }
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

void PlayGameScene::SetInGameTeamScore(unsigned char InGameScore_Team)
{
    GameInfo_CountTeamScore = InGameScore_Team;
}

void PlayGameScene::InitPoisonFogObjects()
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    RECT InitDeActPoisonGasArea = { -3423, 4290, 4577, -3710 };

    float InitDeActPoisonGasAreaSize = 8000.0f;
    float InitPoisonGasAreaSize = 10000.0f;
    float PoisonGasAreaSubSize = InitPoisonGasAreaSize - InitDeActPoisonGasAreaSize;
    const float DefaultPoisonGasSize = 200.0f;
    const float BoundaryPoisonGasSize = 1000.0f;
    const float InnerPoisonGasSize = 400.0f;

    int TotalGameTimeLimit = 180;
    float DeActPoisonGasAreaSizeDownInterval = 5;
    int DeActAreaPoisonGasCount = (int)(TotalGameTimeLimit / DeActPoisonGasAreaSizeDownInterval);
    float DeActInnerAreaPoisonGasSize = InitDeActPoisonGasAreaSize / DeActAreaPoisonGasCount; // 222.222222...

    // Size 10000x10000
    RECT InitActPoisonGasArea =
    {
        InitDeActPoisonGasArea.left - 1000,
        InitDeActPoisonGasArea.top + 1000,
        InitDeActPoisonGasArea.right + 1000,
        InitDeActPoisonGasArea.bottom - 1000,
    };

    ObjectManager ObjManager;

    //Left-Right Boundary PoisonGasArea
    {
        int maxCol = (int)((PoisonGasAreaSubSize / 2.0f) / BoundaryPoisonGasSize);
        int maxRow = (int)(InitPoisonGasAreaSize / BoundaryPoisonGasSize);
        float StartGenLeft = (float)InitActPoisonGasArea.left + (BoundaryPoisonGasSize / 2.0f);
        float StartGenRight = (float)InitActPoisonGasArea.right - (BoundaryPoisonGasSize / 2.0f);
        float StartGenTop = (float)InitActPoisonGasArea.top - (BoundaryPoisonGasSize / 2.0f);
        for (int col = 0; col < maxCol; ++col)
        {
            for (int row = 0; row < maxRow; ++row)
            {
                for (int i = 0; i < 2; ++i)
                {
                    Object* newEffectObject = ObjManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);

                    std::string objName = "BoundaryPoisonFog - Instancing";
                    auto Ritems = { AllRitems["SkillEffect_Poison_Fog_GreenLight"].get() };

                    const float randScale = MathHelper::RandF(0.0f, BoundaryPoisonGasSize / DefaultPoisonGasSize);
                    XMFLOAT3 newWorldScale = { randScale, randScale, randScale };
                    XMFLOAT3 newWorldPosition = { 0.0f, 100.0f, 0.0f };
                    if (i == 0)
                        newWorldPosition.x = StartGenLeft + BoundaryPoisonGasSize * col;
                    else
                        newWorldPosition.x = StartGenRight - BoundaryPoisonGasSize * col;
                    newWorldPosition.y = 500.0f;
                    newWorldPosition.z = StartGenTop - BoundaryPoisonGasSize * row;

                    ObjManager.SetObjectComponent(newEffectObject, objName,
                        Ritems, nullptr,
                        nullptr, nullptr, nullptr,
                        &newWorldScale, nullptr, &newWorldPosition);
                    newEffectObject->m_TransformInfo->UpdateWorldTransform();
                    newEffectObject->m_TransformInfo->m_nonShadowRender = true;
                    newEffectObject->m_TransformInfo->m_TexAlpha = randScale / 2.0f;

                    float scale_sign = 1.0f;
                    if (MathHelper::Rand(0, 1) == 0) scale_sign = -1.0f;
                    newEffectObject->DeActivatedDecrease = scale_sign; // Scale Increase Sign 대용

                    m_EffectObjects.push_back(newEffectObject);
                }
            }
        }
    }
    // Inner PoisonGasArea
    {
        for (auto PoisonGasObjects_Row : PoisonGasObjects_inDeActArea)
            PoisonGasObjects_Row.clear();
        PoisonGasObjects_inDeActArea.clear();

        int maxCol = (int)(InitDeActPoisonGasAreaSize / InnerPoisonGasSize);
        int maxRow = (int)(InitDeActPoisonGasAreaSize / InnerPoisonGasSize);
        PoisonGasObjects_inDeActArea.resize((size_t)maxRow);
        for (auto& PoisonGasObjects_Row : PoisonGasObjects_inDeActArea)
            PoisonGasObjects_Row.resize((size_t)maxCol);

        float StartGenLeft = (float)InitDeActPoisonGasArea.left + (InnerPoisonGasSize / 2.0f);
        float StartGenRight = (float)InitDeActPoisonGasArea.right - (InnerPoisonGasSize / 2.0f);
        float StartGenTop = (float)InitDeActPoisonGasArea.top - (InnerPoisonGasSize / 2.0f);
        float StartGenBottom = (float)InitDeActPoisonGasArea.top - (InnerPoisonGasSize / 2.0f);
        for (int col = 0; col < maxCol; ++col)
        {
            for (int row = 0; row < maxRow; ++row)
            {
                Object* newEffectObject = ObjManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);

                std::string objName = "PoisonFog - Instancing";
                auto Ritems = { AllRitems["SkillEffect_Poison_Fog_GreenLight"].get() };

                const float randScale = MathHelper::RandF(0.0f, InnerPoisonGasSize / DefaultPoisonGasSize);
                XMFLOAT3 newWorldScale = { randScale, randScale, randScale };
                XMFLOAT3 newWorldPosition = { 0.0f, 100.0f, 0.0f };
                newWorldPosition.x = StartGenLeft + InnerPoisonGasSize * col;
                newWorldPosition.y = 400.0f;
                newWorldPosition.z = StartGenTop - InnerPoisonGasSize * row;

                ObjManager.SetObjectComponent(newEffectObject, objName,
                    Ritems, nullptr,
                    nullptr, nullptr, nullptr,
                    &newWorldScale, nullptr, &newWorldPosition);
                newEffectObject->m_TransformInfo->UpdateWorldTransform();
                newEffectObject->m_TransformInfo->m_nonShadowRender = true;
                newEffectObject->m_TransformInfo->m_TexAlpha = randScale / 2.0f;
                newEffectObject->RenderActivated = false;

                float scale_sign = 1.0f;
                if (MathHelper::Rand(0, 1) == 0) scale_sign = -1.0f;
                newEffectObject->DeActivatedDecrease = scale_sign; // Scale Increase Sign 대용

                m_EffectObjects.push_back(newEffectObject);
                PoisonGasObjects_inDeActArea[row][col] = newEffectObject;
            }
        }
    }

    PlayGameScene::SpawnPoisonFogObject(DeActPoisonGasArea);
}

void PlayGameScene::SpawnPoisonFogObject(const RECT& DeActArea)
{
    const RECT InitDeActPoisonGasArea = { -3423, 4290, 4577, -3710 };
    const float InitDeActPoisonGasAreaSize = 8000.0f;
    const float DefaultPoisonGasSize = 200.0f;
    const float InnerPoisonGasSize = 400.0f;

    RECT RemainDeActPoisonGasArea = DeActArea;

    // Test Reduced DeActPoisonArea
    /*{
        int ReducedCount = 6;
        RemainDeActPoisonGasArea.left +=   (int)InnerPoisonGasSize * ReducedCount;
        RemainDeActPoisonGasArea.top -=    (int)InnerPoisonGasSize * ReducedCount;
        RemainDeActPoisonGasArea.right -=  (int)InnerPoisonGasSize * ReducedCount;
        RemainDeActPoisonGasArea.bottom += (int)InnerPoisonGasSize * ReducedCount;
    }*/

    int maxCol = (int)(InitDeActPoisonGasAreaSize / InnerPoisonGasSize);
    int maxRow = (int)(InitDeActPoisonGasAreaSize / InnerPoisonGasSize);
    float StartGenLeft = (float)InitDeActPoisonGasArea.left + (InnerPoisonGasSize / 2.0f);
    float StartGenRight = (float)InitDeActPoisonGasArea.right - (InnerPoisonGasSize / 2.0f);
    float StartGenTop = (float)InitDeActPoisonGasArea.top - (InnerPoisonGasSize / 2.0f);
    float StartGenBottom = (float)InitDeActPoisonGasArea.top - (InnerPoisonGasSize / 2.0f);
    for (int col = 0; col < maxCol; ++col)
    {
        for (int row = 0; row < maxRow; ++row)
        {
            float ActPos_x = StartGenLeft + InnerPoisonGasSize * col;
            float ActPos_z = StartGenTop - InnerPoisonGasSize * row;
            if (ActPos_x <= RemainDeActPoisonGasArea.left || RemainDeActPoisonGasArea.right <= ActPos_x
                || ActPos_z >= RemainDeActPoisonGasArea.top || RemainDeActPoisonGasArea.bottom >= ActPos_z)
            {
                // Activate 대용 (Activate를 false하면 FindDeactiveWorldObject()에서 인식해버린다)
                PoisonGasObjects_inDeActArea[row][col]->RenderActivated = true;
            }
        }
    }
}
