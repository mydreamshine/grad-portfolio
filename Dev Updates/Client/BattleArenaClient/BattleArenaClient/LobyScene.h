#pragma once
#include "Scene.h"

class LobyScene : public Scene
{
public:
    LobyScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~LobyScene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        DXGI_FORMAT BackBufferFormat,
        int& matCB_index, int& diffuseSrvHeap_Index,
        int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    virtual void OnInitProperties();
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
        const bool key_state[], const POINT& oldCursorPos,
        const RECT& ClientRect,
        CTimer& gt);
    virtual void DisposeUploaders();

public:
    virtual void BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    virtual void LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    virtual void LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat);
    virtual void BuildMaterials(int& matCB_index, int& diffuseSrvHeap_Index);
    virtual void BuildRenderItems();
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index);

public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void AnimateLights(CTimer& gt);
    virtual void AnimateSkeletons(CTimer& gt);
    virtual void AnimateCameras(CTimer& gt);

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt);

private:
    const UINT m_MaxWorldObject = 4;
    const UINT m_MaxCharacterObject = 4;

    std::unique_ptr<Player> m_MainPlayer = nullptr;
};