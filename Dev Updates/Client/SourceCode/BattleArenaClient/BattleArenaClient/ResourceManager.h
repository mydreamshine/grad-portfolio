#pragma once
#include <unordered_map>
#include <memory>

#include "Common/FileLoader/TextureLoader.h"
#include "Common/FileLoader/ModelLoader.h"
#include "Common/Util/d3d12/GeometryGenerator.h"
#include "FrameResource.h"

enum class RenderTargetScene : int
{
    LoginScene = 0,
    LobyScene,
    PlayGameScene,
    GameOverScene,
    Count
};

class ResourceManager
{
public:
	ResourceManager() = default;
	~ResourceManager();

    void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        DXGI_FORMAT BackBufferFormat,
        int& matCB_index, int& diffuseSrvHeap_Index);
    void DisposeUploaders();

public:
    std::unique_ptr<MeshGeometry> BuildMeshGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        const std::string& geoName, std::unordered_map<std::string, GeometryGenerator::MeshData>& Meshes);
    void BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

    void LoadSkinnedModelData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        ModelLoader& model_loader,
        const std::string& mesh_filepath, const std::vector<std::string>& anim_filepaths,
        std::vector<std::string>* execpt_nodes = nullptr);
    void LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

    std::vector<UINT8> GenerateTexture_chechboardPattern();
    void LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat);
    void BuildMaterials(int& matCB_index, int& diffuseSrvHeap_index);

    // Material에 대한 지정은 이 함수 외부에서 지정해줘야 한다.
    // (Material의 이름을 기준으로 RenderItem에 매칭시키기 때문.) => 이름으론 Material 매칭 패턴을 찾을 수가 없다.
    void BuildRenderItem(std::unordered_map<std::string, std::unique_ptr<RenderItem>>& GenDestList, MeshGeometry* Geo);
    void BuildRenderItems();

public:
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& GetGeometries() { return m_Geometries; }
    std::unordered_map<std::string, std::unique_ptr<Material>>& GetMaterials() { return m_Materials; }
    std::unordered_map<std::string, std::unique_ptr<Texture>>& GetTextures() { return m_Textures; }
    std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>>& GetModelSkeltons() { return m_ModelSkeltons; }

    std::unordered_map<std::string, std::unique_ptr<RenderItem>>& GetRenderItems(RenderTargetScene TargetScene) { return m_AllRitems[(int)TargetScene]; }

private:
    // Original Resource
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
    std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>> m_ModelSkeltons;

    // List of all render item.
    std::unordered_map<std::string, std::unique_ptr<RenderItem>> m_AllRitems[(int)RenderTargetScene::Count];

public:
    UINT m_nMatCB = 0; // = m_Materials.size();
};