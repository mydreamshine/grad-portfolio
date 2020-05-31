#pragma once
#include "Scene.h"

class PlayGameScene : public Scene
{
public:
    PlayGameScene(UINT width, UINT height) : Scene(width, height) {}
    virtual ~PlayGameScene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    virtual void OnInitProperties(CTimer& gt);
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
        const bool key_state[], const POINT& oldCursorPos,
        const RECT& ClientRect,
        CTimer& gt);

public:
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    void RandomCreateCharacterObject();

public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void UpdateTextInfo(CTimer& gt);
    virtual void AnimateLights(CTimer& gt);
    virtual void AnimateSkeletons(CTimer& gt);
    virtual void AnimateCameras(CTimer& gt);
    void AnimateWorldObjectsTransform(CTimer& gt);

public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt);
    void ProcessCollision(CTimer& gt);

private:
    const UINT m_MaxWorldObject = MAX_WORLD_OBJECT;
    const UINT m_MaxCharacterObject = MAX_CHARACTER_OBJECT;
    UINT m_CurrSkillObjInstanceNUM = 0;
    DirectX::XMFLOAT3 m_WorldCenter = { 0.0f, 0.0f, 0.0f };
    DirectX::BoundingBox m_SpawnBound;

    std::unique_ptr<Player> m_MainPlayer = nullptr;

    // Time Limit Info
    float SceneStartTime = 0.0f;
    int sec = 0;
    int sec2 = 0;
    std::wstring time_str = L"Time Limit\n   03:00";
    UINT TimeLimit_sec = 180;

private:
    /// ��Ÿ�� �߿� VK_LEFT, RIGHT, UP, DOWN Ű��,
    // x,y,zŰ�� ������ ������Ʈ�� LocalTransform�� 
    // �������ش�. �Ž������� �����ϰ� ������Ʈ�� ��ġ�� ��
    // ������ �ߴ����� �ɾ� local_angle�� local_pos ���� Ȯ���Ѵ�.
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