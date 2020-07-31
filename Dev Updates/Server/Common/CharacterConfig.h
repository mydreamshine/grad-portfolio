#pragma once

constexpr int BASIC_HP = 100;
constexpr float BASIC_VELOCITY = 400.0f;
constexpr float RESPAWN_TIME = 5.0f;
constexpr float SEMI_INVINCIBLE_TIME = 3.0f;

constexpr int NORMAL_ATTACK_DAMAGE = 10;
constexpr float NORMAL_ATTACK_VELOCITY = 2100.0f;
constexpr float NORMAL_ATTACK_DURATION = 5.0f;

constexpr int WARRIOR_MAX_HP = 100;
constexpr float WARRIOR_MOVEMENT = 360.0f;
constexpr int WARRIOR_ATTACK_DAMAGE = 30;
constexpr int SWORD_WAVE_DAMAGE = 30;
constexpr float SWORD_WAVE_DURATION = 10.0f;

constexpr int PRIEST_MAX_HP = 80;
constexpr float PRIEST_MOVEMENT = 360.0f;
constexpr int PRIEST_ATTACK_DAMAGE = 15;
constexpr int HOLY_AREA_HEAL_AMOUNT = 10;
constexpr float HOLY_AREA_DURATION = 10.0f;
constexpr float HOLY_AREA_EFFECT_RATE = 1.0f;

constexpr int BERSERKER_MAX_HP = 150;
constexpr float BERSERKER_MOVEMENT = 300.0;
constexpr int BERSERKER_ATTACK_DAMAGE = 40;
constexpr float FURY_ROAR_DURATION = 8.0f;
constexpr float FURY_ROAR_ACCELERATE = 2.0f;

constexpr int ASSASSIN_MAX_HP = 100;
constexpr float ASSASSIN_MOVEMENT = 380.0f;
constexpr int ASSASSIN_ATTACK_DAMAGE = 30;
constexpr float STELTH_DURATION = 7.0f;

constexpr float GAME_TIME_LIMIT = 180.0f;
constexpr float GAS_DECREASE_TIME = 5.0f;
constexpr float GAS_EFFECT_TIME = 1.0f;
constexpr int GAS_DAMAGE = 5;
constexpr int SAFE_AREA_LEFT = -3423;
constexpr int SAFE_AREA_TOP = 4290;
constexpr int SAFE_AREA_RIGHT = 4577;
constexpr int SAFE_AREA_BOTTOM = -3710;

constexpr float ASSIST_TIME = 5.0f;

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
	//POISON_FOG_EFFECT, // POISON_FOG_DEACT_AREA으로만 이펙트 컨트롤
	PICKING_EFFECT // 현재 클리이언트 화면에만 활성화
};

// aiModelData의 AnimActionType과 1:1 매칭됨
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

// 캐릭터 상태
// 은신, 무적, 일시무적(피해에 의한), 죽음
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