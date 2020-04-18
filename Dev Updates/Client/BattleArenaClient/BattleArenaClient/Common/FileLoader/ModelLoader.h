#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <memory>
#include <assert.h>

#include <vector>
#include <unordered_map>
#include <map>
#include <set>

#include "../Util/assimp/Importer.hpp"
#include "../Util/assimp/scene.h"
#include "../Util/assimp/postprocess.h"

namespace aiModelData
{
	struct aiVertex
	{
		aiVector3D vPosition;
		aiVector3D vNormal;
		aiVector3D vTangent;
		aiVector3D vBitangent;
		aiVector3D vTextureUV[8]; // count 8: AI_MAX_NUMBER_OF_TEXTURECOORDS
		ai_real    fBoneWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // last Weight not used, calculated inside the vertex shader
		ai_int32   i32BoneIndices[4] = { -1, -1, -1, -1 };
	};
	struct aiMesh
	{
		std::string mName;
		std::vector<aiVertex> mVertices;
		std::vector<std::uint32_t> mIndices;
		aiModelData::aiMesh() = default;
		aiModelData::aiMesh(const std::string& Name, const std::vector<aiVertex>& Vertices, const std::vector<std::uint32_t>& Indices)
		{
			mName = Name;
			mVertices = Vertices;
			mIndices = Indices;
		}
	};

	struct AnimInfo
	{
		std::string CurrPlayingAnimName;
		std::vector<aiMatrix4x4> CurrAnimJointTransforms;
		float CurrAnimTimePos = 0.0f;
		bool CurrAnimIsLoop = false;
		bool CurrAnimIsStop = true;
		bool CurrAnimIsPause = false;

		void Init()
		{
			CurrPlayingAnimName.clear();
			CurrAnimJointTransforms.clear();
			CurrAnimTimePos = 0.0f;
			CurrAnimIsLoop = false;
			CurrAnimIsStop = true;
			CurrAnimIsPause = false;
		}

		void AnimPlay(const std::string& AnimName)
		{
			CurrPlayingAnimName = AnimName;
			CurrAnimTimePos = 0.0f;
			CurrAnimIsStop = false;
			CurrAnimIsPause = false;
		}
		void AnimStop(const std::string& AnimName)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrAnimTimePos = 0.0f;
			CurrAnimIsStop = true;
			CurrAnimIsPause = false;
		}
		void AnimResume(const std::string& AnimName)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrAnimIsPause = false;
		}
		void AnimPause(const std::string& AnimName)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrAnimIsPause = true;
		}
		void AnimLoop(const std::string& AnimName, bool isLooping = true)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrAnimIsLoop = isLooping;
		}
		bool AnimIsPlaying(const std::string& AnimName, bool& isSetted)
		{
			if (CurrPlayingAnimName != AnimName) isSetted = false;
			else isSetted = true;
			return !CurrAnimIsStop && !CurrAnimIsPause;
		}
	};

	struct aiAnimClip
	{
		std::string mName;
		std::vector<aiVectorKey> mScalingKeys;
		std::vector<aiQuatKey>   mRotationKeys;
		std::vector<aiVectorKey> mPositionKeys;

		float mDuration = 0.0f;
		float mTickPerSecond = 0.0f;

		float CalculateAnimationTime(float& TimePos, float deltaTime, bool IsStop, bool IsPause, bool IsLoop)
		{
			if (IsStop || IsPause) return TimePos;

			float ClipEndTime = mDuration / mTickPerSecond;

			if (TimePos + deltaTime >= ClipEndTime)
			{
				if (IsLoop)
					TimePos = (TimePos + deltaTime) - ClipEndTime;
				else
					TimePos = ClipEndTime;
			}
			else
				TimePos += deltaTime;

			return TimePos;
		}

		int FindKeyIndex(const float animationTime, const std::vector<aiVectorKey>& vectorKeys) const
		{
			for (int i = 0; i < (int)vectorKeys.size() - 1; ++i)
				if (animationTime < ((float)vectorKeys[i + 1].mTime / mTickPerSecond))
					return i;

			return (int)vectorKeys.size() - 2;
		}

		int FindKeyIndex(const float animationTime, const std::vector<aiQuatKey>& quatKeys) const
		{
			for (int i = 0; i < (int)quatKeys.size() - 1; i++)
				if (animationTime < ((float)quatKeys[i + 1].mTime / mTickPerSecond))
					return i;

			return (int)quatKeys.size() - 2;
		}

		aiVector3D CalcInterpolatedValueFromKey(float animationTime, const std::vector<aiVectorKey>& vectorKeys) const
		{
			aiVector3D ret;
			if (vectorKeys.size() == 1)
			{
				ret = vectorKeys[0].mValue;
				return ret;
			}

			int keyIndex = aiAnimClip::FindKeyIndex(animationTime, vectorKeys);
			int nextKeyIndex = keyIndex + 1;

			float deltaTime = ((float)vectorKeys[nextKeyIndex].mTime / mTickPerSecond) - ((float)vectorKeys[keyIndex].mTime / mTickPerSecond);
			float factor = (animationTime - ((float)vectorKeys[keyIndex].mTime / mTickPerSecond)) / deltaTime;

			assert(factor >= 0.0f && factor <= 1.0f);

			const aiVector3D& startValue = vectorKeys[keyIndex].mValue;
			const aiVector3D& endValue = vectorKeys[nextKeyIndex].mValue;

			ret.x = startValue.x + (endValue.x - startValue.x) * factor;
			ret.y = startValue.y + (endValue.y - startValue.y) * factor;
			ret.z = startValue.z + (endValue.z - startValue.z) * factor;

			return ret;
		}

		aiQuaternion CalcInterpolatedValueFromKey(float animationTime, const std::vector<aiQuatKey>& quatKeys) const
		{
			aiQuaternion ret;
			if (quatKeys.size() == 1)
			{
				ret = quatKeys[0].mValue;
				return ret;
			}

			int keyIndex = FindKeyIndex(animationTime, quatKeys);
			int nextKeyIndex = keyIndex + 1;

			float deltaTime = ((float)quatKeys[nextKeyIndex].mTime / mTickPerSecond) - ((float)quatKeys[keyIndex].mTime / mTickPerSecond);
			float factor = (animationTime - ((float)quatKeys[keyIndex].mTime / mTickPerSecond)) / deltaTime;

			assert(factor >= 0.0f && factor <= 1.0f);

			const aiQuaternion& startValue = quatKeys[keyIndex].mValue;
			const aiQuaternion& endValue = quatKeys[nextKeyIndex].mValue;
			aiQuaternion::Interpolate(ret, startValue, endValue, factor);
			ret = ret.Normalize();

			return ret;
		}

		void InterpolateKeyFrame(float animationTime, aiMatrix4x4& pOut)
		{
			// 주어진 key frame의 정보와 animationTime 정보를 이용해 interpolation을 하고 값을 저장
			const aiVector3D& scaling = CalcInterpolatedValueFromKey(animationTime, mScalingKeys);
			const aiQuaternion& rotationQ = CalcInterpolatedValueFromKey(animationTime, mRotationKeys);
			const aiVector3D& translation = CalcInterpolatedValueFromKey(animationTime, mPositionKeys);

			pOut = aiMatrix4x4(scaling, rotationQ, translation);
		}
	};
	
	struct aiJoint
	{
		std::string mName;
		int mParentIndex = -1;
		aiMatrix4x4 mLocalTransform;
		aiMatrix4x4 mOffsetTransform; // Offset == BindPoseInverse
		std::map<std::string, aiAnimClip> mAnimations;

		void AddAnimation(const std::string& AnimName, const aiAnimClip& newAnimation)
		{
			if (mAnimations.find(AnimName) == mAnimations.end())
				mAnimations[AnimName] = newAnimation;
		}

		void EvaluateAnimTransform(const std::string& AnimName, float animationTime, aiMatrix4x4& pOut)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end())
				pOut = mLocalTransform;
			else
				anim_iter->second.InterpolateKeyFrame(animationTime, pOut);
		}
	};

	struct aiSkeleton
	{
		std::string mName;
		std::vector<aiJoint> mJoints;

		int FindJointIndex(const std::string& JointName)
		{
			for (size_t i = 0; i < mJoints.size(); ++i)
			{
				if (mJoints[i].mName == JointName) return (int)i;
			}
			return -1;
		}

		void AddAnimation(const std::string& JointName, const std::string& AnimName, const aiAnimClip& newAnimation)
		{
			int BoneID = aiSkeleton::FindJointIndex(JointName);
			if (BoneID != -1) mJoints[BoneID].AddAnimation(AnimName, newAnimation);
		}

		void UpdateAnimationTransforms(AnimInfo& Anim_info, float deltaTime)
		{
			std::string AnimName = Anim_info.CurrPlayingAnimName;
			bool IsStop = Anim_info.CurrAnimIsStop;
			bool IsPause = Anim_info.CurrAnimIsPause;
			bool IsLoop = Anim_info.CurrAnimIsLoop;
			float& TimePos = Anim_info.CurrAnimTimePos;
			auto& AnimTransforms = Anim_info.CurrAnimJointTransforms;

			if (AnimTransforms.size() < mJoints.size()) AnimTransforms.resize(mJoints.size());

			bool CalculatedAnimationTime = false;
			float animationTime = TimePos;
			for (size_t i = 0; i < mJoints.size(); ++i)
			{
				if (CalculatedAnimationTime == false)
				{
					auto anim_iter = mJoints[i].mAnimations.find(AnimName);
					if (anim_iter != mJoints[i].mAnimations.end())
					{
						auto& anim = anim_iter->second;
						animationTime = anim.CalculateAnimationTime(TimePos, deltaTime, IsStop, IsPause, IsLoop);
						CalculatedAnimationTime = true;
					}
				}
				mJoints[i].EvaluateAnimTransform(AnimName, animationTime, AnimTransforms[i]);
			}

			auto& GlobalTransforms = AnimTransforms;
			auto& LocalTransforms = AnimTransforms;
			for (size_t i = 0; i < mJoints.size(); ++i)
			{
				int ParentIndex = mJoints[i].mParentIndex;
				if (ParentIndex == -1) continue;
				AnimTransforms[i] = GlobalTransforms[ParentIndex] * LocalTransforms[i];
			}
			for (size_t i = 0; i < mJoints.size(); ++i)
				AnimTransforms[i] *= mJoints[i].mOffsetTransform;
		}
	};
}

class ModelLoader
{
private:
	Assimp::Importer mImporter;
	const aiScene* loadScene(std::string filepath);
public:
	std::vector<aiModelData::aiMesh> mMeshes;
	std::unique_ptr<aiModelData::aiSkeleton> mSkeleton = nullptr;

public:
	bool loadMeshAndSkeleton(std::string filepath);
	bool loadAnimation(std::string filepath);

private:
	void processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix);
	aiModelData::aiMesh processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 matrix, aiNode* parentNode = nullptr);
	void processBoneWeights(std::vector<aiModelData::aiVertex>& pOut, aiBone** Bones_intoMesh, unsigned int numBone, aiMatrix4x4 BindPose);

private:
	void generateSkeleton(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix, const std::string& name);
	void getAnimation_0(const aiScene* scene, const std::string& anim_name);

};

#endif // !MODEL_LOADER_H

