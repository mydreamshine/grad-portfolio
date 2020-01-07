#include "stdafx.h"
#include "TgaLoader.h"
#include "FBXLoader.h"

FBXLoader::FBXLoader()
{
    fbxsdkManager = FbxManager::Create();

    FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxsdkManager, IOSROOT);
    fbxsdkManager->SetIOSettings(ioSettings);
}


FBXLoader::~FBXLoader()
{
    if (fbxsdkManager != nullptr) fbxsdkManager->Destroy();
}

void FBXLoader::LoadFBX(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename)
{
    if (fbxScene != nullptr) fbxScene->Destroy(true);
    fbxScene = FbxScene::Create(fbxsdkManager, "");

    FbxImporter* importer = FbxImporter::Create(fbxsdkManager, "");
    bool bSuccess = importer->Initialize(filename, -1, fbxsdkManager->GetIOSettings());
    bSuccess = importer->Import(fbxScene);
    importer->Destroy();

    FbxNode* fbxRootNode = fbxScene->GetRootNode();

    ProcessSkeletonHeirarchy(fbxRootNode);

    FbxGeometryConverter gConverter(fbxsdkManager);
    ProcessNode(pd3dDevice, pd3dCommandList, fbxRootNode, &gConverter);
}

void FBXLoader::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    /*for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Render(pd3dCommandList);
    }*/
}

XMMATRIX FBXLoader::GetAnimatedMatrix(int index)
{
    XMMATRIX bonematxm;
    FbxAMatrix bonemat = skeleton.mJoints[index].mGlobalBindposeInverse; //* skeleton.mJoints[0].mAnimation->mGlobalTransform;

    bonematxm = XMMatrixTranslation((float)bonemat.GetT().mData[0], (float)bonemat.GetT().mData[1], (float)bonemat.GetT().mData[2]);
    bonematxm *= XMMatrixRotationX((float)bonemat.GetR().mData[0]);
    bonematxm *= XMMatrixRotationY((float)bonemat.GetR().mData[1]);
    bonematxm *= XMMatrixRotationZ((float)bonemat.GetR().mData[2]);

    return bonematxm;
}

void FBXLoader::ProcessNode(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FbxNode* node, FbxGeometryConverter* gConverter)
{
    if (node)
    {
        if (node->GetNodeAttribute() != nullptr)
        {
            FbxNodeAttribute::EType AttributeType = node->GetNodeAttribute()->GetAttributeType();

            if (AttributeType == FbxNodeAttribute::eMesh)
            {
                FbxMesh* mesh;

                mesh = (FbxMesh*)gConverter->Triangulate(node->GetNodeAttribute(), true);

                pMeshes.push_back(ProcessMesh(pd3dDevice, pd3dCommandList, mesh));
            }
        }

        for (int i = 0; i < node->GetChildCount(); i++)
        {
            ProcessNode(pd3dDevice, pd3dCommandList, node->GetChild(i), gConverter);
        }
    }
}

CSkinnedMesh* FBXLoader::ProcessMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FbxMesh* mesh)
{
    std::vector<SkinnedVertex> meshvertices;
    std::vector<UINT> vertex_indices;
    //vector<uint8_t> texture_Pixels;

    controlpoints.clear();

    FbxVector4* vertices = mesh->GetControlPoints();

    int vertex_index = 0;
    for (int j = 0; j < mesh->GetPolygonCount(); j++)
    {
        int numVertices = mesh->GetPolygonSize(j);

        FbxLayerElementArrayTemplate<FbxVector2>* uvVertices = NULL;
        mesh->GetTextureUV(&uvVertices, FbxLayerElement::eTextureDiffuse);

        for (int k = 0; k < numVertices; k++)
        {
            int controlPointIndex = mesh->GetPolygonVertex(j, k);

            SkinnedVertex vertex;

            vertex.m_xmf3Position.x = (float)vertices[controlPointIndex].mData[0];
            vertex.m_xmf3Position.y = (float)vertices[controlPointIndex].mData[1];
            vertex.m_xmf3Position.z = (float)vertices[controlPointIndex].mData[2];

            vertex.m_xmf2TextureUV.x = (float)uvVertices->GetAt(mesh->GetTextureUVIndex(j, k)).mData[0];
            vertex.m_xmf2TextureUV.y = /*1.0f - */(float)uvVertices->GetAt(mesh->GetTextureUVIndex(j, k)).mData[1];

            // Parsing Normal
            for (int l = 0; l < mesh->GetElementNormalCount(); ++l)
            {
                FbxGeometryElementNormal* leNormal = mesh->GetElementNormal(l);
                if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                {
                    switch (leNormal->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    {
                        FbxVector4 f4Normal = leNormal->GetDirectArray().GetAt(vertex_index);
                        vertex.m_xmf3Normal.x = (float)f4Normal.mData[0];
                        vertex.m_xmf3Normal.y = (float)f4Normal.mData[1];
                        vertex.m_xmf3Normal.z = (float)f4Normal.mData[2];
                        break;
                    }
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leNormal->GetIndexArray().GetAt(vertex_index);
                        FbxVector4 f4Normal = leNormal->GetDirectArray().GetAt(id);
                        vertex.m_xmf3Normal.x = (float)f4Normal.mData[0];
                        vertex.m_xmf3Normal.y = (float)f4Normal.mData[1];
                        vertex.m_xmf3Normal.z = (float)f4Normal.mData[2];
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }
            }

            // Parsing Tangent
            for (int l = 0; l < mesh->GetElementTangentCount(); ++l)
            {
                FbxGeometryElementTangent* leTangent = mesh->GetElementTangent(l);

                if (leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                {
                    switch (leTangent->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    {
                        FbxVector4 f4Tangent = leTangent->GetDirectArray().GetAt(vertex_index);
                        vertex.m_xmf3TangentU.x = (float)f4Tangent.mData[0];
                        vertex.m_xmf3TangentU.y = (float)f4Tangent.mData[1];
                        vertex.m_xmf3TangentU.z = (float)f4Tangent.mData[2];
                        break;
                    }
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leTangent->GetIndexArray().GetAt(vertex_index);
                        FbxVector4 f4Tangent = leTangent->GetDirectArray().GetAt(id);
                        vertex.m_xmf3TangentU.x = (float)f4Tangent.mData[0];
                        vertex.m_xmf3TangentU.y = (float)f4Tangent.mData[1];
                        vertex.m_xmf3TangentU.z = (float)f4Tangent.mData[2];
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }

            }

            controlpoints[controlPointIndex] = (int)meshvertices.size();
            meshvertices.emplace_back(vertex);

            ++vertex_index;
        }
    }

    int materialcount = mesh->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();

    for (int i = 0; i < materialcount; i++)
    {
        FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(i);

        if (material)
        {
            FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

            const FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(0));
            const FbxFileTexture* filetexture = FbxCast<FbxFileTexture>(texture);

            // texture 파일 경로가 절대경로인데다가 확장자가 *.tga가 아닌 경우가 있어서
            // texture 파일은 따로 불러와야 한다.
            // texture가 여러개일 경우에는? 메쉬별로 텍스쳐가 다를 경우?
            // 해당 메쉬가 어떤 텍스쳐 리소스를 쓰는지 전부 파악하고
            // 별도로 텍스쳐'들'을 관리해야 한다.
            //FBXLoader::LoadTexture(filetexture->GetFileName());
            break;
        }
    }

    const FbxVector4 lT = mesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = mesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = mesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);

    FbxAMatrix geometryTransform = FbxAMatrix(lT, lR, lS);

    for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(); ++deformerIndex)
    {
        FbxSkin* skin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
        if (!skin)
            continue;

        for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); ++clusterIndex)
        {
            FbxCluster* cluster = skin->GetCluster(clusterIndex);
            std::string jointname = cluster->GetLink()->GetName();
            size_t jointIndex = FindJointIndex(jointname);
            FbxAMatrix transformMatrix;
            FbxAMatrix transformLinkMatrix;
            FbxAMatrix globalBindposeInverseMatrix;

            cluster->GetTransformMatrix(transformMatrix);
            cluster->GetTransformLinkMatrix(transformLinkMatrix);
            globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

            skeleton.mJoints[jointIndex].mGlobalBindposeInverse = globalBindposeInverseMatrix;
            skeleton.mJoints[jointIndex].mNode = cluster->GetLink();

            for (int i = 0; i < cluster->GetControlPointIndicesCount(); ++i)
            {
                int vertexid = controlpoints[cluster->GetControlPointIndices()[i]];

                if (meshvertices[vertexid].m_xmf4Boneids.x == 0.0f) meshvertices[vertexid].m_xmf4Boneids.x = (float)jointIndex;
                if (meshvertices[vertexid].m_xmf4Boneids.y == 0.0f) meshvertices[vertexid].m_xmf4Boneids.y = (float)jointIndex;
                if (meshvertices[vertexid].m_xmf4Boneids.z == 0.0f) meshvertices[vertexid].m_xmf4Boneids.z = (float)jointIndex;
                if (meshvertices[vertexid].m_xmf4Boneids.w == 0.0f) meshvertices[vertexid].m_xmf4Boneids.w = (float)jointIndex;
                if (meshvertices[vertexid].m_xmf4BoneWeights.x == 0.0f) meshvertices[vertexid].m_xmf4BoneWeights.x = (float)cluster->GetControlPointWeights()[i];
                if (meshvertices[vertexid].m_xmf4BoneWeights.y == 0.0f) meshvertices[vertexid].m_xmf4BoneWeights.y = (float)cluster->GetControlPointWeights()[i];
                if (meshvertices[vertexid].m_xmf4BoneWeights.z == 0.0f) meshvertices[vertexid].m_xmf4BoneWeights.z = (float)cluster->GetControlPointWeights()[i];
                if (meshvertices[vertexid].m_xmf4BoneWeights.w == 0.0f) meshvertices[vertexid].m_xmf4BoneWeights.w = (float)cluster->GetControlPointWeights()[i];
            }

            FbxAnimStack* animstack = fbxScene->GetSrcObject<FbxAnimStack>(0);
            if (animstack == NULL) continue;
            FbxString animstackname = animstack->GetName();
            FbxTakeInfo* takeinfo = fbxScene->GetTakeInfo(animstackname);
            FbxTime start = takeinfo->mLocalTimeSpan.GetStart();
            FbxTime end = takeinfo->mLocalTimeSpan.GetStop();
            FbxLongLong animationlength = end.GetFrameCount(FbxTime::eFrames30) - start.GetFrameCount(FbxTime::eFrames30) + 1;
            Keyframe** anim = &skeleton.mJoints[jointIndex].mAnimation;

            for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames30); i <= end.GetFrameCount(FbxTime::eFrames30); ++i)
            {
                FbxTime time;
                time.SetFrame(i, FbxTime::eFrames30);
                *anim = new Keyframe();
                (*anim)->mFrameNum = i;
                FbxAMatrix transformoffset = mesh->GetNode()->EvaluateGlobalTransform(1000) * geometryTransform;
                (*anim)->mGlobalTransform = transformoffset.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(time);
                anim = &((*anim)->mNext);
            }
        }
    }

    return new CSkinnedMesh(pd3dDevice, pd3dCommandList, meshvertices, vertex_indices);
}

void FBXLoader::ProcessSkeletonHeirarchy(FbxNode* rootnode)
{
    for (int childindex = 0; childindex < rootnode->GetChildCount(); ++childindex)
    {
        FbxNode* node = rootnode->GetChild(childindex);
        ProcessSkeletonHierarchyRecursively(node, 0, 0, -1);
    }
}

void FBXLoader::ProcessSkeletonHierarchyRecursively(FbxNode* node, int depth, int index, int parentindex)
{
    if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        Joint joint;
        joint.mParentIndex = parentindex;
        joint.mName = node->GetName();
        skeleton.mJoints.push_back(joint);
    }
    for (int i = 0; i < node->GetChildCount(); i++)
    {
        ProcessSkeletonHierarchyRecursively(node->GetChild(i), depth + 1, (int)skeleton.mJoints.size(), index);
    }
}

size_t FBXLoader::FindJointIndex(const std::string& jointname)
{
    for (size_t i = 0; i < skeleton.mJoints.size(); ++i)
    {
        if (skeleton.mJoints[i].mName == jointname)
        {
            return i;
        }
    }
    while (true);
    return 0;
}

void FBXLoader::LoadTexture(const char* TGAfilename)
{
    Tga TGA_data(TGAfilename);
    texture_pixels.clear();
    texture_pixels = TGA_data.GetPixels();
    texture_width = TGA_data.GetWidth();
    texture_height = TGA_data.GetHeight();
}

CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<SkinnedVertex>& triangleVertices, std::vector<UINT>& indices)
{
    m_nvetices = (UINT)triangleVertices.size();
    // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
    pd3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(SkinnedVertex) * triangleVertices.size()),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    memcpy(pVertexDataBegin, triangleVertices.data(), sizeof(SkinnedVertex) * triangleVertices.size());
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(SkinnedVertex);
    m_vertexBufferView.SizeInBytes = (UINT)(sizeof(SkinnedVertex) * triangleVertices.size());
}
