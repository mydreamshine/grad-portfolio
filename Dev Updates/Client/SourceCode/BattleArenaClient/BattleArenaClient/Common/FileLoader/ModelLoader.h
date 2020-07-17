#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <memory>
#include <assert.h>

#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <float.h>

#include "../Util/assimp/Importer.hpp"
#include "../Util/assimp/scene.h"
#include "../Util/assimp/postprocess.h"

namespace aiModelData
{
	struct aiSkeleton;

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
		aiAABB mAABB;
		int mParentBoneIndex = -1;

		aiModelData::aiMesh() = default;
		aiModelData::aiMesh(
			const std::string& Name,
			const std::vector<aiVertex>& Vertices, const std::vector<std::uint32_t>& Indices,
			const aiAABB& AABB,
			const int& ParentBoneIndex)
		{
			mName = Name;
			mVertices = Vertices;
			mIndices = Indices;
			mAABB = AABB;
			mParentBoneIndex = ParentBoneIndex;
		}
	};

	struct aiBoundingBox
	{
		aiVector3D vCenter;
		aiVector3D vExtents;
	};

	struct AnimNotify
	{
		float TimePos = FLT_MAX;
		bool  TimeLineOver = false;
		bool  PickUp = false;
	};

	// FrameworkEvent의 MOTION_TYPE과 1:1 매칭됨
	enum class AnimActionType : char
	{
		Idle, Walk, Attack, Impact,	Dieing, SkillPose, FreeMotion, Count, Non
	};

	struct AnimInfo
	{
		std::string CurrPlayingAnimName;

		std::vector<aiMatrix4x4> CurrAnimJointTransforms;
		std::vector<aiMatrix4x4> OffsetJointTransforms;

		float CurrAnimTimePos = 0.0f;

		std::unordered_map<std::string, AnimNotify> AnimTimeLineNotifys;

		bool CurrAnimIsLoop = false;
		bool CurrAnimIsStop = true;
		bool CurrAnimIsPause = false;
		bool CurrAnimDurationIsOnceDone = false;

		std::vector<std::string> Actions[(int)AnimActionType::Count];
		AnimActionType CurrPlayingAction = AnimActionType::Non;


		void Init()
		{
			CurrPlayingAnimName.clear();
			CurrAnimJointTransforms.clear();
			OffsetJointTransforms.clear();
			CurrAnimTimePos = 0.0f;
			CurrAnimIsLoop = false;
			CurrAnimIsStop = true;
			CurrAnimIsPause = false;
			CurrAnimDurationIsOnceDone = false;
			AnimTimeLineNotifys.clear();
			for (auto& ActionNames : Actions)
				ActionNames.clear();
			CurrPlayingAction = AnimActionType::Non;
		}

		void SetAction(const std::string& AnimName, AnimActionType ActionType)
		{
			size_t CountAction = Actions[(int)ActionType].size();
			if (CountAction > 0)
			{
				bool AlreadyExist = false;
				for (auto& ActionName : Actions[(int)ActionType])
				{
					if (ActionName == AnimName)
					{
						AlreadyExist = true;
						break;
					}
				}
				if (AlreadyExist == false) Actions[(int)ActionType].emplace_back(AnimName);
			}
			else Actions[(int)ActionType].emplace_back(AnimName);
		}

		void AnimTimeLineNotifyInit(const std::string& AnimName)
		{
			for (auto& notify_iter : AnimTimeLineNotifys)
			{
				auto& notify_name = notify_iter.first;
				if (notify_name.find(AnimName.c_str()) != std::string::npos)
				{
					auto& notify = notify_iter.second;
					notify.TimeLineOver = false;
					notify.PickUp = false;
				}
			}
		}

		void AnimPlay(const std::string& AnimName)
		{
			CurrPlayingAnimName = AnimName;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);
			CurrAnimTimePos = 0.0f;
			CurrAnimIsStop = false;
			CurrAnimIsPause = false;
			CurrAnimDurationIsOnceDone = false;
			AnimInfo::AnimTimeLineNotifyInit(AnimName);
		}
		void AnimStop(const std::string& AnimName)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);
			CurrAnimTimePos = 0.0f;
			CurrAnimIsStop = true;
			CurrAnimIsPause = false;
			CurrAnimDurationIsOnceDone = false;
			AnimInfo::AnimTimeLineNotifyInit(AnimName);
		}
		void AnimResume(const std::string& AnimName)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);
			CurrAnimIsPause = false;
		}
		void AnimPause(const std::string& AnimName)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);
			CurrAnimIsPause = true;
		}
		void AnimLoop(const std::string& AnimName, bool isLooping = true)
		{
			if (CurrPlayingAnimName != AnimName) return;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);
			CurrAnimIsLoop = isLooping;
		}
		bool AnimIsPlaying(const std::string& AnimName, bool& isSetted)
		{
			if (CurrPlayingAnimName != AnimName) isSetted = false;
			else isSetted = true;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);

			return !CurrAnimIsStop && !CurrAnimIsPause;
		}
		bool AnimOnceDone(const std::string& AnimName, bool& isSetted)
		{
			if (CurrPlayingAnimName != AnimName) isSetted = false;
			else isSetted = true;
			CurrPlayingAction = AnimInfo::FindActionFrom(AnimName);

			return CurrAnimDurationIsOnceDone;
		}

		AnimActionType FindActionFrom(const std::string& AnimName)
		{
			for (int i = 0; i < (int)AnimActionType::Count; ++i)
			{
				for (auto& ActionName : Actions[i])
				{
					if (ActionName == AnimName)
						return (AnimActionType)i;
				}
			}

			return AnimActionType::Non;
		}

		void AnimPlay(const AnimActionType& ActionType, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			AnimInfo::AnimPlay(AnimName);
		}
		void AnimStop(const AnimActionType& ActionType, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			AnimInfo::AnimStop(AnimName);
		}
		void AnimResume(const AnimActionType& ActionType, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			AnimInfo::AnimResume(AnimName);
		}
		void AnimPause(const AnimActionType& ActionType, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			AnimInfo::AnimPause(AnimName);
		}
		void AnimLoop(const AnimActionType& ActionType, bool isLooping = true, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			AnimInfo::AnimLoop(AnimName, isLooping);
		}
		bool AnimIsPlaying(const AnimActionType& ActionType, bool& isSetted, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return false;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			return AnimInfo::AnimIsPlaying(AnimName, isSetted);
		}
		bool AnimOnceDone(const AnimActionType& ActionType, bool& isSetted, int IndexAction = 0)
		{
			if (Actions[(int)ActionType].size() == 0) return false;
			std::string AnimName = Actions[(int)ActionType][IndexAction];
			return AnimInfo::AnimOnceDone(AnimName, isSetted);
		}

		// 애니메이션 파일 이름에 ActionType이 있을 경우에만 호출 가능한 메소드
	    // ex) "Meshtint Free Knight@Idle.fbx"와 같이 애니메이션 파일이름에 "Idle"이 들어가 있으면
	    // ActionType의 Idle과 부합하므로 해당 애니메이션을 Action으로 등록 가능
		void AutoApplyActionFromSkeleton(const std::set<std::string>& AnimNameList)
		{
			for (auto& AnimName : AnimNameList)
			{
				if      (AnimName.find("Idle") != std::string::npos)      this->SetAction(AnimName, aiModelData::AnimActionType::Idle);
				else if (AnimName.find("Walk") != std::string::npos)      this->SetAction(AnimName, aiModelData::AnimActionType::Walk);
				else if (AnimName.find("Attack") != std::string::npos)    this->SetAction(AnimName, aiModelData::AnimActionType::Attack);
				else if (AnimName.find("Impact") != std::string::npos)    this->SetAction(AnimName, aiModelData::AnimActionType::Impact);
				else if (AnimName.find("Dieing") != std::string::npos)    this->SetAction(AnimName, aiModelData::AnimActionType::Dieing);
				else if (AnimName.find("SkillPose") != std::string::npos) this->SetAction(AnimName, aiModelData::AnimActionType::SkillPose);
				else                                                      this->SetAction(AnimName, aiModelData::AnimActionType::FreeMotion);
			}
		}

		void SetAnimTimeLineNotify(const std::string& notify_name, float notify_timePos)
		{
			auto& notify = AnimTimeLineNotifys[notify_name];
			notify.TimePos = notify_timePos;
			notify.TimeLineOver = false;
			notify.PickUp = false;
		}

		bool CheckAnimTimeLineNotify(const std::string& notify_name, bool& IsSetted)
		{
			if (AnimTimeLineNotifys.find(notify_name) != AnimTimeLineNotifys.end())
			{
				IsSetted = true;
			}
			else
			{
				IsSetted = false;
				return false;
			}

			auto& notify = AnimTimeLineNotifys[notify_name];

			bool retCheck = false;
			if (notify.PickUp == false && notify.TimeLineOver == true)
			{
				notify.PickUp = true;
				retCheck = true;
			}

			return retCheck;
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

		float CalculateAnimationTime(float& TimePos, float deltaTime, bool IsStop, bool IsPause, bool IsLoop, bool& DurationOnceDone, bool& initDuration)
		{
			if (IsStop || IsPause) return TimePos;

			float ClipEndTime = mDuration / mTickPerSecond;

			if (TimePos + deltaTime >= ClipEndTime)
			{
				if (IsLoop)
					TimePos = (TimePos + deltaTime) - ClipEndTime;
				else
					TimePos = ClipEndTime;

				DurationOnceDone = true;

				initDuration = true;
			}
			else
			{
				TimePos += deltaTime;
				initDuration = false;
			}

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

		void GetAnimationList_Name(std::set<std::string>& AnimNameList)
		{
			for (auto& Joint : mJoints)
			{
				for (auto& Anim_iter : Joint.mAnimations)
					AnimNameList.insert(Anim_iter.first);
			}
		}

		void GetAnimationList_ClipEndTime(std::unordered_map<std::string, float>& AnimClipEndTimeList)
		{
			for (auto& Joint : mJoints)
			{
				for (auto& Anim_iter : Joint.mAnimations)
				{
					if (AnimClipEndTimeList.find(Anim_iter.first) != AnimClipEndTimeList.end()) continue;
					auto& AnimClip = Anim_iter.second;
					AnimClipEndTimeList[Anim_iter.first] = AnimClip.mDuration / AnimClip.mTickPerSecond;
				}
			}
		}

		void UpdateAnimationTransforms(AnimInfo& Anim_info, float deltaTime)
		{
			std::string AnimName = Anim_info.CurrPlayingAnimName;
			bool IsStop = Anim_info.CurrAnimIsStop;
			bool IsPause = Anim_info.CurrAnimIsPause;
			bool IsLoop = Anim_info.CurrAnimIsLoop;
			bool& DurationOnceDone = Anim_info.CurrAnimDurationIsOnceDone;
			bool InitDuration = false;
			float& TimePos = Anim_info.CurrAnimTimePos;
			auto& AnimTransforms = Anim_info.CurrAnimJointTransforms;
			auto& OffsetTransforms = Anim_info.OffsetJointTransforms;
			auto& AnimNotifys = Anim_info.AnimTimeLineNotifys;

			if (AnimTransforms.size() < mJoints.size()) AnimTransforms.resize(mJoints.size());
			if (OffsetTransforms.size() < mJoints.size()) OffsetTransforms.resize(mJoints.size());

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
						animationTime = anim.CalculateAnimationTime(TimePos, deltaTime, IsStop, IsPause, IsLoop, DurationOnceDone, InitDuration);

						// Check&Set AnimTimeLineNotifys
						{
							if (InitDuration == true)
								Anim_info.AnimTimeLineNotifyInit(AnimName);

							for (auto& anim_notify_iter : AnimNotifys)
							{
								auto& notify_name = anim_notify_iter.first;
								if (notify_name.find(AnimName.c_str()) != std::string::npos)
								{
									auto& notify = anim_notify_iter.second;
									if (notify.TimePos <= TimePos)
										notify.TimeLineOver = true;
								}
							}
						}

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
			// Offset 적용은 AnimInfo외부에서 따로 해준다.
			for (size_t i = 0; i < mJoints.size(); ++i)
				OffsetTransforms[i] = mJoints[i].mOffsetTransform;
		}
	};
}

// 1m 기준 비례 단위
// ModelLoader에 Import할 때
// FileUnit에 따라 컨버팅을 하려 했으나
// 단순히 Vertex의 Position만 유닛변환을 하면
// 회전이 변환된 Vertex에 제대로 적용이 안된다.
// Quarternion에서의 유닛변환도 해줘야 하는 것으로 추측.
// Quarternion의 유닛변환은 구현하기가 어려우므로
// 단순히 해당 메쉬를 취하는 오브젝트의 트랜스폼 스케일값을 변경하기로 결정.
namespace ModelFileUnit
{
	constexpr float meter = 1.0f;
	constexpr float centimeter = 0.01f;
	constexpr float inch = 0.0254f;
}

class ModelLoader
{
private:
	Assimp::Importer mImporter;
	bool mAllMeshToSkinned = false;

	const aiScene* loadScene(std::string filepath);

public:
	std::vector<aiModelData::aiMesh> mMeshes;
	std::unique_ptr<aiModelData::aiSkeleton> mSkeleton = nullptr;
	std::unordered_map<std::string, aiModelData::aiBoundingBox> mBoundingBoxes;

public:
	// true: Static Mesh도 부모 뼈대 ID를 갖게 한다.
	// false: Static Mesh의 뼈대가 존재하지 않게 한다. (Attaching을 요구하는 Mesh일 경우 외부에서 따로 구현한다.)
	void ImportingAllMeshAsSkinned(bool IsAccept) { mAllMeshToSkinned = IsAccept; }
	bool loadMeshAndSkeleton(std::string filepath, std::vector<std::string>* execpt_nodes = nullptr);
	bool loadAnimation(std::string filepath);
	bool loadBoundingBox(std::string filepath, std::vector<std::string>* execpt_nodes = nullptr);
	// SkinnedMesh에 한해서만 BoundingBox를 Merge한다.
	bool loadMergedBoundingBox(std::string filepath, aiModelData::aiBoundingBox& MergedBoundingBox, std::vector<std::string>* execpt_nodes = nullptr);
	bool loadBoundingBoxesToTXTfile(const std::string& filepath_ToWrite, const std::string& soruce_path,
		float CovertUnit = 1.0f, bool MergeBoundingBoxes = false, std::vector<std::string>* execpt_nodes = nullptr);

private:
	void processNode(
		aiNode* node, const aiScene* scene,
		aiMatrix4x4 matrix,
		std::vector<std::string>* execpt_nodes = nullptr);
	aiModelData::aiMesh processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 matrix, aiNode* parentNode = nullptr);
	void processBoneWeights(
		std::vector<aiModelData::aiVertex>& pOut,
		aiBone** Bones_intoMesh, unsigned int numBone,
		aiMatrix4x4 BindPose);
	void processBoundingBox(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix, std::vector<std::string>* execpt_nodes = nullptr);
	void processMergedAABB(aiNode* node, const aiScene* scene, aiMatrix4x4 matrix, aiAABB& mergedAABB,
		std::vector<std::string>* execpt_nodes = nullptr);

private:
	void generateSkeleton(
		aiNode* node, const aiScene* scene,
		aiMatrix4x4 matrix,
		const std::string& name,
		std::vector<std::string>* execpt_nodes = nullptr);
	void getAnimation_0(const aiScene* scene, const std::string& anim_name);

private:
	bool execptNodeProcessing(aiNode* node, const std::vector<std::string>& execpt_nodes);

};

#endif // !MODEL_LOADER_H

