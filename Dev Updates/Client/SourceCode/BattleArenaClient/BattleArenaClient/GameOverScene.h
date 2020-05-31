#pragma once
#include "Scene.h"

class GameOverScene : public Scene
{
public:
    GameOverScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~GameOverScene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    virtual void OnInitProperties(CTimer& gt);
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
        const bool key_state[], const POINT& oldCursorPos,
        const RECT& ClientRect,
        CTimer& gt);

public:
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index);

public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void UpdateTextInfo(CTimer& gt) {};
    virtual void AnimateLights(CTimer& gt);
    virtual void AnimateSkeletons(CTimer& gt);
    virtual void AnimateCameras(CTimer& gt);

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt);

private:
    const UINT m_MaxWorldObject = 4;
    const UINT m_MaxCharacterObject = 4;
};