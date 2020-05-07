#include "stdafx.h"
#include "GameOverScene.h"


GameOverScene::~GameOverScene()
{
}

void GameOverScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    Scene::OnInit(device, commandList,
        BackBufferFormat,
        matCB_index, diffuseSrvHeap_Index,
        objCB_index, skinnedCB_index, textBatch_index);
}

void GameOverScene::OnInitProperties()
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
            animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
        }
    }
    if (m_MainPlayer != nullptr) m_MainPlayer->m_CurrAction = ActionType::Idle;

    m_LightRotationAngle = 0.0f;
}

void GameOverScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt)
{
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt);
}

void GameOverScene::DisposeUploaders()
{
    Scene::DisposeUploaders();
}

void GameOverScene::BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    GeometryGenerator geoGen;

    // UI Position relative to DC(Device Coordinate) with origin at screen center.
    std::unordered_map<std::string, GeometryGenerator::MeshData> Meshes;
    Meshes["UI_Layout_GameOverBackground"] = geoGen.CreateQuad(-640.0f, 360.0f, 1280.0f, 720.0f, 0.0f);
    Meshes["UI_Layout_GameOverInfo"]       = geoGen.CreateQuad(-550.0f,  270.0f, 320.0f, 380.0f, 0.0f);
    Meshes["UI_Layout_GameOverResult"]     = geoGen.CreateQuad(-85.0f,   340.0f, 170.0f, 70.0f,  0.0f);
    Meshes["UI_Layout_ReturnMainMenu"]     = geoGen.CreateQuad(-100.0f, -210.0f, 200.0f, 80.0f,  0.0f);

    m_Geometries["GameOverSceneUIGeo"]
        = std::move(Scene::BuildMeshGeometry(device, commandList, "GameOverSceneUIGeo", Meshes));
}

void GameOverScene::LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    ModelLoader model_loader;

    std::string mesh_path;
    std::vector<std::string> anim_paths;
    std::vector<std::string> execptProcessing_file_nodes;

    mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    anim_paths = { "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);
}

void GameOverScene::LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat)
{
    std::vector<std::string> texture_filepaths =
    {
        "Models/Meshtint Free Knight/Materials/Meshtint Free Knight.tga",
        "UI/Background_SubTitle.png",
        "UI/White_Transparency50.png",
    };

    for (auto& texture_path : texture_filepaths)
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
}

void GameOverScene::BuildMaterials(int& matCB_index, int& diffuseSrvHeap_index)
{
    for (auto& tex_iter : m_Textures)
    {
        auto& tex = tex_iter.second;

        auto mat = std::make_unique<Material>();
        mat->Name = tex->Name;
        mat->NumFramesDirty = gNumFrameResources;
        mat->MatCBIndex = matCB_index++;
        mat->DiffuseSrvHeapIndex = diffuseSrvHeap_index++;
        mat->DiffuseAlbedo = XMFLOAT4(0.88f, 0.88f, 0.88f, 1.0f);
        mat->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
        mat->Roughness = 0.8f;

        m_Materials[mat->Name] = std::move(mat);
    }

    m_nMatCB = (UINT)m_Materials.size();
}

void GameOverScene::BuildRenderItems()
{
    Scene::BuildRenderItem(m_AllRitems, m_Geometries["Meshtint Free Knight"].get());
    for (auto& subMesh : m_Geometries["Meshtint Free Knight"]->DrawArgs)
        m_AllRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

    Scene::BuildRenderItem(m_AllRitems, m_Geometries["GameOverSceneUIGeo"].get());
    for (auto& subMesh_iter : m_Geometries["GameOverSceneUIGeo"]->DrawArgs)
    {
        std::string subMeshName = subMesh_iter.first;
        if(subMeshName.find("Background") != std::string::npos)
            m_AllRitems[subMeshName]->Mat = m_Materials["Background_SubTitle"].get();
        else
            m_AllRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
    }
}

void GameOverScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    ObjectManager objManager;

    m_MainPlayer = std::make_unique<Player>();

    const UINT maxUIObject = (UINT)m_AllRitems.size();
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

            float ConvertModelUnit = ModelFileUnit::meter;
            XMFLOAT3 WorldScale = { ConvertModelUnit, ConvertModelUnit, ConvertModelUnit };
            XMFLOAT3 WorldRotationEuler = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 WorldPosition = { 0.0f, 0.0f, 0.0f };
            XMFLOAT3 LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
            objManager.SetObjectComponent(newObj, objName, Ritem,
                m_ModelSkeltons["Meshtint Free Knight"].get(),
                nullptr, &LocalRotationEuler, nullptr,
                &WorldScale, &WorldRotationEuler, &WorldPosition);
            auto animInfo = newObj->m_SkeletonInfo->m_AnimInfo.get();
            animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
            animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");

            m_MainPlayer->m_ObjectRef = newObj;
        }
        if (Ritem_iter.first.find("UI") != std::string::npos)
        {
            auto newObj = objManager.CreateUIObject(objCB_index++, m_AllObjects, m_UIObjects, maxUIObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            objManager.SetObjectComponent(newObj, objName, Ritem);

            if (objName.find("Background") != std::string::npos) continue;

            newObj->m_UIinfos[objName] = std::make_unique<TextInfo>();
            auto text_info = newObj->m_UIinfos[objName].get();
            text_info->m_FontName = L"���� ����";
            text_info->m_TextColor = DirectX::Colors::Blue;
            auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
            text_info->m_TextPos.x = UI_LayoutPos.x + m_width / 2.0f;
            text_info->m_TextPos.y = m_height / 2.0f - UI_LayoutPos.y;
            text_info->TextBatchIndex = textBatch_index++;

            if (objName == "UI_Layout_GameOverInfo")        text_info->m_Text = L"Game Over Info\n\nTotal Damage,\nTotal Kill,\nTotal Death,\n etc.";
            else if (objName == "UI_Layout_GameOverResult") text_info->m_Text = L"Game Over";
            else if (objName == "UI_Layout_ReturnMainMenu") text_info->m_Text = L"Main Menu";
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

    m_nObjCB = (UINT)m_WorldObjects.size() + (UINT)m_UIObjects.size();
    m_nSKinnedCB = (UINT)m_CharacterObjects.size();
    for (auto& ui_obj : m_UIObjects) m_nTextBatch += (UINT)ui_obj->m_UIinfos.size();
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

void GameOverScene::AnimateSkeletons(CTimer& gt)
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

void GameOverScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt)
{
}
