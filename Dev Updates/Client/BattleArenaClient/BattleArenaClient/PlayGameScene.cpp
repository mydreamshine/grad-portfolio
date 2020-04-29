#include "stdafx.h"
#include "PlayGameScene.h"

PlayGameScene::~PlayGameScene()
{
}

void PlayGameScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index, int& objCB_index, int& skinnedCB_index)
{
    Scene::OnInit(device, commandList,
        BackBufferFormat,
        matCB_index, diffuseSrvHeap_Index, objCB_index, skinnedCB_index);

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
            auto objInfo = obj->m_ObjectInfo.get();
            objInfo->NumObjectCBDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(objInfo->m_WorldTransform), PosM);
            objInfo->m_TexTransform = MathHelper::Identity4x4();

            auto skeletonInfo = obj->m_SkeletonInfo.get();
            skeletonInfo->m_AnimInfo->AnimStop(skeletonInfo->m_AnimInfo->CurrPlayingAnimName);
            skeletonInfo->m_AnimInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
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

    ///////////// alphaTest /////////////
    {
        for (auto& obj : m_UIObjects)
            obj->m_ObjectInfo->m_TexAlpha = 1.0f;
    }
    /////////////////////////////////////

    XMFLOAT3 EyePosition = { 0.0f, 300.0f, -500.0f };
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();

    m_LightRotationAngle = 0.0f;
    m_BaseLightDirections[0] = m_BaseLightDirections[0] = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
    m_BaseLightDirections[1] = m_BaseLightDirections[1] = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
    m_BaseLightDirections[2] = m_BaseLightDirections[2] = XMFLOAT3(0.0f, -0.707f, -0.707f);
}

void PlayGameScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::OnUpdate(frame_resource, shadow_map, gt);
}

void PlayGameScene::DisposeUploaders()
{
    Scene::DisposeUploaders();
}

void PlayGameScene::BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    GeometryGenerator geoGen;

    float wnd_width = (float)m_width;
    float wnd_height = (float)m_height;
    float wnd_top = wnd_height / 2;
    float wnd_bottom = -wnd_top;
    float wnd_right = wnd_width / 2;
    float wnd_left = -wnd_right;

    // UI Position relative to DC(Device Coordinate) with origin at screen center.
    std::unordered_map<std::string, GeometryGenerator::MeshData> UI_Meshes;
    UI_Meshes["UI_quad1"] = geoGen.CreateQuad(wnd_left + 10.0f, wnd_top - 10.0f, 400.0f, 150.0f, 0.0f);
    UI_Meshes["UI_quad2"] = geoGen.CreateQuad(wnd_right - 410.0f, wnd_top - 10.0f, 400.0f, 150.0f, 0.0f);
    UI_Meshes["UI_quad3"] = geoGen.CreateQuad(-200.0f, wnd_bottom + 160.0f, 400.0f, 150.0f, 0.0f);

    m_Geometries["PlayGameSceneUIGeo"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "PlayGameSceneUIGeo", UI_Meshes));

    std::unordered_map<std::string, GeometryGenerator::MeshData> Geo_Meshs;
    Geo_Meshs["ground_grid"] = geoGen.CreateGrid(10000.0f, 10000.0f, 100, 100);

    m_Geometries["GamePlayGround"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "GamePlayGround", Geo_Meshs));
}

void PlayGameScene::LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    ModelLoader model_loader;

    std::string mesh_path = "Models/Environment/Environment.fbx";
    std::vector<std::string> anim_paths;
    std::vector<std::string> execptProcessing_file_nodes = { "Environment_root", "RootNode" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);

    mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    anim_paths = { "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx" };
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
        "UI/ui_test.png"
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
    for (auto& subMesh : m_Geometries["PlayGameSceneUIGeo"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["ui_test"].get();
}

void PlayGameScene::BuildObjects(int& objCB_index, int& skinnedCB_index)
{
    ObjectManager objManager;

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

            // 스케일을 2로 늘려줘야 거시적으로
            // 주변 사물과의 크기 비례가 어색하지 않게 된다.
            float ConvertModelUnit = /*ModelFineUnit::centimeter * 2.0f*/ 2.0f;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 17.86f, 0.0f, 0.0f };
            objManager.SetObjectComponent(newObj, objName, Ritem,
                m_ModelSkeltons["Meshtint Free Knight"].get(),
                nullptr, nullptr, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);
            newObj->m_SkeletonInfo->m_AnimInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            newObj->m_SkeletonInfo->m_AnimInfo->AnimLoop("Meshtint Free Knight@Battle Idle");

            m_MainPlayerObj = newObj;
        }
        else if(Ritem_iter.first.find("UI") != std::string::npos)
        {
            const UINT maxUIObject = (UINT)m_Geometries["PlayGameSceneUIGeo"]->DrawArgs.size();
            auto newObj = objManager.CreateUIObject(objCB_index++, m_AllObjects, m_UIObjects, maxUIObject);
            m_ObjRenderLayer[(int)RenderLayer::UI].push_back(newObj);

            // UI 오브젝트 같은 경우에는 메쉬의 원점이 (0,0,0)이 아니라
            // 이미 버텍스 자체가 스크린좌표계 기준으로 지정되어 있기 때문에
            // UI 오브젝트의 Transform에 대해선 따로 지정하지 않아도 된다.
            std::string objName = Ritem_iter.first;
            objManager.SetObjectComponent(newObj, objName, Ritem);
        }
        else // Environment Objects + Equipment Object
        {
            auto newObj = objManager.CreateWorldObject(objCB_index++, m_AllObjects, m_WorldObjects, m_MaxWorldObject);
            m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);

            std::string objName = Ritem_iter.first;

            // 장비 아이템은 어차피 캐릭터 오브젝트에
            // Attaching 할 거라 WorldTransform을 지정하지 않아도 된다.
            if (objName == "Sword" || objName == "Shield")
            {
                XMFLOAT3 ModelLocalScale = { 1.0f, 1.0f, 1.0f };
                if (objName == "Sword")
                {
                    XMFLOAT3 ModelLocalRotationEuler = { 1.0f, 3.0f, 83.0f };
                    XMFLOAT3 ModelLocalPosition = { 10.0f, -7.0f, 1.0f };
                    objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                        &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
                    objManager.SetAttaching(newObj, m_MainPlayerObj, "RigRPalm");
                }
                else if (objName == "Shield")
                {
                    XMFLOAT3 ModelLocalRotationEuler = { 12.0f, -96.0f, 93.0f };
                    XMFLOAT3 ModelLocalPosition = { 0.0f, 0.0f, 0.0f };
                    objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                        &ModelLocalScale, &ModelLocalRotationEuler, &ModelLocalPosition);
                    objManager.SetAttaching(newObj, m_MainPlayerObj, "RigLPalm");
                }
            }
            else if (objName == "ground_grid")
            {
                XMFLOAT3 WorldPosition = { 0.0f, 3.0f, 0.0f };
                objManager.SetObjectComponent(newObj, objName, Ritem, nullptr,
                    nullptr, nullptr, nullptr,
                    nullptr, nullptr, &WorldPosition);
            }
            else // Environment Object
            {
                // 환경 사물 같은 경우에는 메쉬의 원점이 (0,0,0)이 아니라
                // 이미 파일 자체에서 메쉬가 배치되어 있는 상태라서
                // 환경 사물의 Transform에 대해선 따로 지정하지 않아도 된다.
                objManager.SetObjectComponent(newObj, objName, Ritem);

                if(objName == "Floor1")
                {
                    auto& FloorBound = newObj->m_ObjectInfo->m_Bound;
                    float width = FloorBound.Extents.x;
                    float depth = FloorBound.Extents.z;

                    m_SceneBounds.Center = m_WorldCenter = FloorBound.Center;
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

        for (UINT i = m_MaxCharacterObject - nDeAcativateCharacterObj; i < m_MaxCharacterObject; ++i)
            m_CharacterObjects[i]->Activated = false;
        for (UINT i = m_MaxWorldObject - nDeAcativateWorldObj; i < m_MaxWorldObject; ++i)
            m_WorldObjects[i]->Activated = false;
    }

    m_nObjCB = (UINT)m_WorldObjects.size() + (UINT)m_UIObjects.size();
    m_nSKinnedCB = (UINT)m_CharacterObjects.size();
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

        // 스케일을 2로 늘려줘야 거시적으로
        // 주변 사물과의 크기 비례가 어색하지 않게 된다.
        float ConvertModelUnit = /*ModelFineUnit::centimeter * 2.0f*/ 2.0f;
        XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
        XMFLOAT3 WorldRotationEuler = { 0.0f, MathHelper::RandF(0.0f, 360.0f), 0.0f };
        XMFLOAT3 WorldPosition =
        {
            MathHelper::RandF(-m_SpawnBound.Extents.x, m_SpawnBound.Extents.x),
            0.0f,
            MathHelper::RandF(-m_SpawnBound.Extents.z, m_SpawnBound.Extents.z),
        };
        std::string objName = "Meshtint Free Knight - Instancing" + std::to_string(CharacterCreatingNum++);
        objManager.SetObjectComponent(newCharacterObj, objName,
            m_AllRitems["Meshtint Free Knight"].get(),
            m_ModelSkeltons["Meshtint Free Knight"].get(),
            nullptr, nullptr, nullptr,
            &WorldScale, &WorldRotationEuler, &WorldPosition);
        newCharacterObj->m_SkeletonInfo->m_AnimInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
        newCharacterObj->m_SkeletonInfo->m_AnimInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
    }

    // equipment gen test
    {
        static int EquipmentCreatingNum = 1;
        XMFLOAT3 ModelLocalScale = { 1.0f, 1.0f, 1.0f };
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
    /////////////////////////////////////////////// alphaTest ///////////////////////////////////////////////
    static float sign = -1.0f;
    for (auto& obj : m_UIObjects)
    {
        obj->m_ObjectInfo->NumObjectCBDirty = gNumFrameResources;
        float& TexAlpha = obj->m_ObjectInfo->m_TexAlpha;
        TexAlpha += 0.01f * sign;
        sign = (TexAlpha < 0.0f || TexAlpha > 1.0f) ? sign * -1.0f : sign;
        TexAlpha = (TexAlpha < 0.0f ? 0.0f : (TexAlpha > 1.0f ? 1.0f : TexAlpha));
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    m_LightRotationAngle += (10.0f * deg2rad) * gt.GetTimeElapsed();

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
                    auto parentObjInfo = obj->m_ObjectInfo.get();
                    auto childObjInfo = child_obj->m_ObjectInfo.get();
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
                        // LocalTransform * AnimTransform * WorldTransform
                        // Attaching을 한 오브젝트는
                        // 해당 오브젝트가 Skeleton을 지니고 있는 것이 아니기 때문에
                        // VS쉐이더 단계에서 animTransform을 계산하는 과정이 생략된다
                        // 고로, Attaching 오브젝트의 애니메이션을 위해선
                        // Attaching 오브젝트의 WorldTransform이
                        // AnimTransform과 WorldTransform을 포함한 행렬이어야 한다.
                        // 이때 WorldTransform은 해당 오브젝트의 WorldTransform이 아닌
                        // Attaching을 당한 오브젝트(장비 오브젝트 기준으론 캐릭터 오브젝트)
                        // 의 WorldTransform을 취해준다.
                        XMFLOAT4X4 WorldTransform = parentObjInfo->GetWorldTransform();
                        XMMATRIX WorldM = XMLoadFloat4x4(&WorldTransform);
                        XMMATRIX AnimWorldM = AnimM * WorldM;

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
    float deg2rad = MathHelper::Pi / 180.0f;
    static float camAngle = -90.0f * deg2rad;
    camAngle += (15.0f * deg2rad) * gt.GetTimeElapsed();

    XMFLOAT3 LookTargetWorldScale = m_MainPlayerObj->m_ObjectInfo->m_WorldScale;
    XMFLOAT3 LookAtPosition = m_MainPlayerObj->m_ObjectInfo->m_WorldPosition;
    float Scale_average = (LookTargetWorldScale.x + LookTargetWorldScale.y + LookTargetWorldScale.z) * 0.3333f;
    float phi = 20.0f * deg2rad;
    float rad = 1500.0f * Scale_average;
    XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
    Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
    XMFLOAT3 EyePosition;
    XMStoreFloat3(&EyePosition, Eye_Pos);
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 10000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}
