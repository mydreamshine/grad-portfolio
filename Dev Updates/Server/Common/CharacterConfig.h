#pragma once

constexpr int BASIC_HP = 100;
constexpr float BASIC_VELOCITY = 400.0f;
constexpr float RESPAWN_TIME = 5.0f;
constexpr float SEMI_INVINCIBLE_TIME = 3.0f;

constexpr int NORMAL_ATTACK_DAMAGE = 10;
constexpr float NORMAL_ATTACK_VELOCITY = 10.0f;
constexpr float NORMAL_ATTACK_DURATION = 5.0f;

constexpr int SWORD_WAVE_DAMAGE = 30;
constexpr float SWORD_WAVE_DURATION = 10.0f;

constexpr int HOLY_AREA_HEAL_AMOUNT = 10;
constexpr float HOLY_AREA_DURATION = 10.0f;
constexpr float HOLY_AREA_EFFECT_RATE = 1.0f;

constexpr float FURY_ROAR_DURATION = 8.0f;
constexpr float FURY_ROAR_ACCELERATE = 2.0f;

constexpr float STELTH_DURATION = 7.0f;

enum class SCENE_TYPE : char
{
	LOGIN_SCENE,
	LOBBY_SCENE,
	PLAYGMAE_SCENE,
	GAMEOVER_SCENE
};

enum class CHARACTER_TYPE : char
{
	WARRIOR,
	BERSERKER,
	ASSASSIN,
	PRIEST,
	COUNT,
	NON
};

enum class SKILL_TYPE : char
{
	NON,
	SWORD_WAVE,
	HOLY_AREA,
	FURY_ROAR,
	STEALTH
};

enum class EFFECT_TYPE : char
{
	NON,
	HOLY_EFFECT,
	FURY_ROAR_EFFECT,
	STEALTH_EFFECT,
	//POISON_FOG_EFFECT, // POISON_FOG_DEACT_AREA���θ� ����Ʈ ��Ʈ��
	PICKING_EFFECT // ���� Ŭ���̾�Ʈ ȭ�鿡�� Ȱ��ȭ
};

// aiModelData�� AnimActionType�� 1:1 ��Ī��
enum class MOTION_TYPE : char
{
	IDLE,
	WALK,
	ATTACK,
	IMPACT, // be attacked
	DIEING,
	SKILL_POSE,
	FREE_MOTION,
	COUNT,
	NON
};

// ĳ���� ����
// ����, ����, �Ͻù���(���ؿ� ����), ����
enum class PLAYER_STATE : char
{
	NON,
	ACT_STEALTH,
	ACT_INVINCIBILITY,
	ACT_SEMI_INVINCIBILITY,
	ACT_DIE
};

enum class ANIM_NOTIFY_TYPE : char
{
	NON,
	WARRIOR_NORMAL_ATTACK_OBJ_GEN,
	WARRIOR_SKILL_SWORD_WAVE_OBJ_GEN,
	BERSERKER_NORMAL_ATTACK_OBJ_GEN,
	BERSERKER_SKILL_FURY_ROAR_ACT,
	ASSASSIN_NORMAL_ATTACK_OBJ_GEN,
	ASSASSIN_SKILL_STEALTH_ACT,
	PRIEST_NORMAL_ATTACK_OBJ_GEN,
	PRIEST_SKILL_HOLY_AREA_OBJ_GEN
};