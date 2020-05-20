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

    m_SpawnBound.Center = m_WorldCenter;
    m_SpawnBound.Extents.x = 1355.4405f;
    m_SpawnBound.Extents.z = 1243.2412f;
}

void PlayGameScene::OnInitProperties(CTimer& gt)
{
    for (auto& obj : m_CharacterObjects)
    {
        if (obj->m_Name == "Meshtint Free Knight")
        {
            auto transformInfo = obj->m_TransformInfo.get();
            // �������� 2�� �÷���� �Ž�������
            // �ֺ� �繰���� ũ�� ��ʰ� ������� �ʰ� �ȴ�.
            float ConvertModelUnit = ModelFileUnit::meter * 2.0f;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 17.86f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 180.0f, 0.0f };
            transformInfo->Init();
            auto& MeshBound = obj->m_RenderItem->Geo->DrawArgs[obj->m_RenderItem->Name].Bounds;
            transformInfo->SetBound(MeshBound, TransformInfo::BoundPivot::Bottom);
            transformInfo->SetWorldTransform(WorldScale, WorldRotationEuler, WorldPosition);
            transformInfo->SetLocalRotationEuler(LocalRotationEuler);


            auto skeletonInfo = obj->m_SkeletonInfo.get();
            auto animInfo = skeletonInfo->m_AnimInfo.get();
            animInfo->Init();
            animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
            animInfo->SetAnimTimeLineNotify("Meshtint Free Knight@Sword And Shield Slash-SlashGen", ActionNotifyTime::MeshtintFreeKnight_SwordSlashStart);
        }
        else
        {
            ObjectManager objManager;
            if (obj->m_Name.find("Instancing") != std::string::npos)
            {
                for (auto& attachedObj : obj->m_Childs)
                    objManager.DeActivateObj(attachedObj);
            }
            objManager.DeActivateObj(obj);
        }
    }

    for (auto& obj : m_WorldObjects)
    {
        if (obj->m_Name.find("Instancing") != std::string::npos)
        {
            ObjectManager objManager;
            for (auto& attachedObj : obj->m_Childs)
                objManager.DeActivateObj(attachedObj);
            objManager.DeActivateObj(obj);
        }
    }

    if (m_MainPlayer != nullptr)
    {
        m_MainPlayer->m_CurrAction = ActionType::Idle;
        m_MainPlayer->SetCreateSkillObjRef(m_AllObjects, m_WorldObjects, m_MaxWorldObject, *m_AllRitemsRef, m_CurrSkillObjInstanceNUM);
    }

    m_CurrSkillObjInstanceNUM = 0;

    m_LightRotationAngle = 0.0f;


    SceneStartTime = gt.GetTotalTime();
    sec = sec2 = 0;
    time_str = L"Time Limit\n   03:00";
    TimeLimit_sec = 180;

    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    static XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
}

void PlayGameScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt)
{
    PlayGameScene::AnimateWorldObjectsTransform(gt);
    PlayGameScene::ProcessCollision(gt);
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt);

    float totalTime = gt.GetTotalTime() - SceneStartTime;
    int totalTime_i = (int)totalTime;

    if ((totalTime_i != 0) && (totalTime_i % 5 == 0))
    {
        if (sec2 < totalTime_i / 5)
        {
            sec2 = totalTime_i / 5;
            this->RandomCreateCharacterObject();
        }
    }
}

void PlayGameScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;

    m_MainPlayer = std::make_unique<Player>();

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

            // �������� 2�� �÷���� �Ž�������
            // �ֺ� �繰���� ũ�� ��ʰ� ������� �ʰ� �ȴ�.
            float ConvertModelUnit = ModelFileUnit::meter * 2.0f;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 17.86f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 180.0f, 0.0f };
            objManager.SetObjectComponent(newObj, objName, Ritem,
                ModelSkeletons["Meshtint Free Knight"].get(),
                nullptr, &LocalRotationEuler, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);
            auto& MeshBound = newObj->m_RenderItem->Geo->DrawArgs[newObj->m_RenderItem->Name].Bounds;
            newObj->m_TransformInfo->SetBound(MeshBound, TransformInfo::BoundPivot::Bottom);
            newObj->m_TransformInfo->UpdateLocalTransform();
            newObj->m_TransformInfo->UpdateWorldTransform();
            auto animInfo = newObj->m_SkeletonInfo->m_AnimInfo.get();
            animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
            animInfo->SetAnimTimeLineNotify("Meshtint Free Knight@Sword And Shield Slash-SlashGen", ActionNotifyTime::MeshtintFreeKnight_SwordSlashStart);

            m_MainPlayer->m_ObjectRef = newObj;
        }
        else if (Ritem_iter.first.find("UI") != std::string::npos)
        {
            const UINT maxUIObject = (UINT)Geometries["PlayGameSceneUIGeo"]->DrawArgs.size();
            auto newObj = objManager.CreateUIObject(objCB_index++, m_AllObjects, m_UIObjects, maxUIObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            // UI ������Ʈ ���� ��쿡�� �޽��� ������ (0,0,0)�� �ƴ϶�
            // �̹� ���ؽ� ��ü�� ��ũ����ǥ�� �������� �����Ǿ� �ֱ� ������
            // UI ������Ʈ�� Transform�� ���ؼ� ���� �������� �ʾƵ� �ȴ�.
            std::string objName = Ritem_iter.first;
            objManager.SetObjectComponent(newObj, objName, Ritem);

            if (objName == "UI_Layout_SkillList") continue;
            newObj->m_UIinfos[objName] = std::make_unique<TextInfo>();
            auto text_info = newObj->m_UIinfos[objName].get();
            text_info->m_FontName = L"���� ���";
            text_info->m_TextColor = DirectX::Colors::Blue;
            auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
            text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;
            text_info->TextBatchIndex = textBatch_index++;

            if (objName == "UI_Layout_GameTimeLimit") text_info->m_Text = L"Time Limit";
            else if (objName == "UI_Layout_KDA")      text_info->m_Text = L"K: 0  D: 0  A: 0";
            else if (objName == "UI_Layout_KillLog")  text_info->m_Text = L"Kill Log";
            else if (objName == "UI_Layout_ChattingLog")  text_info->m_Text = L"Chatting Log";
            //else if (objName == "UI_Layout_SkillList");
            else if (objName == "UI_Layout_Skill1")  text_info->m_Text = L"Skill1";
            else if (objName == "UI_Layout_Skill2")  text_info->m_Text = L"Skill2";
            else if (objName == "UI_Layout_Skill3")  text_info->m_Text = L"Skill3";
            else if (objName == "UI_Layout_Skill4")  text_info->m_Text = L"Skill4";
        }
        else if (Ritem_iter.first.find("Effect") != std::string::npos)
        {
            continue;
        }
        else
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);
            std::string objName = Ritem_iter.first;

            // ��� �������� ������ ĳ���� ������Ʈ��
            // Attaching �� �Ŷ� WorldTransform�� �������� �ʾƵ� �ȴ�.
            if (objName == "Sword" || objName == "Shield")
            {
                objName += " - Equipment";
                float ConvertModelUnit = ModelFileUnit::meter;
                XMFLOAT3 ModelLocalScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
                if (objName.find("Sword") != std::string::npos)
                {
                    XMFLOAT3 ModelLocalRotationEuler = { 1.0f, 3.0f, 83.0f };
                    XMFLOAT3 ModelLocalPosition = { 10.0f, -7.0f, 1.0f };
                    objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                        &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
                    objManager.SetAttaching(newObj, m_MainPlayer->m_ObjectRef, "RigRPalm");
                }
                else if (objName.find("Shield") != std::string::npos)
                {
                    XMFLOAT3 ModelLocalRotationEuler = { 12.0f, -96.0f, 93.0f };
                    XMFLOAT3 ModelLocalPosition = { 0.0f, 0.0f, 0.0f };
                    objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                        &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
                    objManager.SetAttaching(newObj, m_MainPlayer->m_ObjectRef, "RigLPalm");
                }
            }
            else if (objName == "SpawnStageGround")
            {
                XMFLOAT3 WorldPosition = { 17.86f, 5.0f, 0.0f };
                objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                    nullptr, nullptr, nullptr,
                    nullptr, nullptr, &WorldPosition);
            }
            else // Environment Object
            {
                // ȯ�� �繰 ���� ��쿡�� �޽��� ������ (0,0,0)�� �ƴ϶�
                // �̹� ���� ��ü���� �޽��� ��ġ�Ǿ� �ִ� ���¶�
                // ȯ�� �繰�� Transform�� ���ؼ� ���� �������� �ʾƵ� �ȴ�.
                objManager.SetObjectComponent(newObj, objName, Ritem);

                if(objName == "Floor1")
                {
                    auto& FloorBound = newObj->m_TransformInfo->m_Bound;
                    float width = FloorBound.Extents.x;
                    float depth = FloorBound.Extents.z;

                    m_SceneBounds.Center = m_WorldCenter = FloorBound.Center;
                    // m_SceneBounds.Radius�� ����ؼ� ShadowMap�� ���εǴٺ���
                    // ShadowMap�� �ػ󵵺��� m_SceneBounds.Radius�� ũ��
                    // BackBuffer�� ���εǴ� �׸����� �ػ󵵰� ��������� �������� �ȴ�.
                    m_SceneBounds.Radius = (float)SIZE_ShadowMap*2;
                }
            }
        }
    }

    // Create DeActive Objects
    {
        // SkinnedCB�� ����(Skeleton�� �ִ�) ������Ʈ�� �����ϸ�
        // SkinnedCB�Ӹ� �ƴ϶� ObjCB��(WorldObject��) ������ ������
        // WorldObject���� ���� SkinnedObject�� ���� �������ش�.
        const UINT nDeAcativateCharacterObj = m_MaxCharacterObject - (UINT)m_CharacterObjects.size();
        for (UINT i = 0; i < nDeAcativateCharacterObj; ++i)
        {
            auto newObj = objManager.CreateCharacterObject(
                objCB_index++, skinnedCB_index++,
                m_AllObjects,
                m_CharacterObjects, m_MaxCharacterObject,
                m_WorldObjects, m_MaxWorldObject);
            // CreateCharacterObject���������� FindDeactiveCharacterObject()�� ����
            // ���ο� ������Ʈ�� ����� �Ǵµ�
            // �̶� FindDeactiveCharacterObject()��
            // ���� ������Ʈ ����Ʈ �߿��� Active�� false�� ������Ʈ�� ���� ã�Ƽ� ��ȯ�ϱ� ������
            // ��ó�� �Ѳ����� ���� ������Ʈ�� ���� ��쿡��
            // �� ������Ʈ�� Activate ���θ� ������Ʈ���� �� ����� ���� ��������� �Ѵ�.
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

    m_MainPlayer->SetCreateSkillObjRef(m_AllObjects, m_WorldObjects, m_MaxWorldObject, AllRitems, m_CurrSkillObjInstanceNUM);

    m_nObjCB = (UINT)m_WorldObjects.size() + (UINT)m_UIObjects.size();
    m_nSKinnedCB = (UINT)m_CharacterObjects.size();
    for (auto& ui_obj : m_UIObjects) m_nTextBatch += (UINT)ui_obj->m_UIinfos.size();
}

// character & equipment object generate test
void PlayGameScene::RandomCreateCharacterObject()
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr || m_ModelSkeltonsRef == nullptr) return;
    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;
    auto& ModelSkeletons = *m_ModelSkeltonsRef;

    ObjectManager objManager;
    Object* newCharacterObj = nullptr;
    static int CharacterCreatingNum = 1;
    // character gen test
    {
        newCharacterObj = objManager.FindDeactiveCharacterObject(
            m_AllObjects,
            m_CharacterObjects, m_MaxCharacterObject,
            m_WorldObjects, m_MaxWorldObject);

        if (newCharacterObj == nullptr)
        {
            MessageBox(NULL, L"No characters available in the character list.", L"Object Generate Warning", MB_OK);
            return;
        }

        // �������� 2�� �÷���� �Ž�������
        // �ֺ� �繰���� ũ�� ��ʰ� ������� �ʰ� �ȴ�.
        float ConvertModelUnit = ModelFileUnit::meter * 2.0f;
        XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
        XMFLOAT3 WorldRotationEuler = { 0.0f, MathHelper::RandF(0.0f, 360.0f), 0.0f };
        XMFLOAT3 WorldPosition =
        {
            MathHelper::RandF(-m_SpawnBound.Extents.x, m_SpawnBound.Extents.x),
            0.0f,
            MathHelper::RandF(-m_SpawnBound.Extents.z, m_SpawnBound.Extents.z),
        };
        XMFLOAT3 LocalRotationEuler = { 0.0f, 180.0f, 0.0f };
        std::string objName = "Meshtint Free Knight - Instancing" + std::to_string(CharacterCreatingNum++);
        objManager.SetObjectComponent(newCharacterObj, objName,
            AllRitems["Meshtint Free Knight"].get(),
            ModelSkeletons["Meshtint Free Knight"].get(),
            nullptr, &LocalRotationEuler, nullptr,
            &WorldScale, &WorldRotationEuler, &WorldPosition);
        auto& MeshBound = newCharacterObj->m_RenderItem->Geo->DrawArgs[newCharacterObj->m_RenderItem->Name].Bounds;
        newCharacterObj->m_TransformInfo->SetBound(MeshBound, TransformInfo::BoundPivot::Bottom);
        newCharacterObj->m_TransformInfo->UpdateLocalTransform();
        newCharacterObj->m_TransformInfo->UpdateWorldTransform();

        auto animInfo = newCharacterObj->m_SkeletonInfo->m_AnimInfo.get();
        animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
        animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
        animInfo->SetAnimTimeLineNotify("Meshtint Free Knight@Sword And Shield Slash-SlashGen", ActionNotifyTime::MeshtintFreeKnight_SwordSlashStart);
    }

    // equipment gen test
    {
        static int EquipmentCreatingNum = 1;

        float ConvertModelUnit = ModelFileUnit::meter;
        XMFLOAT3 ModelLocalScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
        // Attaching Sword
        {
            auto newEquipmentObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            if (newEquipmentObj == nullptr)
            {
                MessageBox(NULL, L"No equipment object available in the world object list.", L"Object Generate Warning", MB_OK);
                return;
            }

            XMFLOAT3 ModelLocalRotationEuler = { 1.0f, 3.0f, 83.0f };
            XMFLOAT3 ModelLocalPosition = { 10.0f, -7.0f, 1.0f };
            std::string objName = "Sword - Equipment - Instancing" + std::to_string(EquipmentCreatingNum);
            objManager.SetObjectComponent(newEquipmentObj, objName,
                AllRitems["Sword"].get(), nullptr,
                &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
            objManager.SetAttaching(newEquipmentObj, newCharacterObj, "RigRPalm");
        }

        // Attaching Shield
        {
            auto newEquipmentObj = objManager.FindDeactiveWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            if (newEquipmentObj == nullptr)
            {
                MessageBox(NULL, L"No equipment object available in the world object list.", L"Object Generate Warning", MB_OK);
                return;
            }

            XMFLOAT3 ModelLocalRotationEuler = { 12.0f, -96.0f, 93.0f };
            XMFLOAT3 ModelLocalPosition = { 0.0f, 0.0f, 0.0f };
            std::string objName = "Shield - Equipment - Instancing" + std::to_string(EquipmentCreatingNum++);
            objManager.SetObjectComponent(newEquipmentObj, objName,
                AllRitems["Shield"].get(), nullptr,
                &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
            objManager.SetAttaching(newEquipmentObj, newCharacterObj, "RigLPalm");
        }
    }
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

void PlayGameScene::UpdateTextInfo(CTimer& gt)
{
    float totalTime = gt.GetTotalTime() - SceneStartTime;
    int totalTime_i = (int)totalTime;

    if ((totalTime_i != 0) && (totalTime_i % 1 == 0))
    {
        if (sec < totalTime_i / 1)
        {
            sec = totalTime_i / 1;
            TimeLimit_sec = ((int)TimeLimit_sec - 1 > 0) ? TimeLimit_sec - 1 : 0;
            time_str = L"Time Limit\n   ";
            if ((TimeLimit_sec / 60) / 10 == 0)
                time_str += std::to_wstring(0);
            time_str += std::to_wstring(TimeLimit_sec / 60);
            time_str += L":";
            if ((TimeLimit_sec % 60) / 10 == 0)
                time_str += std::to_wstring(0);
            time_str += std::to_wstring(TimeLimit_sec % 60);
        }
    }

    for (auto& obj : m_UIObjects)
    {
        for (auto& ui_info_iter : obj->m_UIinfos)
        {
            auto ui_info = ui_info_iter.second.get();
            if (obj->m_Name.find("TimeLimit") != std::string::npos)
                ui_info->m_Text = time_str;
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

void PlayGameScene::AnimateSkeletons(CTimer& gt)
{
    if (m_MainPlayer != nullptr)
    {
        CD3DX12_VIEWPORT ViewPort(0.0f, 0.0f, (float)m_width, (float)m_height);
        m_MainPlayer->ProcessSkeletonAnimDurationDone();
    }

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
                    // child ������Ʈ�� AttachingTargetBoneID�� �������� AnimatedTransform�� ������
                    // child ������Ʈ�� ���� Ʈ�������� ��ü�Ѵ�.
                    int AttachedBoneID = childObjInfo->m_AttachingTargetBoneID;
                    if (AttachedBoneID != -1)
                    {
                        XMFLOAT4X4 AnimTransform = MathHelper::Identity4x4();
                        Scene::aiM2dxM(AnimTransform, AnimInfo->CurrAnimJointTransforms[AttachedBoneID]);

                        // UpdateSkinnedCBs()���� skinConstants.BoneTransform�� ���ԵǴ�
                        // AnimTransform�� ��ġ���� ���� ������ Transform�̴�.
                        // ������ UpdateObjectCBs()���� ���� AnimTransform�� �����
                        // WorldTransform�� ��ġ�Ǿ������ ����.
                        // �̸� �̿��� �����ϱ� ����(AnimTransform�� ��ġ�� ����ϱ� ����)
                        // Attaching�� ���� ������Ʈ�� WorldTransform�� ������ ������
                        // AnimTransform�� �̸� ��ġ��Ų��.
                        XMStoreFloat4x4(&AnimTransform, XMMatrixTranspose(XMLoadFloat4x4(&AnimTransform)));
                        XMMATRIX AnimM = XMLoadFloat4x4(&AnimTransform);

                        // ������Ʈ �޽� Ʈ�������� ���� ��� �� ����
                        // AnimTransform * LocalTransform * WorldTransform
                        // Attaching�� �� ������Ʈ��
                        // �ش� ������Ʈ�� Skeleton�� ���ϰ� �ִ� ���� �ƴϱ� ������
                        // VS���̴� �ܰ迡�� animTransform�� ����ϴ� ������ �����ȴ�
                        // ���, Attaching ������Ʈ�� �ִϸ��̼��� ���ؼ�
                        // Attaching ������Ʈ�� LocalTransform * WorldTransform��
                        // AnimTransform�� LocalTransform * WorldTransform�� ������ ����̾�� �Ѵ�.
                        // �̶� LocalTransform�� WorldTransform�� �ش� ������Ʈ�� World/Local�� �ƴ�
                        // Attaching�� ���� ������Ʈ(��� ������Ʈ �������� ĳ���� ������Ʈ)
                        // �� Local/WorldTransform�� �����ش�.
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
        camAngle += 5.0f * deg2rad;
        float phi = 30.0f * deg2rad;
        float rad = 1500.0f;
        XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
        Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
        XMStoreFloat3(&EyePosition, Eye_Pos);
    }

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 10000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}

void PlayGameScene::AnimateWorldObjectsTransform(CTimer& gt)
{
    for (auto& obj : m_WorldObjects)
    {
        if (obj->ProcessSelfDeActivate(gt) != true)
            obj->m_TransformInfo->Animate(gt);
    }
}

void PlayGameScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt)
{
    if (m_MainPlayer != nullptr)
    {
        CD3DX12_VIEWPORT ViewPort((float)m_ClientRect.left, (float)m_ClientRect.top, (float)m_ClientRect.right, (float)m_ClientRect.bottom);
        m_MainPlayer->ProcessInput(key_state, oldCursorPos, ViewPort, gt);
    }
}

void PlayGameScene::ProcessCollision(CTimer& gt)
{
    if (m_MainPlayer != nullptr)
        m_MainPlayer->ProcessCollision(m_WorldObjects);
}
