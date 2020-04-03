#include "ModelLoader.h"

#include <stdexcept>

const aiScene* ModelLoader::loadScene(std::string filepath)
{
	const aiScene* pScene = mImporter.ReadFile(filepath,
		aiProcess_JoinIdenticalVertices |     // ������ ������ ����, �ε��� ����ȭ
		aiProcess_ValidateDataStructure |     // �δ��� ����� ����
		aiProcess_ImproveCacheLocality |      // ��� ������ ĳ����ġ�� ����
		aiProcess_RemoveRedundantMaterials |  // �ߺ��� ���͸��� ����
		aiProcess_GenUVCoords |               // ����, ������, ���� �� ��� ������ ������ UV�� ��ȯ
		aiProcess_TransformUVCoords |         // UV ��ȯ ó���� (�����ϸ�, ��ȯ...)
		aiProcess_FindInstances |             // �ν��Ͻ��� �Ž��� �˻��Ͽ� �ϳ��� �����Ϳ� ���� ������ ����
		aiProcess_LimitBoneWeights |          // ������ ���� ����ġ�� �ִ� 4���� ����
		aiProcess_OptimizeMeshes |            // ������ ��� ���� �Ž��� ����
		aiProcess_GenSmoothNormals |          // �ε巯�� �븻����(��������) ����
		aiProcess_SplitLargeMeshes |          // �Ŵ��� �ϳ��� �Ž��� �����Ž���� ��Ȱ(����)
		aiProcess_Triangulate |               // 3�� �̻��� �𼭸��� ���� �ٰ��� ���� �ﰢ������ ����(����)
		aiProcess_ConvertToLeftHanded |       // D3D�� �޼���ǥ��� ��ȯ
		aiProcess_SortByPType);
	return pScene;
}

bool ModelLoader::loadMeshAndSkeleton(std::string filepath)
{
	const aiScene* pScene = ModelLoader::loadScene(filepath);

	if (pScene == NULL)
		return false;

	char _Drive[_MAX_DRIVE];
	char _Dir[_MAX_DIR];
	char _Filename[_MAX_FNAME];
	char _Ext[_MAX_EXT];
	_splitpath_s(filepath.c_str(), _Drive, _Dir, _Filename, _Ext);

	if (!mMeshes.empty()) mMeshes.clear();
	if (mSkeleton != nullptr) mSkeleton = nullptr;

	ModelLoader::generateSkeleton(pScene->mRootNode, pScene, aiMatrix4x4(), _Filename);
	ModelLoader::processNode(pScene->mRootNode, pScene, aiMatrix4x4());

	return true;
}

bool ModelLoader::loadAnimation(std::string filepath)
{
	const aiScene* pScene = ModelLoader::loadScene(filepath);

	if (pScene == NULL)
		return false;

	char _Drive[_MAX_DRIVE];
	char _Dir[_MAX_DIR];
	char _Filename[_MAX_FNAME];
	char _Ext[_MAX_EXT];
	_splitpath_s(filepath.c_str(), _Drive, _Dir, _Filename, _Ext);

	ModelLoader::generateSkeleton(pScene->mRootNode, pScene, aiMatrix4x4(), _Filename);
	ModelLoader::getAnimation_0(pScene, _Filename);

	return true;
}

aiModelData::aiMesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 matrix, aiNode* parentNode)
{
	std::vector<aiModelData::aiVertex> Vertices;
	std::vector<std::uint32_t> Indices;
	aiNode* parentBoneNode = parentNode;
	int parentBoneID = mSkeleton->FindJointIndex(parentBoneNode->mName.C_Str());
	aiMatrix4x4 BindPoseTransform = matrix;

	// read mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		aiModelData::aiVertex vertex;

		vertex.vPosition = BindPoseTransform * mesh->mVertices[i];;

		if (mesh->mNormals) vertex.vNormal = BindPoseTransform * mesh->mNormals[i];
		if (mesh->mTangents) vertex.vTangent = BindPoseTransform * mesh->mTangents[i];
		if (mesh->mBitangents) vertex.vBitangent = BindPoseTransform * mesh->mBitangents[i];
		if (mesh->mTextureCoords[0])
		{
			vertex.vTextureUV[0].x = (float)mesh->mTextureCoords[0][i].x;
			vertex.vTextureUV[0].y = 1.0f - (float)mesh->mTextureCoords[0][i].y;
		}

		// static Mesh
		if (parentBoneNode && mesh->mBones == nullptr)
		{
			if (parentBoneID != -1)
			{
				// �ִϸ��̼ǿ� ���ε��Ǳ� ���� BoneID�� BoneWeights�� �ο��Ѵ�.
				vertex.i32BoneIndices[0] = parentBoneID;
				vertex.fBoneWeights[0] = 1.0f;
			}
		}

		Vertices.push_back(vertex);
	}

	if (mesh->mBones == nullptr) // Static Mesh
	{
		// Static Mesh�� ���� �ִϸ��̼��� ����
		// Static Mesh�� �θ� ���뿡 OffsetTransform(BindPoseInverse) �ο�
		if (parentBoneNode)
		{
			if (parentBoneID != -1)
			{
				aiMatrix4x4 bindPose = BindPoseTransform; // BindPoseTransform.Inverse()�� �ϸ� BindPoseTransform��ü�� Inverse�ȴ�.
				mSkeleton->mJoints[parentBoneID].mOffsetTransform = bindPose.Inverse();
			}
		}
	}
	else
		ModelLoader::processBoneWeights(Vertices, mesh->mBones, mesh->mNumBones, BindPoseTransform);

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			Indices.push_back(face.mIndices[j]);
	}

	return aiModelData::aiMesh(mesh->mName.C_Str(), Vertices, Indices);
}

void ModelLoader::processBoneWeights(std::vector<aiModelData::aiVertex>& pOut, aiBone** Bones_intoMesh, unsigned int numBone, aiMatrix4x4 BindPoseTransform)
{
	for (unsigned int BoneID_intoMesh = 0; BoneID_intoMesh < numBone; ++BoneID_intoMesh)
	{
		for (unsigned int weightID = 0; weightID < Bones_intoMesh[BoneID_intoMesh]->mNumWeights; ++weightID)
		{
			aiVertexWeight* BoneWeights = Bones_intoMesh[BoneID_intoMesh]->mWeights;
			aiModelData::aiVertex& vertex = pOut[BoneWeights[weightID].mVertexId];

			int BoneID = mSkeleton->FindJointIndex(Bones_intoMesh[BoneID_intoMesh]->mName.C_Str());
			if (BoneID >= 0)
			{
				aiModelData::aiJoint& Joint = mSkeleton->mJoints[BoneID];
				// Offset Transform == BindPoseInverse
				if (Bones_intoMesh[BoneID_intoMesh]->mOffsetMatrix.IsIdentity() != true)
				{
					if(Joint.mOffsetTransform.IsIdentity())
						Joint.mOffsetTransform = Bones_intoMesh[BoneID_intoMesh]->mOffsetMatrix/* * BindPoseTransform.Inverse()*/;
				}

				if (vertex.i32BoneIndices[0] == -1) vertex.i32BoneIndices[0] = BoneID;
				else if (vertex.i32BoneIndices[1] == -1) vertex.i32BoneIndices[1] = BoneID;
				else if (vertex.i32BoneIndices[2] == -1) vertex.i32BoneIndices[2] = BoneID;
				else if (vertex.i32BoneIndices[3] == -1) vertex.i32BoneIndices[3] = BoneID;
				else throw std::invalid_argument("The vertex has exceeded the maximum number of BoneIDs it can have.");
				if (vertex.fBoneWeights[0] == 0.0f) vertex.fBoneWeights[0] = BoneWeights[weightID].mWeight;
				else if (vertex.fBoneWeights[1] == 0.0f) vertex.fBoneWeights[1] = BoneWeights[weightID].mWeight;
				else if (vertex.fBoneWeights[2] == 0.0f) vertex.fBoneWeights[2] = BoneWeights[weightID].mWeight;
				else if (vertex.fBoneWeights[3] == 0.0f) vertex.fBoneWeights[3] = BoneWeights[weightID].mWeight;
				else throw std::invalid_argument("The vertex has exceeded the maximum number of BoneWeights it can have.");
			}
		}
	}
}

void ModelLoader::generateSkeleton(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix, const std::string& name)
{
	std::string cs_NodeName = node->mName.C_Str();
	aiMatrix4x4 curMat = matrix * node->mTransformation;

	if (node->mNumMeshes == 0) // ���� ����� ���
	{
		if (mSkeleton == nullptr)
		{
			mSkeleton = std::make_unique<aiModelData::aiSkeleton>();
			mSkeleton->mName = name;
		}

		if (mSkeleton->FindJointIndex(cs_NodeName) == -1)
		{
			aiModelData::aiJoint joint;
			joint.mName = cs_NodeName;
			if (node->mParent) joint.mParentIndex = mSkeleton->FindJointIndex(node->mParent->mName.C_Str());
			joint.mLocalTransform = node->mTransformation;
			aiMatrix4x4 globalM = curMat;
			joint.mOffsetTransform = globalM.Inverse();

			mSkeleton->mJoints.push_back(joint);
		}
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		ModelLoader::generateSkeleton(node->mChildren[i], scene, curMat, name);
}

void ModelLoader::getAnimation_0(const aiScene* scene, const std::string& anim_name)
{
	if (scene->mAnimations)
	{
		for (unsigned int i = 0; i < scene->mAnimations[0]->mNumChannels; ++i)
		{
			aiNodeAnim* animNode = scene->mAnimations[0]->mChannels[i];
			int BoneID = mSkeleton->FindJointIndex(animNode->mNodeName.C_Str());
			if (BoneID != -1)
			{
				aiModelData::aiAnimClip newAnimClip;
				newAnimClip.mName = scene->mAnimations[0]->mName.C_Str();
				newAnimClip.mDuration = (float)scene->mAnimations[0]->mDuration;
				newAnimClip.mTickPerSecond = (float)scene->mAnimations[0]->mTicksPerSecond;
				newAnimClip.mScalingKeys.reserve(animNode->mNumScalingKeys);
				newAnimClip.mRotationKeys.reserve(animNode->mNumRotationKeys);
				newAnimClip.mPositionKeys.reserve(animNode->mNumPositionKeys);

				for (unsigned int j = 0; j < animNode->mNumPositionKeys; ++j)
					newAnimClip.mPositionKeys.push_back(animNode->mPositionKeys[j]);
				for (unsigned int j = 0; j < animNode->mNumRotationKeys; ++j)
					newAnimClip.mRotationKeys.push_back(animNode->mRotationKeys[j]);
				for (unsigned int j = 0; j < animNode->mNumScalingKeys; ++j)
					newAnimClip.mScalingKeys.push_back(animNode->mScalingKeys[j]);

				mSkeleton->AddAnimation(animNode->mNodeName.C_Str(), anim_name, newAnimClip);
			}
		}
	}
}

void ModelLoader::processNode(aiNode * node, const aiScene * scene, aiMatrix4x4 matrix)
{
	aiMatrix4x4 curMat = matrix * node->mTransformation;
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mMeshes.push_back(this->processMesh(mesh, scene, curMat, node->mParent));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene, curMat);
	}
}
