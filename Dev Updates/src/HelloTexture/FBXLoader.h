#pragma once
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct SkinnedVertex
{
    XMFLOAT3 m_xmf3Position;
    XMFLOAT3 m_xmf3Normal;
    XMFLOAT2 m_xmf2TextureUV;
    XMFLOAT3 m_xmf3TangentU;
    XMFLOAT4 m_xmf4BoneWeights;
    XMFLOAT4 m_xmf4Boneids;
    SkinnedVertex()
    {
        m_xmf3Position = m_xmf3Normal = m_xmf3TangentU = { 0.0f, 0.0f, 0.0f };
        m_xmf2TextureUV = { 0.0f, 0.0f };
        m_xmf4BoneWeights = m_xmf4Boneids = { 0.0f, 0.0f, 0.0f, 0.0f };
    }
    SkinnedVertex(SkinnedVertex& other)
    {
        *this = other;
    }
};

struct CSkinnedMesh
{
    UINT m_nvetices = 0;
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<SkinnedVertex>& triangleVertices, std::vector<UINT>& indices);
};

struct Keyframe {
    FbxLongLong mFrameNum;
    FbxAMatrix mGlobalTransform;
    Keyframe* mNext;

    Keyframe() : mNext(nullptr) {}
};

struct Joint {
    int mParentIndex;
    const char* mName;
    FbxAMatrix mGlobalBindposeInverse;
    Keyframe* mAnimation;
    FbxNode* mNode;

    Joint() :
        mNode(nullptr),
        mAnimation(nullptr)
    {
        mGlobalBindposeInverse.SetIdentity();
        mParentIndex = -1;
    }

    ~Joint()
    {
        while (mAnimation)
        {
            Keyframe* temp = mAnimation->mNext;
            delete mAnimation;
            mAnimation = temp;
        }
    }
};

struct Skeleton {
    std::vector<Joint> mJoints;
};

class FBXLoader
{
public:
    FBXLoader();
    ~FBXLoader();

    // fbx에 내장되어 있는 텍스쳐 파일 경로가 절대경로라서 텍스쳐 파일은 LoadTexture()를 통해 별도로 임포트해줘야 한다.
    void LoadFBX(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename);
    void LoadTexture(const char* TGAfilename);

    void Render(ID3D12GraphicsCommandList* pd3dCommandList);

    XMMATRIX GetAnimatedMatrix(int index);

    Skeleton skeleton;
    std::vector<CSkinnedMesh*> pMeshes;
    std::vector<uint8_t> texture_pixels;
    UINT texture_width;
    UINT texture_height;

private:
    FbxManager* fbxsdkManager = nullptr;
    FbxScene* fbxScene;
    std::map<int, int> controlpoints;

    void ProcessNode(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FbxNode* node, FbxGeometryConverter* gConverter);

    CSkinnedMesh* ProcessMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FbxMesh* mesh);

    void ProcessSkeletonHeirarchy(FbxNode* rootnode);

    void ProcessSkeletonHierarchyRecursively(FbxNode* node, int depth, int index, int parentindex);

    size_t FindJointIndex(const std::string& jointname);

};