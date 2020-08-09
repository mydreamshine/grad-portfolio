#pragma once
#include "Scene.h"
#include "InputTextBox.h"

class LoginScene : public Scene
{
public:
    LoginScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~LoginScene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    virtual void OnInitProperties(CTimer& gt);
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
        const bool key_state[], const POINT& oldCursorPos,
        const RECT& ClientRect,
        CTimer& gt,
        std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index);

public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);
    virtual void AnimateLights(CTimer& gt) {}
    virtual void AnimateSkeletons(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents) {}
    virtual void AnimateCameras(CTimer& gt);
    void UpdateUITransform(CTimer& gt);

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

private:
    const UINT m_MaxTextObject = 4;

    size_t m_CurrBackgroundLayerIndex = 0;
    std::vector<RenderItem*> m_BackgroundLayers;

private:
    const float InputBoxMaxWidth = 120.0f;
    InputTextBox inputTextBox_ID;
    InputTextBox inputTextBox_Password;

    bool LoginButtonPress = false;
    bool LoginButtonUp = true;

    bool ID_InputBoxSelected = false;
    bool InputBoxSelected = false;
    bool InputBoxSelectChange = false;

    bool BackgroundLayeT1IsFront = true;
    bool BackgroundLayerTransforming = false;

    const float BackgroundLayerTransformingDoneTime = 1.0f;
    const float BackgroundLayerTransformInterval = 3.0f;

    float BackgroundLayerTransformingTimeStack = 0.0f;
    float BackgroundLayerTransformIntervalTimeStack = 0.0f;

    float BackgroundLayerAlphaIncreaseSign = 1.0f;

    bool OnceSendLogin = false;
};