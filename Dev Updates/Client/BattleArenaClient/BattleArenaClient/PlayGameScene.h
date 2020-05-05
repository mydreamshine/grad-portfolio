#pragma once
#include "Scene.h"
#include "Player.h"

class PlayGameScene : public Scene
{
public:
    PlayGameScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~PlayGameScene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        DXGI_FORMAT BackBufferFormat,
        int& matCB_index, int& diffuseSrvHeap_Index,
        int& objCB_index, int& skinnedCB_index);
    virtual void OnInitProperties();
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map, CTimer& gt);
    virtual void DisposeUploaders();

public:
    virtual void BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    virtual void LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    virtual void LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat);
    virtual void BuildMaterials(int& matCB_index, int& diffuseSrvHeap_Index);
    virtual void BuildRenderItems();
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index);

    virtual void RandomCreateCharacterObject();

private:
    std::vector<UINT8> GenerateTextureData();

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
    void AnimateWorldObjectsTransform(CTimer& gt);

public:
    virtual void ProcessInput(CTimer& gt);

private:
    const UINT m_MaxWorldObject = 2000;
    const UINT m_MaxCharacterObject = 100;
    UINT m_CurrSkillObjInstanceNUM = 0;
    DirectX::XMFLOAT3 m_WorldCenter = { 0.0f, 0.0f, 0.0f };
    DirectX::BoundingBox m_SpawnBound;

    std::unique_ptr<Player> m_MainPlayer = nullptr;

private:
    /// 런타임 중에 VK_LEFT, RIGHT, UP, DOWN 키와,
    // x,y,z키를 가지고 오브젝트의 LocalTransform을 
    // 지정해준다. 거시적으로 적당하게 오브젝트가 배치된 것
    // 같으면 중단점을 걸어 local_angle과 local_pos 값을 확인한다.
    void ControlSetObjectLocalScaleAndTranslation(Object* obj)
    {
        static XMFLOAT3 local_angle = { 0.0f, 0.0f, 0.0f };
        static XMFLOAT3 local_pos = { 0.0f, 0.0f, 0.0f };

        //x
        if (GetAsyncKeyState(0x58) && 0x8000)
        {
            if (GetAsyncKeyState(VK_UP) && 0x8000)
                local_angle.x += 1.0f;
            else if (GetAsyncKeyState(VK_DOWN) && 0x8000)
                local_angle.x -= 1.0f;

            if (GetAsyncKeyState(VK_LEFT) && 0x8000)
                local_pos.x -= 1.0f;
            else if (GetAsyncKeyState(VK_RIGHT) && 0x8000)
                local_pos.x += 1.0f;
        }
        //y
        if (GetAsyncKeyState(0x59) && 0x8000)
        {
            if (GetAsyncKeyState(VK_UP) && 0x8000)
                local_angle.y += 1.0f;
            else if (GetAsyncKeyState(VK_DOWN) && 0x8000)
                local_angle.y -= 1.0f;

            if (GetAsyncKeyState(VK_LEFT) && 0x8000)
                local_pos.y -= 1.0f;
            else if (GetAsyncKeyState(VK_RIGHT) && 0x8000)
                local_pos.y += 1.0f;
        }
        //z
        if (GetAsyncKeyState(0x5A) && 0x8000)
        {
            if (GetAsyncKeyState(VK_UP) && 0x8000)
                local_angle.z += 1.0f;
            else if (GetAsyncKeyState(VK_DOWN) && 0x8000)
                local_angle.z -= 1.0f;

            if (GetAsyncKeyState(VK_LEFT) && 0x8000)
                local_pos.z -= 1.0f;
            else if (GetAsyncKeyState(VK_RIGHT) && 0x8000)
                local_pos.z += 1.0f;
        }
        obj->m_TransformInfo->SetLocalRotationEuler(local_angle);
        obj->m_TransformInfo->SetLocalPosition(local_pos);
    }
};
