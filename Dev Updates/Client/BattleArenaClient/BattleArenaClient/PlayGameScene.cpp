#include "stdafx.h"
#include "PlayGameScene.h"

PlayGameScene::~PlayGameScene()
{
}

void PlayGameScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    Scene::OnInit(device, commandList,
        BackBufferFormat,
        matCB_index, diffuseSrvHeap_Index,
        objCB_index, skinnedCB_index, textBatch_index);

    m_SpawnBound.Center = m_WorldCenter;
    m_SpawnBound.Extents.x = 1355.4405f;
    m_SpawnBound.Extents.z = 1243.2412f;
}

void PlayGameScene::OnInitProperties()
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

    if (m_MainPlayer != nullptr) m_MainPlayer->m_CurrAction = ActionType::Idle;

    m_CurrSkillObjInstanceNUM = 0;

    m_LightRotationAngle = 0.0f;
}

void PlayGameScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt)
{
    PlayGameScene::AnimateWorldObjectsTransform(gt);
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt);
}

void PlayGameScene::DisposeUploaders()
{
    Scene::DisposeUploaders();
}

void PlayGameScene::BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    GeometryGenerator geoGen;

    // UI Position relative to DC(Device Coordinate) with origin at screen center.
    std::unordered_map<std::string, GeometryGenerator::MeshData> UILayerBacground_Meshes;
    UILayerBacground_Meshes["UI_Layout_GameTimeLimit"] = geoGen.CreateQuad(-50.0f, 360.0F, 100.0f, 60.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_KDA"] = geoGen.CreateQuad(-640.0f, 360.0f, 200.0f, 70.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_KillLog"] = geoGen.CreateQuad(-640.0f, 280.0f, 200.0f, 200.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_ChattingLog"] = geoGen.CreateQuad(-640.0f, 20.0f, 200.0f, 300.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_SkillList"] = geoGen.CreateQuad(-250.0f, -280.0f, 500.0f, 80.0f, 0.1f);
    UILayerBacground_Meshes["UI_Layout_Skill1"] = geoGen.CreateQuad(-200.0f, -295.0f, 50.0f, 50.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_Skill2"] = geoGen.CreateQuad(-80.0f, -295.0f, 50.0f, 50.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_Skill3"] = geoGen.CreateQuad(30.0f, -295.0f, 50.0f, 50.0f, 0.0f);
    UILayerBacground_Meshes["UI_Layout_Skill4"] = geoGen.CreateQuad(140.0f, -295.0f, 50.0f, 50.0f, 0.0f);

    m_Geometries["PlayGameSceneUIGeo"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "PlayGameSceneUIGeo", UILayerBacground_Meshes));

    std::unordered_map<std::string, GeometryGenerator::MeshData> StageGround_Meshes;
    StageGround_Meshes["ground_grid"] = geoGen.CreateGrid(1000.0f, 1000.0f, 10, 10);
    m_Geometries["GamePlayGround"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "GamePlayGround", StageGround_Meshes));

    std::unordered_map<std::string, GeometryGenerator::MeshData> EffectGeo_Meshs;
    EffectGeo_Meshs["SkillEffect_SwordSlash_a"] = geoGen.CreateGrid(326.0f, 200.0f, 10, 10);
    EffectGeo_Meshs["PickingEffect_CrossTarget"] = geoGen.CreateGrid(150.0f, 150.0f, 10, 10);
    m_Geometries["GameEffectGeo"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "GameEffectGeo", EffectGeo_Meshs));
}

void PlayGameScene::LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    ModelLoader model_loader;

    std::string mesh_path = "Models/Environment/Environment.fbx";
    std::vector<std::string> anim_paths;
    std::vector<std::string> execptProcessing_file_nodes = { "Environment_root", "RootNode" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);

    mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    anim_paths = {
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Stride Walking.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Sword And Shield Walk.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Sword And Shield Slash.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/Meshtint Free Knight/Meshes/Shield.fbx";
    anim_paths.clear();
    execptProcessing_file_nodes = { "PreRotation", "RootNode" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);

    mesh_path = "Models/Meshtint Free Knight/Meshes/Sword.fbx";
    anim_paths.clear();
    execptProcessing_file_nodes = { "PreRotation", "RootNode" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);
}

void PlayGameScene::LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat)
{
    std::vector<std::string> material_filepaths =
    {
        "Models/Environment/Materials/TextureWorld.png",
        "Models/Meshtint Free Knight/Materials/Meshtint Free Knight.tga",
        "UI/White_Transparency50.png",
        "UI/LightGreen_Transparency50.png",
        "UI/Effect/SwordSlash_a.png",
        "UI/Effect/CrossTarget.png"
    };

    for (auto& texture_path : material_filepaths)
    {
        TextureLoader texLoader(texture_path.c_str());
        TextureData texInfo;
        texLoader.MoveTextureData(texInfo);
        if (texInfo.Pixels == nullptr) continue;
        std::vector<std::uint8_t>& Pixels = *texInfo.Pixels;
        if (Pixels.empty()) continue;

        UINT texWidth = (UINT)texInfo.Width;
        UINT texHeight = (UINT)texInfo.Height;
        UINT texPixelSize = texInfo.BytesPerPixel;

        std::string texName;
        getFileName(texture_path.c_str(), texName);
        auto& tex = m_Textures[texName] = std::make_unique<Texture>();
        tex->Name = texName;

        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = BackBufferFormat;
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&tex->Resource)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex->Resource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&tex->UploadHeap)));

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = Pixels.data();
        textureData.RowPitch = texWidth * texPixelSize;
        textureData.SlicePitch = textureData.RowPitch * texHeight;

        UpdateSubresources(commandList, tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    // Create the non-tex texture.
    {
        auto& tex = m_Textures["checkerboard"] = std::make_unique<Texture>();
        tex->Name = "checkerboard";

        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = 256;
        textureDesc.Height = 256;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&tex->Resource)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex->Resource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&tex->UploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = 256 * 4;
        textureData.SlicePitch = textureData.RowPitch * 256;

        UpdateSubresources(commandList, tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }
}

void PlayGameScene::BuildMaterials(int& matCB_index, int& diffuseSrvHeap_Index)
{
    for (auto& tex_iter : m_Textures)
    {
        auto& tex = tex_iter.second;

        auto mat = std::make_unique<Material>();
        mat->Name = tex->Name;
        mat->NumFramesDirty = gNumFrameResources;
        mat->MatCBIndex = matCB_index++;
        mat->DiffuseSrvHeapIndex = diffuseSrvHeap_Index++;
        mat->DiffuseAlbedo = XMFLOAT4(0.88f, 0.88f, 0.88f, 1.0f);
        mat->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
        mat->Roughness = 0.8f;

        m_Materials[mat->Name] = std::move(mat);
    }
    m_nMatCB = (UINT)m_Materials.size();
}

void PlayGameScene::BuildRenderItems()
{
    Scene::BuildRenderItem(m_AllRitems, m_Geometries["GamePlayGround"].get());
    for (auto& subMesh : m_Geometries["GamePlayGround"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["checkerboard"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Environment"].get());
    for (auto& subMesh : m_Geometries["Environment"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["TextureWorld"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Meshtint Free Knight"].get());
    for (auto& subMesh : m_Geometries["Meshtint Free Knight"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Shield"].get());
    for (auto& subMesh : m_Geometries["Shield"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Sword"].get());
    for (auto& subMesh : m_Geometries["Sword"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["PlayGameSceneUIGeo"].get());
    for (auto& subMesh_iter : m_Geometries["PlayGameSceneUIGeo"]->DrawArgs)
    {
        std::string subMeshName = subMesh_iter.first;
        if (subMeshName.find("Skill") != std::string::npos)
        {
            if(subMeshName.find("List") != std::string::npos)
                m_AllRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
            else m_AllRitems[subMeshName]->Mat = m_Materials["LightGreen_Transparency50"].get();
        }
        else m_AllRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
    }

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["GameEffectGeo"].get());
    for (auto& subMesh_iter : m_Geometries["GameEffectGeo"]->DrawArgs)
    {
        std::string subMeshName = subMesh_iter.first;
        if(subMeshName == "SkillEffect_SwordSlash_a")
            m_AllRitems[subMeshName]->Mat = m_Materials["SwordSlash_a"].get();
        else if(subMeshName == "PickingEffect_CrossTarget")
            m_AllRitems[subMeshName]->Mat = m_Materials["CrossTarget"].get();
    }
}

void PlayGameScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    ObjectManager objManager;

    m_MainPlayer = std::make_unique<Player>();

    for (auto& Ritem_iter : m_AllRitems)
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
                m_ModelSkeltons["Meshtint Free Knight"].get(),
                nullptr, &LocalRotationEuler, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);
            auto animInfo = newObj->m_SkeletonInfo->m_AnimInfo.get();
            animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
            animInfo->SetAnimTimeLineNotify("Meshtint Free Knight@Sword And Shield Slash-SlashGen", ActionNotifyTime::MeshtintFreeKnight_SwordSlashStart);

            m_MainPlayer->m_ObjectRef = newObj;
        }
        else if (Ritem_iter.first.find("UI") != std::string::npos)
        {
            const UINT maxUIObject = (UINT)m_Geometries["PlayGameSceneUIGeo"]->DrawArgs.size();
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
            else if (objName == "UI_Layout_KDA")      text_info->m_Text = L"K: 0    D: 0    A: 0";
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
                float ConvertModelUnit = ModelFileUnit::meter;
                XMFLOAT3 ModelLocalScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
                if (objName == "Sword")
                {
                    XMFLOAT3 ModelLocalRotationEuler = { 1.0f, 3.0f, 83.0f };
                    XMFLOAT3 ModelLocalPosition = { 10.0f, -7.0f, 1.0f };
                    objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                        &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
                    objManager.SetAttaching(newObj, m_MainPlayer->m_ObjectRef, "RigRPalm");
                }
                else if (objName == "Shield")
                {
                    XMFLOAT3 ModelLocalRotationEuler = { 12.0f, -96.0f, 93.0f };
                    XMFLOAT3 ModelLocalPosition = { 0.0f, 0.0f, 0.0f };
                    objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                        &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
                    objManager.SetAttaching(newObj, m_MainPlayer->m_ObjectRef, "RigLPalm");
                }
            }
            else if (objName == "ground_grid")
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

    m_MainPlayer->SetCreateSkillObjRef(m_AllObjects, m_WorldObjects, m_MaxWorldObject, m_AllRitems, m_CurrSkillObjInstanceNUM);

    m_nObjCB = (UINT)m_WorldObjects.size() + (UINT)m_UIObjects.size();
    m_nSKinnedCB = (UINT)m_CharacterObjects.size();
    for (auto& ui_obj : m_UIObjects) m_nTextBatch += (UINT)ui_obj->m_UIinfos.size();
}

// character & equipment object generate test
void PlayGameScene::RandomCreateCharacterObject()
{
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
            m_AllRitems["Meshtint Free Knight"].get(),
            m_ModelSkeltons["Meshtint Free Knight"].get(),
            nullptr, &LocalRotationEuler, nullptr,
            &WorldScale, &WorldRotationEuler, &WorldPosition);
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
            std::string objName = "Sword - Instancing" + std::to_string(EquipmentCreatingNum);
            objManager.SetObjectComponent(newEquipmentObj, objName,
                m_AllRitems["Sword"].get(), nullptr,
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
            std::string objName = "Shield - Instancing" + std::to_string(EquipmentCreatingNum++);
            objManager.SetObjectComponent(newEquipmentObj, objName,
                m_AllRitems["Shield"].get(), nullptr,
                &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
            objManager.SetAttaching(newEquipmentObj, newCharacterObj, "RigLPalm");
        }
    }
}

std::vector<UINT8> PlayGameScene::GenerateTextureData()
{
    const UINT rowPitch = 256 * 4;
    const UINT cellPitch = rowPitch >> 4;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = 256 >> 4;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * 256;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += 4)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
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

    float cam_move_factor = 5.0f * gt.GetTimeElapsed();

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
        float camAngle = -90.0f * deg2rad;
        float phi = 20.0f * deg2rad;
        float rad = 1500.0f * 2.0f;
        XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
        LookAtPosition.z += 40.0f * gt.GetTimeElapsed();
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
        m_MainPlayer->ProcessInput(key_state, oldCursorPos,
            ViewPort, gt);
    }
}
