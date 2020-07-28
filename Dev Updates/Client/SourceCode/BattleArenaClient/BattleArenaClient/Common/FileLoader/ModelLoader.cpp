#include "ModelLoader.h"

#include <fstream>
#include <stdexcept>

const aiScene* ModelLoader::loadScene(std::string filepath)
{
	const aiScene* pScene = mImporter.ReadFile(filepath,
		aiProcess_JoinIdenticalVertices |     // 동일한 꼭지점 결합, 인덱싱 최적화
		aiProcess_ValidateDataStructure |     // 로더의 출력을 검증
		aiProcess_ImproveCacheLocality |      // 출력 정점의 캐쉬위치를 개선
		aiProcess_RemoveRedundantMaterials |  // 중복된 매터리얼 제거
		aiProcess_GenUVCoords |               // 구형, 원통형, 상자 및 평면 매핑을 적절한 UV로 변환
		aiProcess_TransformUVCoords |         // UV 변환 처리기 (스케일링, 변환...)
		aiProcess_FindInstances |             // 인스턴스된 매쉬를 검색하여 하나의 마스터에 대한 참조로 제거
		aiProcess_LimitBoneWeights |          // 정점당 뼈의 가중치를 최대 4개로 제한
		aiProcess_OptimizeMeshes |            // 가능한 경우 작은 매쉬를 조인
		aiProcess_GenSmoothNormals |          // 부드러운 노말벡터(법선벡터) 생성
		aiProcess_SplitLargeMeshes |          // 거대한 하나의 매쉬를 하위매쉬들로 분활(나눔)
		aiProcess_Triangulate |               // 3개 이상의 모서리를 가진 다각형 면을 삼각형으로 만듬(나눔)
		aiProcess_ConvertToLeftHanded |       // D3D의 왼손좌표계로 변환
		aiProcess_SortByPType);
	return pScene;
}

bool ModelLoader::loadMeshAndSkeleton(std::string filepath, std::vector<std::string>* execpt_nodes)
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

	ModelLoader::generateSkeleton(pScene->mRootNode, pScene, aiMatrix4x4(), _Filename, execpt_nodes);
	ModelLoader::processNode(pScene->mRootNode, pScene, aiMatrix4x4(), execpt_nodes);

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

bool ModelLoader::loadBoundingBox(std::string filepath, std::vector<std::string>* execpt_nodes)
{
	const aiScene* pScene = ModelLoader::loadScene(filepath);

	if (pScene == NULL)
		return false;

	char _Drive[_MAX_DRIVE];
	char _Dir[_MAX_DIR];
	char _Filename[_MAX_FNAME];
	char _Ext[_MAX_EXT];
	_splitpath_s(filepath.c_str(), _Drive, _Dir, _Filename, _Ext);

	ModelLoader::processBoundingBox(pScene->mRootNode, pScene, aiMatrix4x4(), execpt_nodes);

	return true;
}

bool ModelLoader::loadMergedBoundingBox(std::string filepath, aiModelData::aiBoundingBox& MergedBoundingBox, std::vector<std::string>* execpt_nodes)
{
	const aiScene* pScene = ModelLoader::loadScene(filepath);

	if (pScene == NULL)
		return false;

	char _Drive[_MAX_DRIVE];
	char _Dir[_MAX_DIR];
	char _Filename[_MAX_FNAME];
	char _Ext[_MAX_EXT];
	_splitpath_s(filepath.c_str(), _Drive, _Dir, _Filename, _Ext);

	aiAABB MergedAABB;
	ModelLoader::processMergedAABB(pScene->mRootNode, pScene, aiMatrix4x4(), MergedAABB, execpt_nodes);

	MergedBoundingBox.vCenter = {
				(MergedAABB.mMax.x + MergedAABB.mMin.x) * 0.5f,
				(MergedAABB.mMax.y + MergedAABB.mMin.y) * 0.5f,
				(MergedAABB.mMax.z + MergedAABB.mMin.z) * 0.5f };
	MergedBoundingBox.vExtents = {
		(MergedAABB.mMax.x - MergedAABB.mMin.x) * 0.5f,
		(MergedAABB.mMax.y - MergedAABB.mMin.y) * 0.5f,
		(MergedAABB.mMax.z - MergedAABB.mMin.z) * 0.5f };

	return true;
}

bool ModelLoader::loadBoundingBoxesToTXTfile(const std::string& filepath_ToWrite, const std::string& soruce_path,
	float CovertUnit, bool MergeBoundingBoxes, std::vector<std::string>* execpt_nodes)
{
	const aiScene* pScene = ModelLoader::loadScene(soruce_path);

	if (pScene == NULL)
		return false;

	char _Drive[_MAX_DRIVE];
	char _Dir[_MAX_DIR];
	char _Filename[_MAX_FNAME];
	char _Ext[_MAX_EXT];
	_splitpath_s(soruce_path.c_str(), _Drive, _Dir, _Filename, _Ext);

	if (MergeBoundingBoxes == true)
	{
		aiAABB MergedAABB;
		ModelLoader::processMergedAABB(pScene->mRootNode, pScene, aiMatrix4x4(), MergedAABB, execpt_nodes);

		aiModelData::aiBoundingBox MergedBoundingBox;
		MergedBoundingBox.vCenter = {
				(MergedAABB.mMax.x + MergedAABB.mMin.x) * 0.5f,
				(MergedAABB.mMax.y + MergedAABB.mMin.y) * 0.5f,
				(MergedAABB.mMax.z + MergedAABB.mMin.z) * 0.5f };
		MergedBoundingBox.vExtents = {
			(MergedAABB.mMax.x - MergedAABB.mMin.x) * 0.5f,
			(MergedAABB.mMax.y - MergedAABB.mMin.y) * 0.5f,
			(MergedAABB.mMax.z - MergedAABB.mMin.z) * 0.5f };

		std::ofstream writeFile(filepath_ToWrite.c_str());
		if (writeFile.is_open() != true) return false;

		// write file as format
		// Name: BoundingBoxName, Center (x, y, z), Extents (x, y, z)\n
		//writeFile << "Name: " + std::string(_Filename);
		writeFile << std::string(_Filename);
		writeFile
			//<< ", Center: ("
			<< " " << std::to_string(MergedBoundingBox.vCenter.x * CovertUnit)
			<< " " << std::to_string(MergedBoundingBox.vCenter.y * CovertUnit)
			<< " " << std::to_string(MergedBoundingBox.vCenter.z * CovertUnit)
			//<< "), Exetents: ("
			<< " " << std::to_string(MergedBoundingBox.vExtents.x * CovertUnit)
			<< " " << std::to_string(MergedBoundingBox.vExtents.y * CovertUnit)
			<< " " << std::to_string(MergedBoundingBox.vExtents.z * CovertUnit)
			<< "\n";
			//<< ")\n";

		writeFile.close();
	}
	else
	{
		ModelLoader::processBoundingBox(pScene->mRootNode, pScene, aiMatrix4x4(), execpt_nodes);

		std::ofstream writeFile(filepath_ToWrite.c_str());
		if (writeFile.is_open() != true) return false;

		for (auto& BoundingBox_iter : mBoundingBoxes)
		{
			auto& BoudingBoxName = BoundingBox_iter.first;
			auto& BoudingBox = BoundingBox_iter.second;

			// write file as format
			// Name: BoundingBoxName, Center (x, y, z), Extents (x, y, z)\n
			//writeFile << "Name: " + BoudingBoxName;
			writeFile << BoudingBoxName;
			writeFile
				//<< ", Center: ("
				<< " " << std::to_string(BoudingBox.vCenter.x * CovertUnit)
				<< " " << std::to_string(BoudingBox.vCenter.y * CovertUnit)
				<< " " << std::to_string(BoudingBox.vCenter.z * CovertUnit)
				//<< "), Exetents: ("
				<< " " << std::to_string(BoudingBox.vExtents.x * CovertUnit)
				<< " " << std::to_string(BoudingBox.vExtents.y * CovertUnit)
				<< " " << std::to_string(BoudingBox.vExtents.z * CovertUnit)
				//<< ")\n";
				<< "\n";
		}

		writeFile.close();
	}

	return true;
}

aiModelData::aiMesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 matrix, aiNode* parentNode)
{
	std::vector<aiModelData::aiVertex> Vertices;
	std::vector<std::uint32_t> Indices;

	aiMatrix4x4 BindPoseTransform = matrix;

	aiAABB AABB;
	AABB.mMin = { FLT_MAX, FLT_MAX, FLT_MAX };
	AABB.mMax = BindPoseTransform * mesh->mVertices[0];

	// read mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		aiModelData::aiVertex vertex;

		vertex.vPosition = BindPoseTransform * mesh->mVertices[i];;

		AABB.mMin.x = fminf(AABB.mMin.x, vertex.vPosition.x);
		AABB.mMin.y = fminf(AABB.mMin.y, vertex.vPosition.y);
		AABB.mMin.z = fminf(AABB.mMin.z, vertex.vPosition.z);
		AABB.mMax.x = fmaxf(AABB.mMax.x, vertex.vPosition.x);
		AABB.mMax.y = fmaxf(AABB.mMax.y, vertex.vPosition.y);
		AABB.mMax.z = fmaxf(AABB.mMax.z, vertex.vPosition.z);

		if (mesh->mNormals) vertex.vNormal = BindPoseTransform * mesh->mNormals[i];
		if (mesh->mTangents) vertex.vTangent = BindPoseTransform * mesh->mTangents[i];
		if (mesh->mBitangents) vertex.vBitangent = BindPoseTransform * mesh->mBitangents[i];
		if (mesh->mTextureCoords[0])
		{
			vertex.vTextureUV[0].x = (float)mesh->mTextureCoords[0][i].x;
			vertex.vTextureUV[0].y = 1.0f - (float)mesh->mTextureCoords[0][i].y;
		}

		Vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			Indices.push_back(face.mIndices[j]);
	}

	aiNode* parentBoneNode = parentNode;
	int parentBoneID = -1;
	if (mSkeleton != nullptr) parentBoneID = mSkeleton->FindJointIndex(parentBoneNode->mName.C_Str());
	
	if (mesh->mBones == nullptr) // Static Mesh
	{
		// Static Mesh에 대한 애니메이션을 위해
		// Static Mesh의 부모 뼈대에 OffsetTransform(BindPoseInverse) 부여
		if (parentBoneNode)
		{
			if (parentBoneID != -1)
			{
				aiMatrix4x4 bindPose = BindPoseTransform; // BindPoseTransform.Inverse()를 하면 BindPoseTransform자체가 Inverse된다.
				mSkeleton->mJoints[parentBoneID].mOffsetTransform = bindPose.Inverse();

				if (mAllMeshToSkinned == true)
				{
					for (auto& vertex : Vertices)
					{
						vertex.fBoneWeights[0] = 1.0f;
						vertex.i32BoneIndices[0] = parentBoneID;
					}
				}
			}
		}
	}
	else
	{
		parentBoneID = -1;
		ModelLoader::processBoneWeights(Vertices, mesh->mBones, mesh->mNumBones, BindPoseTransform);
	}

	return aiModelData::aiMesh(mesh->mName.C_Str(), Vertices, Indices, AABB, parentBoneID);
}

void ModelLoader::processBoneWeights(
	std::vector<aiModelData::aiVertex>& pOut,
	aiBone** Bones_intoMesh, unsigned int numBone,
	aiMatrix4x4 BindPoseTransform)
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
					if (Joint.mOffsetTransform.IsIdentity())
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

void ModelLoader::processBoundingBox(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix, std::vector<std::string>* execpt_nodes)
{
	aiMatrix4x4 curMat = matrix * node->mTransformation;

	bool execpt_processing = false;
	if (execpt_nodes)
		execpt_processing = ModelLoader::execptNodeProcessing(node, *execpt_nodes);

	if (execpt_processing != true)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			
			aiMatrix4x4 BindPoseTransform = curMat;
			aiAABB AABB;
			AABB.mMin = { FLT_MAX, FLT_MAX, FLT_MAX };
			AABB.mMax = BindPoseTransform * mesh->mVertices[0];

			// read mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				aiVector3D vPosition = BindPoseTransform * mesh->mVertices[i];;

				AABB.mMin.x = fminf(AABB.mMin.x, vPosition.x);
				AABB.mMin.y = fminf(AABB.mMin.y, vPosition.y);
				AABB.mMin.z = fminf(AABB.mMin.z, vPosition.z);
				AABB.mMax.x = fmaxf(AABB.mMax.x, vPosition.x);
				AABB.mMax.y = fmaxf(AABB.mMax.y, vPosition.y);
				AABB.mMax.z = fmaxf(AABB.mMax.z, vPosition.z);
			}

			aiModelData::aiBoundingBox BoundingBox;
			BoundingBox.vCenter = {
				(AABB.mMax.x + AABB.mMin.x) * 0.5f,
				(AABB.mMax.y + AABB.mMin.y) * 0.5f,
				(AABB.mMax.z + AABB.mMin.z) * 0.5f };
			BoundingBox.vExtents = {
				(AABB.mMax.x - AABB.mMin.x) * 0.5f,
				(AABB.mMax.y - AABB.mMin.y) * 0.5f,
				(AABB.mMax.z - AABB.mMin.z) * 0.5f };

			mBoundingBoxes[mesh->mName.C_Str()] = BoundingBox;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->processBoundingBox(node->mChildren[i], scene, curMat, execpt_nodes);
	}
}

void ModelLoader::processMergedAABB(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix, aiAABB& mergedAABB,
	std::vector<std::string>* execpt_nodes)
{
	aiMatrix4x4 curMat = matrix * node->mTransformation;

	bool execpt_processing = false;
	if (execpt_nodes)
		execpt_processing = ModelLoader::execptNodeProcessing(node, *execpt_nodes);

	if (execpt_processing != true)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			// Skinned Mesh에 한해서만 Bounding Box를 구한다.
			if (mesh->HasBones() != true) continue;

			aiMatrix4x4 BindPoseTransform = curMat;

			// read mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				aiVector3D vPosition = BindPoseTransform * mesh->mVertices[i];;

				mergedAABB.mMin.x = fminf(mergedAABB.mMin.x, vPosition.x);
				mergedAABB.mMin.y = fminf(mergedAABB.mMin.y, vPosition.y);
				mergedAABB.mMin.z = fminf(mergedAABB.mMin.z, vPosition.z);
				mergedAABB.mMax.x = fmaxf(mergedAABB.mMax.x, vPosition.x);
				mergedAABB.mMax.y = fmaxf(mergedAABB.mMax.y, vPosition.y);
				mergedAABB.mMax.z = fmaxf(mergedAABB.mMax.z, vPosition.z);
			}

		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->processMergedAABB(node->mChildren[i], scene, curMat, mergedAABB, execpt_nodes);
	}
}

void ModelLoader::generateSkeleton(
	aiNode* node, const aiScene* scene,
	aiMatrix4x4 matrix,
	const std::string& name,
	std::vector<std::string>* execpt_nodes)
{
	std::string cs_NodeName = node->mName.C_Str();
	aiMatrix4x4 curMat = matrix * node->mTransformation;

	bool execpt_processing = false;
	if (execpt_nodes)
		execpt_processing = ModelLoader::execptNodeProcessing(node, *execpt_nodes);

	if (execpt_processing != true && node->mNumMeshes == 0) // 뼈대 노드인 경우
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
		ModelLoader::generateSkeleton(node->mChildren[i], scene, curMat, name, execpt_nodes);
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

void ModelLoader::processNode(aiNode * node, const aiScene * scene, aiMatrix4x4 matrix, std::vector<std::string>* execpt_nodes)
{
	aiMatrix4x4 curMat = matrix * node->mTransformation;

	bool execpt_processing = false;
	if (execpt_nodes)
		execpt_processing = ModelLoader::execptNodeProcessing(node, *execpt_nodes);

	if (execpt_processing != true)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			mMeshes.push_back(this->processMesh(mesh, scene, curMat, node->mParent));
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene, curMat, execpt_nodes);
	}
}

bool ModelLoader::execptNodeProcessing(aiNode* node, const std::vector<std::string>& execpt_nodes)
{
	for (auto& execpt_node_name : execpt_nodes)
	{
		std::string nodeName = node->mName.C_Str();
		if (execpt_node_name == nodeName || (nodeName.find(execpt_node_name.c_str()) != std::string::npos))
			return true;
	}
	return false;
}
