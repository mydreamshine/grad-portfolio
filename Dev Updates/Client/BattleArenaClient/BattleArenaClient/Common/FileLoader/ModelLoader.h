#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <memory>
#include <assert.h>

#include <vector>
#include <unordered_map>
#include <map>

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

	struct aiAnimClip
	{
		std::string mName;
		std::vector<aiVectorKey> mScalingKeys;
		std::vector<aiQuatKey>   mRotationKeys;
		std::vector<aiVectorKey> mPositionKeys;

		float mDuration = 0.0f;
		float mTickPerSecond = 0.0f;

		float mTimePos = 0.0f;

		bool mIsLoop = false;
		bool mIsStop = true;
		bool mIsPause = false;

		void Loop(bool isLoop) { mIsLoop = isLoop; }
		void Stop()
		{
			mIsStop = true;
			mIsPause = false;
			mTimePos = 0.0f;
		}
		void Play()
		{
			mIsStop = false;
			mIsPause = false;
			mTimePos = 0.0f;
		}
		void Pause() { mIsStop = false;  mIsPause = true; }
		void Resume() { mIsStop = false;  mIsPause = false; }

		float CalculateAnimationTime(float deltaTime)
		{
			if (mIsStop || mIsPause) return mTimePos;

			float ClipEndTime = mDuration / mTickPerSecond;

			if (mTimePos + deltaTime >= ClipEndTime)
			{
				if (mIsLoop)
					mTimePos = (mTimePos + deltaTime) - ClipEndTime;
				else
					mTimePos = ClipEndTime;
			}
			else
				mTimePos += deltaTime;

			return mTimePos;
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

		void InterpolateKeyFrame(float deltaTime, aiMatrix4x4& pOut)
		{
			float animationTime = CalculateAnimationTime(deltaTime);
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

		bool AnimLoop(const std::string& AnimName, bool isLoop)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end()) return false;
			else anim_iter->second.Loop(isLoop); return true;
		}
		bool AnimStop(const std::string& AnimName)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end()) return false;
			else anim_iter->second.Stop(); return true;
		}
		bool AnimPlay(const std::string& AnimName)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end()) return false;
			else anim_iter->second.Play(); return true;
		}
		bool AnimPause(const std::string& AnimName)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end()) return false;
			else anim_iter->second.Pause(); return true;
		}
		bool AnimResume(const std::string& AnimName)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end()) return false;
			else anim_iter->second.Resume(); return true;
		}

		bool AnimIsPlaying(const std::string& AnimName) const
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end()) return false;
			else return (!anim_iter->second.mIsStop && !anim_iter->second.mIsPause);
		}

		void EvaluateAnimTransform(const std::string& AnimName, float deltaTime, aiMatrix4x4& pOut)
		{
			auto anim_iter = mAnimations.find(AnimName);
			if (anim_iter == mAnimations.end())
				pOut = mLocalTransform;
			else
				anim_iter->second.InterpolateKeyFrame(deltaTime, pOut);
		}
	};

	struct aiSkeleton
	{
		std::string mName;
		std::vector<aiJoint> mJoints;

		std::string mCurrPlayingAnimName;

		std::vector<aiMatrix4x4> mCurrAnimTransforms;
		int SkinnedCBIndex = -1;

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

		void AnimLoop(const std::string& AnimName, bool isLoop)
		{
			for (auto& joint : mJoints)
				joint.AnimLoop(AnimName, isLoop);
		}
		void AnimStop(const std::string& AnimName)
		{
			for (auto& joint : mJoints)
				joint.AnimStop(AnimName);
		}
		void AnimPlay(const std::string& AnimName)
		{
			bool isSetted = false;
			for (auto& joint : mJoints)
			{
				if (joint.AnimPlay(AnimName) && !isSetted)
				{
					isSetted = true;
					mCurrPlayingAnimName = AnimName;
				}
			}
		}
		void AnimPause(const std::string& AnimName)
		{
			for (auto& joint : mJoints)
				joint.AnimPause(AnimName);
		}
		void AnimResume(const std::string& AnimName)
		{
			for (auto& joint : mJoints)
				joint.AnimResume(AnimName);
		}

		bool AnimIsPlaying(const std::string& AnimName) const
		{
			for (auto& joint : mJoints)
			{
				if (joint.AnimIsPlaying(AnimName)) return true;
			}
			return false;
		}

		void UpdateAnimationTransforms(const std::string& AnimName, float deltaTime, std::vector<aiMatrix4x4>* pOut = nullptr)
		{
			if (mCurrAnimTransforms.size() < mJoints.size()) mCurrAnimTransforms.resize(mJoints.size());

			for (size_t i = 0; i < mJoints.size(); ++i)
				mJoints[i].EvaluateAnimTransform(AnimName, deltaTime, mCurrAnimTransforms[i]);

			std::vector<aiMatrix4x4>& GlobalTransforms = mCurrAnimTransforms;
			std::vector<aiMatrix4x4>& LocalTransforms = mCurrAnimTransforms;
			for (size_t i = 0; i < mJoints.size(); ++i)
			{
				int ParentIndex = mJoints[i].mParentIndex;
				if (ParentIndex == -1) continue;
				mCurrAnimTransforms[i] = GlobalTransforms[ParentIndex] * LocalTransforms[i];
			}
			for (size_t i = 0; i < mJoints.size(); ++i)
				mCurrAnimTransforms[i] *= mJoints[i].mOffsetTransform;

			if (pOut != nullptr)
			{
				for (size_t i = 0; i < mCurrAnimTransforms.size(); ++i)
					(*pOut)[i] = mCurrAnimTransforms[i];
			}
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

