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

    // Estimate the scene bounding sphere manually since we know how the scene was constructed.
    // The grid is the "widest object" with a width of 500 and depth of 500.0f, and centered at
    // the world space origin.  In general, you need to loop over every world space vertex
    // position and compute the bounding sphere.
    m_SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_SceneBounds.Radius = sqrtf(250.0f * 250.0f + 250.0f * 250.0f);
}

void PlayGameScene::OnInitProperties()
{
    for (auto& obj : m_SkinnedObjects)
    {
        if (obj->m_Name == "Meshtint Free Knight")
        {
            obj->NumObjectCBDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(obj->m_WorldTransform), PosM);
            obj->m_TexTransform = MathHelper::Identity4x4();
            obj->m_AnimInfo->AnimStop(obj->m_AnimInfo->CurrPlayingAnimName);
            obj->m_AnimInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
        }
        else if (obj->m_Name == "TT_RTS_Demo_Character")
        {
            obj->NumObjectCBDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(obj->m_WorldTransform), PosM);
            obj->m_TexTransform = MathHelper::Identity4x4();
            obj->m_AnimInfo->AnimStop(obj->m_AnimInfo->CurrPlayingAnimName);
            obj->m_AnimInfo->AnimPlay("infantry_01_idle");
        }
        else if (obj->m_Name == "claire@Dancing")
        {
            obj->NumObjectCBDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, 100.0f);
            XMStoreFloat4x4(&(obj->m_WorldTransform), PosM);
            obj->m_TexTransform = MathHelper::Identity4x4();
            obj->m_AnimInfo->AnimStop(obj->m_AnimInfo->CurrPlayingAnimName);
            obj->m_AnimInfo->AnimPlay("claire@Dancing");
        }
        else if (obj->m_Name == "Soldier_demo")
        {
            obj->NumObjectCBDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, 100.0f);
            XMStoreFloat4x4(&(obj->m_WorldTransform), PosM);
            obj->m_TexTransform = MathHelper::Identity4x4();
            obj->m_AnimInfo->AnimStop(obj->m_AnimInfo->CurrPlayingAnimName);
            obj->m_AnimInfo->AnimPlay("demo_mixamo_idle");
        }
        else
        {
            ObjectManager objManager;
            objManager.DeActivateObj(obj);
        }
    }

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
    Geo_Meshs["ground_grid"] = geoGen.CreateGrid(500.0f, 500.0f, 50, 50);

    m_Geometries["GamePlayGround"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "GamePlayGround", Geo_Meshs));
}

void PlayGameScene::LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    ModelLoader model_loader;
    std::string mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    std::vector<std::string> anim_paths = { "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/ToonyTinyPeople/TT_RTS_Demo_Character.fbx";
    anim_paths = { "Models/ToonyTinyPeople/Animations/infantry_01_idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/claire@Dancing.fbx";
    anim_paths = { "Models/claire@Dancing.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/Soldier_demo/Soldier_demo.fbx";
    anim_paths = { "Models/Soldier_demo/Animations/demo_mixamo_idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);
}

void PlayGameScene::LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat)
{
    std::vector<std::string> material_filepaths =
    {
        "Models/Meshtint Free Knight/Materials/Meshtint Free Knight.tga",
        "Models/ToonyTinyPeople/Materials/TT_RTS_Units_blue.tga",
        "Models/Soldier_demo/Materials/demo_soldier_512.tga",
        "Models/Soldier_demo/Materials/demo_weapon.tga",
        "UI/ui_test.tga"
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
    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Meshtint Free Knight"].get());
    for (auto& subMesh : m_Geometries["Meshtint Free Knight"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["TT_RTS_Demo_Character"].get());
    for (auto& subMesh : m_Geometries["TT_RTS_Demo_Character"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["TT_RTS_Units_blue"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["claire@Dancing"].get());
    for (auto& subMesh : m_Geometries["claire@Dancing"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["checkerboard"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Soldier_demo"].get());
    for (auto& subMesh : m_Geometries["Soldier_demo"]->DrawArgs)
    {
        if (subMesh.first == "Soldier_mesh_0")
            m_AllRitems[subMesh.first]->Mat = m_Materials["demo_weapon"].get();
        else
            m_AllRitems[subMesh.first]->Mat = m_Materials["demo_soldier_512"].get();
    }

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["GamePlayGround"].get());
    for(auto& subMesh : m_Geometries["GamePlayGround"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["checkerboard"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["PlayGameSceneUIGeo"].get());
    for (auto& subMesh : m_Geometries["PlayGameSceneUIGeo"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["ui_test"].get();
}

void PlayGameScene::BuildObjects(int& objCB_index, int& skinnedCB_index)
{
    ObjectManager objManager;

    for (auto& geo_iter : m_Geometries)
    {
        if (geo_iter.first == "Meshtint Free Knight")
        {
            auto newObj = objManager.GenerateSkinnedObject(
                m_AllObjects, m_SkinnedObjects, m_WorldObjects,
                m_MaxSkinnedObject, m_MaxWorldObject);

            newObj->m_Name = geo_iter.first;
            for (auto& subMesh_iter : geo_iter.second->DrawArgs)
                newObj->m_RenderItems[subMesh_iter.first] = m_AllRitems[subMesh_iter.first].get();
            newObj->NumObjectCBDirty = gNumFrameResources;

            newObj->m_Type = ObjectType::CharacterGeo;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(newObj->m_WorldTransform), PosM);
            newObj->ObjCBIndex = objCB_index++;
            newObj->m_Skeleton = m_ModelSkeltons["Meshtint Free Knight"].get();
            newObj->m_AnimInfo = std::make_unique<aiModelData::AnimInfo>();
            newObj->m_AnimInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            newObj->m_AnimInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
            newObj->SkinCBIndex = skinnedCB_index++;

            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
        }
        else if (geo_iter.first == "TT_RTS_Demo_Character")
        {
            auto newObj = objManager.GenerateSkinnedObject(
                m_AllObjects, m_SkinnedObjects, m_WorldObjects,
                m_MaxSkinnedObject, m_MaxWorldObject);

            newObj->m_Name = geo_iter.first;
            for (auto& subMesh_iter : geo_iter.second->DrawArgs)
                newObj->m_RenderItems[subMesh_iter.first] = m_AllRitems[subMesh_iter.first].get();
            newObj->NumObjectCBDirty = gNumFrameResources;

            newObj->m_Type = ObjectType::CharacterGeo;
            XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(newObj->m_WorldTransform), PosM);
            newObj->ObjCBIndex = objCB_index++;
            newObj->m_Skeleton = m_ModelSkeltons["TT_RTS_Demo_Character"].get();
            newObj->m_AnimInfo = std::make_unique<aiModelData::AnimInfo>();
            newObj->m_AnimInfo->AnimPlay("infantry_01_idle");
            newObj->m_AnimInfo->AnimLoop("infantry_01_idle");
            newObj->SkinCBIndex = skinnedCB_index++;

            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
        }
        else if (geo_iter.first == "claire@Dancing")
        {
            auto newObj = objManager.GenerateSkinnedObject(
                m_AllObjects, m_SkinnedObjects, m_WorldObjects,
                m_MaxSkinnedObject, m_MaxWorldObject);

            newObj->m_Name = geo_iter.first;
            for (auto& subMesh_iter : geo_iter.second->DrawArgs)
                newObj->m_RenderItems[subMesh_iter.first] = m_AllRitems[subMesh_iter.first].get();
            newObj->NumObjectCBDirty = gNumFrameResources;

            newObj->m_Type = ObjectType::CharacterGeo;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, 100.0f);
            XMStoreFloat4x4(&(newObj->m_WorldTransform), PosM);
            newObj->ObjCBIndex = objCB_index++;
            newObj->m_Skeleton = m_ModelSkeltons["claire@Dancing"].get();
            newObj->m_AnimInfo = std::make_unique<aiModelData::AnimInfo>();
            newObj->m_AnimInfo->AnimPlay("claire@Dancing");
            newObj->m_AnimInfo->AnimLoop("claire@Dancing");
            newObj->SkinCBIndex = skinnedCB_index++;

            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
        }
        else if (geo_iter.first == "Soldier_demo")
        {
            auto newObj = objManager.GenerateSkinnedObject(
                m_AllObjects, m_SkinnedObjects, m_WorldObjects,
                m_MaxSkinnedObject, m_MaxWorldObject);

            newObj->m_Name = geo_iter.first;
            for (auto& subMesh_iter : geo_iter.second->DrawArgs)
                newObj->m_RenderItems[subMesh_iter.first] = m_AllRitems[subMesh_iter.first].get();
            newObj->NumObjectCBDirty = gNumFrameResources;

            newObj->m_Type = ObjectType::CharacterGeo;
            XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, 100.0f);
            XMStoreFloat4x4(&(newObj->m_WorldTransform), PosM);
            newObj->ObjCBIndex = objCB_index++;
            newObj->m_Skeleton = m_ModelSkeltons["Soldier_demo"].get();
            newObj->m_AnimInfo = std::make_unique<aiModelData::AnimInfo>();
            newObj->m_AnimInfo->AnimPlay("demo_mixamo_idle");
            newObj->m_AnimInfo->AnimLoop("demo_mixamo_idle");
            newObj->SkinCBIndex = skinnedCB_index++;

            m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
        }
        else if (geo_iter.first == "GamePlayGround")
        {
            auto newObj = objManager.GenerateWorldObject(m_AllObjects, m_WorldObjects, m_MaxWorldObject);

            newObj->m_Name = geo_iter.first;
            for (auto& subMesh_iter : geo_iter.second->DrawArgs)
                newObj->m_RenderItems[subMesh_iter.first] = m_AllRitems[subMesh_iter.first].get();
            newObj->NumObjectCBDirty = gNumFrameResources;

            newObj->m_Type = ObjectType::LandGeo;
            XMMATRIX PosM = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
            XMStoreFloat4x4(&(newObj->m_WorldTransform), PosM);
            newObj->ObjCBIndex = objCB_index++;

            m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);
        }
        else if (geo_iter.first == "PlayGameSceneUIGeo")
        {
            auto newObj = std::make_unique<Object>();
            newObj->m_Type = ObjectType::UI;
            newObj->m_Name = geo_iter.first;
            for (auto& subMesh_iter : geo_iter.second->DrawArgs)
                newObj->m_RenderItems[subMesh_iter.first] = m_AllRitems[subMesh_iter.first].get();
            newObj->Activated = true;

            m_ObjRenderLayer[(int)RenderLayer::UIOpaque].push_back(newObj.get());
            m_UIObjects.push_back(newObj.get());
            m_AllObjects.push_back(std::move(newObj));
        }
    }

    const UINT nDeAcativateSkinnedCB = m_MaxSkinnedObject - (UINT)m_SkinnedObjects.size();
    // SkinnedCB를 지닌(Skeleton이 있는) 오브젝트를 생성하면
    // SkinnedCB뿐만 아니라 ObjCB도(WorldObject도) 잡히기 때문에
    // WorldObject보다 적은 SkinnedObject를 먼저 생성해준다.
    for (UINT i = 0; i < nDeAcativateSkinnedCB; ++i)
    {
        auto newObj = objManager.GenerateSkinnedObject(
            m_AllObjects, m_SkinnedObjects, m_WorldObjects,
            m_MaxSkinnedObject, m_MaxWorldObject);

        newObj->NumObjectCBDirty = 0;
        newObj->m_Type = ObjectType::CharacterGeo;
        newObj->m_AnimInfo = std::make_unique<aiModelData::AnimInfo>();
        newObj->ObjCBIndex = objCB_index++;
        newObj->SkinCBIndex = skinnedCB_index++;

        m_ObjRenderLayer[(int)RenderLayer::SkinnedOpaque].push_back(newObj);
    }

    const UINT nDeActivateObjCB = m_MaxWorldObject - (UINT)m_WorldObjects.size();
    for (UINT i = 0; i < nDeActivateObjCB; ++i)
    {
        auto newObj = objManager.GenerateWorldObject(
            m_AllObjects, m_WorldObjects,
            m_MaxWorldObject);

        newObj->NumObjectCBDirty = 0;
        newObj->ObjCBIndex = objCB_index++;

        m_ObjRenderLayer[(int)RenderLayer::Opaque].push_back(newObj);
    }

    for (UINT i = m_MaxSkinnedObject - nDeAcativateSkinnedCB; i < m_MaxSkinnedObject; ++i)
        m_SkinnedObjects[i]->Activated = false;
    for (UINT i = m_MaxWorldObject - nDeActivateObjCB; i < m_MaxWorldObject; ++i)
        m_WorldObjects[i]->Activated = false;

    m_nObjCB = (UINT)m_WorldObjects.size();
    m_nSKinnedCB = (UINT)m_SkinnedObjects.size();
}

void PlayGameScene::RandomCreateSkinnedObject()
{
    ObjectManager objManager;
    auto newObj = objManager.GenerateSkinnedObject(
        m_AllObjects, m_SkinnedObjects, m_WorldObjects,
        m_MaxSkinnedObject, m_MaxWorldObject);

    if (newObj == nullptr) return;

    int rand_i = MathHelper::Rand(0, 3);
    static int CreateNum = 0;

    std::string objName = m_SkinnedObjects[rand_i]->m_Name;
    newObj->m_Name = objName + std::to_string(++CreateNum);
    newObj->m_RenderItems = m_SkinnedObjects[rand_i]->m_RenderItems;
    newObj->NumObjectCBDirty = gNumFrameResources;
    newObj->m_Skeleton = m_SkinnedObjects[rand_i]->m_Skeleton;
    if (objName.find("Meshtint Free Knight") != std::string::npos)
    {
        newObj->m_AnimInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
        newObj->m_AnimInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
    }
    else if (objName.find("TT_RTS_Demo_Character") != std::string::npos)
    {
        newObj->m_AnimInfo->AnimPlay("infantry_01_idle");
        newObj->m_AnimInfo->AnimLoop("infantry_01_idle");
    }
    else if (objName.find("claire@Dancing") != std::string::npos)
    {
        newObj->m_AnimInfo->AnimPlay("claire@Dancing");
        newObj->m_AnimInfo->AnimLoop("claire@Dancing");
    }
    else if (objName.find("Soldier_demo") != std::string::npos)
    {
        newObj->m_AnimInfo->AnimPlay("demo_mixamo_idle");
        newObj->m_AnimInfo->AnimLoop("demo_mixamo_idle");
    }

    float x, y, z;
    x = -200.0f + MathHelper::RandF() * 400.0f;
    y = 0;
    z = -200.0f + MathHelper::RandF() * 400.0f;

    XMMATRIX PosM = XMMatrixTranslation(x, y, z);
    XMStoreFloat4x4(&(newObj->m_WorldTransform), PosM);
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
    m_LightRotationAngle += 1.0f * gt.GetTimeElapsed();

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
    for (auto& obj : m_SkinnedObjects)
    {
        std::string& AnimName = obj->m_AnimInfo->CurrPlayingAnimName;
        bool isSetted = false;
        if (obj->m_AnimInfo->AnimIsPlaying(AnimName, isSetted) == true)
        {
            if (isSetted == false) continue;
            obj->m_Skeleton->UpdateAnimationTransforms(*(obj->m_AnimInfo), gt.GetTimeElapsed());
        }
    }
}

void PlayGameScene::AnimateCameras(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 300.0f, -500.0f };
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}
