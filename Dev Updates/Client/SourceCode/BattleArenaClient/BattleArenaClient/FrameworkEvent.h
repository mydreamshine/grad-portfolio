#pragma once
#include <chrono>
#include <queue>
#include <memory>

// Event Type
enum class FRAMEWORK_EVENT_TYPE : char
{
	DO_DIRECT,          // 즉시 실행
	DO_AFTER_SOME_TIME  // 일정 시간 후 실행
};

enum class FRAMEWORK_EVENT_DATA_TYPE : char
{
	OBJ_TRANSFORM,
	PLAYER_SPAWN_INFO,
	CHARACTER_MOTION_INFO,
	DONE_CHARACTER_MOTION_INFO,
	ACT_ANIM_NOTIFY_INFO,
	PLAYER_STATE_INFO,
	DEACT_PGAREA, // InAactive Poison Gas Area
	LOGIN_INFO,
	USER_INFO,
	SETTING_CHAT_LOG,
	SENDING_CHAT_LOG,
	KDA_SCORE,
	KILL_LOG,
	GAME_MATCHING,
	GAME_PLAY_TIME_LIMIT,
	NORMAL_ATTACK_INFO,
	SKILL_USE_INFO,
	SKILL_OBJ_SPAWN_INFO,
	EFFECT_OBJ_SPAWN_INFO,
	NORMAL_ATTACK_OBJ_SPAWN_INFO,
	PLAYER_HP,
	MATCH_STATISTIC_INFO,
	MOVE_INFO,
	ROTATION_INFO,
	IN_GAME_TEAM_SCORE_INFO,
};

// 오브젝트 성향
// 적이 사용한 공격스킬 오브젝트일 경우에만 피해를 입는다던지
// 아군이 사용한 힐스킬 오브젝트일 경우에만 힐링이 된다던지.
enum class OBJECT_PROPENSITY : char
{
	ALLIES, // 아군
	ENEMY,   // 적
	NON
};

enum class CHARACTER_TYPE : char
{
	WARRIOR = 0,
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
	IDLE = 0,
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

namespace AnimNotifyTime
{
	constexpr float Warrior_NormalAttackObjGenTiming = 0.6f;
	constexpr float Warrior_SkillObjGenTiming = 0.875f;

	constexpr float Berserker_NormalAttackObjGenTiming = 0.913f;
	constexpr float Berserker_SkillObjGenTiming = 0.242f;

	constexpr float Assassin_NormalAttackObjGenTiming = 0.501f;
	constexpr float Assassin_SkillObjGenTiming = 0.281f;

	constexpr float Priest_NormalAttackObjGenTiming = 0.572f;
	constexpr float Priest_SkillObjGenTiming = 0.994f;
}
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

#pragma pack(push, 1)
struct EVENT_DATA
{
	FRAMEWORK_EVENT_DATA_TYPE EventType;
	//BYTE* DataByteStream = nullptr;
};

// Scale, Rotation(Euler), Position
struct EVENT_DATA_OBJ_TRANSFORM : EVENT_DATA
{
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT3 RotationEuler;
	DirectX::XMFLOAT3 Position;
};

struct EVENT_DATA_PLAYER_SPAWN_INFO : EVENT_DATA_OBJ_TRANSFORM
{
	std::wstring      Name;
	CHARACTER_TYPE    CharacterType;
	OBJECT_PROPENSITY Propensity;
	bool              IsMainCharacter;
};

struct EVENT_DATA_SKILL_USE_INFO : EVENT_DATA
{
	float      Character_Yaw_angle;
};

struct EVENT_DATA_SKILL_OBJ_SPAWN_INFO : EVENT_DATA_OBJ_TRANSFORM
{
	SKILL_TYPE        SkillType;
	OBJECT_PROPENSITY Propensity;
};

struct EVENT_DATA_NORMAL_ATTACK_INFO : EVENT_DATA
{
	float Character_Yaw_angle;
};

struct EVENT_DATA_EFFECT_OBJ_SPAWN_INFO : EVENT_DATA
{
	DirectX::XMFLOAT3 Position;
	EFFECT_TYPE       EffectType;
};

struct EVENT_DATA_NORMAL_ATTACK_OBJ_SPAWN_INFO : EVENT_DATA_OBJ_TRANSFORM
{
	CHARACTER_TYPE    AttackOrder;
	OBJECT_PROPENSITY Propensity;
};

struct EVENT_DATA_CHARACTER_MOTION_INFO : EVENT_DATA
{
	MOTION_TYPE MotionType;
	SKILL_TYPE  SkillMotionType;
	float       MotionSpeed;
};

struct EVENT_DATA_DONE_CHARACTER_MOTION_INFO : EVENT_DATA
{
	MOTION_TYPE MotionType;
};

struct EVENT_DATA_ACT_ANIM_NOTIFY : EVENT_DATA
{
	ANIM_NOTIFY_TYPE AnimNotifyType;
};

struct EVENT_DATA_PLAYER_STATE_INFO : EVENT_DATA
{
	PLAYER_STATE PlayerState;
};

struct EVENT_DATA_POISON_FOG_DEACT_AREA : EVENT_DATA
{
	RECT Area;
};

struct EVENT_DATA_USER_INFO : EVENT_DATA
{
	std::wstring UserName;
	int          UserRank;
};

struct EVENT_DATA_KDA_SCORE : EVENT_DATA
{
	unsigned char Count_Kill;
	unsigned char Count_Death;
	unsigned char Count_Assistance;
};

struct EVENT_DATA_KILL_LOG : EVENT_DATA
{
	short kill_player_id;
	short death_player_id;
};

struct EVENT_DATA_CHAT_LOG : EVENT_DATA
{
	std::wstring Message;
};

struct EVENT_DATA_GAME_PLAY_TIME_LIMIT : EVENT_DATA
{
	unsigned int Sec;
};

struct EVENT_DATA_PLAYER_HP : EVENT_DATA
{
	int HP;
};

struct EVENT_DATA_MATCH_STATISTIC_INFO : EVENT_DATA
{
	std::wstring  UserName;
	int           UserRank;
	unsigned char Count_Kill;
	unsigned char Count_Death;
	unsigned char Count_Assistance;
	int           TotalScore_Damage;
	int           TotalScore_Heal;
	CHARACTER_TYPE PlayedCharacterType;
};

struct EVENT_DATA_LOGIN_INFO : EVENT_DATA
{
	std::wstring ID;
	std::wstring Password;
};

struct EVENT_DATA_GAME_MATCHING : EVENT_DATA
{
	CHARACTER_TYPE SelectedCharacter;
};

struct EVENT_DATA_SENDING_CHAT_LOG : EVENT_DATA
{
	std::wstring Message;
};

struct EVENT_DATA_TRY_MATCH_LOGIN : EVENT_DATA
{
	char character_type;
	std::wstring user_name;
};

namespace CharacterSpeed
{
	constexpr float Warrior_Speed = 360.0f;
	constexpr float Berserker_Speed = 300.0f;
	constexpr float Assassin_Speed = 380.0f;
	constexpr float Priest_Speed = 360.0f;
}
struct EVENT_DATA_MOVE_INFO : EVENT_DATA
{
	float MoveDirection_Yaw_angle;
};

struct EVENT_DATA_ACCESS_MATCH : EVENT_DATA
{
	int room_id;
};

struct EVENT_DATA_IN_GAME_TEAM_SCORE : EVENT_DATA
{
	unsigned char InGameScore_Team;
};


#pragma pack(pop)

// Type, Act_Object, Ref_Object, Act_Place, Command
// 어떤 유형으로, 누가, 무엇을(무엇으로부터), 어디서, 어떻게 하는가
struct EVENT
{
	FRAMEWORK_EVENT_TYPE Type;       // 즉시 수행해야 할 이벤트(DIRECT)인가, 특정 시간 후 수행해야 할 이벤트(TIME)인가
	int        Act_Object; // 이벤트를 수행할 오브젝트      (Who, 누가)
	int        Ref_Object; // 이벤트에 참조되는 오브젝트    (What, 무엇을) 또는 (From, 무엇으로부터)
	char       Act_Place;  // 이벤트를 수행할 장소          (Where, 어디서)
	char       Command;    // 수행해야 할 이벤트는 무엇인가 (How Do, 어떻게 하는 가)
	std::unique_ptr<EVENT_DATA> Data = nullptr; // 이벤트에 쓰일 데이터

	EVENT() = default;
	EVENT(FRAMEWORK_EVENT_TYPE type, int act_object, int ref_object, char act_place, char command) :
		Type(type), Act_Object(act_object), Ref_Object(ref_object), Act_Place(act_place), Command(command) {}

	virtual EVENT& operator=(EVENT& other)
	{
		EVENT::Move_Property(other);
		return *this;
	}

protected:
	void Move_Property(EVENT& other)
	{
		this->Type = other.Type;
		this->Act_Object = other.Act_Object;
		this->Ref_Object = other.Ref_Object;
		this->Act_Place = other.Act_Place;
		this->Command = other.Command;
		this->Data = std::move(other.Data);
		other.Data = nullptr;
	}
};


// Type, Do_Object, Ref_Object, Act_Place, Command, ExecutionTime
// 어떤 유형으로, 누가, 무엇을(무엇으로부터), 어디서, 어떻게, 언제 하는가
struct TIME_EVENT : EVENT
{
	std::chrono::high_resolution_clock::time_point ExecutionTime; // 이벤트를 수행할 시간

	TIME_EVENT() = default;
	TIME_EVENT(
		FRAMEWORK_EVENT_TYPE type, int act_object, int ref_object, char act_place, char command,
		std::chrono::high_resolution_clock::time_point ex_time) :
		EVENT(type, act_object, ref_object, act_place, command), ExecutionTime(ex_time) {}
	virtual ~TIME_EVENT() { EVENT::~EVENT(); }

	virtual TIME_EVENT& operator=(TIME_EVENT& other)
	{
		EVENT::Move_Property(other);
		this->ExecutionTime = other.ExecutionTime;
		return *this;
	}
};


///////////////////////////////////// 이벤트 플레이스 /////////////////////////////////////
// Framework Event Execution Place
#define FEP_LOGIN_SCENE    0x0
#define FEP_LOBY_SCENE     0x1
#define FEP_PLAYGMAE_SCENE 0x2
#define FEP_GAMEOVER_SCENE 0x3
///////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////// 이벤트 커맨드 //////////////////////////////////////
//// Framework Event Command (Framework 외부에서 발생한 이벤트) ////
enum FEC {
    FEC_CHANGE_SCENE,

    // Scene Object Ref
    FEC_SPAWN_PLAYER,
    FEC_SPAWN_NORMAL_ATTACK_OBJ,
    FEC_SPAWN_SKILL_OBJ,
    FEC_SPAWN_EFFECT_OBJ,
    FEC_SET_TRANSFORM_WORLD_OBJECT,
    FEC_SET_CHARACTER_MOTION,
    FEC_SET_PLAYER_STATE,
    FEC_UPDATE_POISON_FOG_DEACT_AREA,
    FEC_DEACTIVATE_OBJ,

    // UI Info Ref
    FEC_SET_USER_INFO,
    FEC_SET_KDA_SCORE,
    FEC_SET_KILL_LOG,
    FEC_SET_CHAT_LOG,
    FEC_SET_GAME_PLAY_TIME_LIMIT,
    FEC_SET_PLAYER_HP,
    FEC_SET_MATCH_STATISTIC_INFO,
	FEC_SET_IN_GAME_TEAM_SCORE,

	// For lobby.
	FEC_MATCH_ENQUEUE,
	FEC_MATCH_DEQUEUE,
	FEC_ACCESS_MATCH,
	FEC_TRY_MATCH_LOGIN,
	FEC_SEND_IN_LOBY_CHAT,
    ////////////////////////////////////////////////////////////////////

    //// Framework Event Command (Framework 내부에서 발생한 이벤트) /////
    FEC_TRY_LOBBY_LOGIN,
    FEC_GET_USER_INFO,
    FEC_TRY_GAME_MATCHING,
    FEC_SEND_IN_GAME_CHAT,
    FEC_TRY_MOVE_CHARACTER,
    FEC_TRY_MOVESTOP_CHARACTER,
    FEC_TRY_NORMAL_ATTACK,
    FEC_TRY_USE_SKILL,
    FEC_DONE_CHARACTER_MOTION,
    FEC_ACTIVATE_ANIM_NOTIFY,
    FEC_SPAWN_PICKING_EFFECT_OBJ, //현재 클라이언트 화면에서만 스폰
    FEC_TRY_RETURN_LOBY
};
//#define FEC_CHANGE_SCENE                        0x00

//// Scene Object Ref
//#define FEC_SPAWN_PLAYER                        0x01
//#define FEC_SPAWN_NORMAL_ATTACK_OBJ             0x02
//#define FEC_SPAWN_SKILL_OBJ                     0x03
//#define FEC_SPAWN_EFFECT_OBJ                    0x04
//#define FEC_SET_TRANSFORM_WORLD_OBJECT          0x05
//#define FEC_SET_CHARACTER_MOTION                0x06
//#define FEC_SET_PLAYER_STATE                    0x07
//#define FEC_UPDATE_POISON_FOG_DEACT_AREA        0x08
//#define FEC_DEACTIVATE_OBJ                      0x09
//
//// UI Info Ref
//#define FEC_SET_USER_INFO                       0x0A
//#define FEC_SET_KDA_SCORE                       0x0B
//#define FEC_SET_KILL_LOG                        0x0C
//#define FEC_SET_CHAT_LOG                        0x0D
//#define FEC_SET_GAME_PLAY_TIME_LIMIT            0x0E
//#define FEC_SET_PLAYER_HP                       0x0F
//#define FEC_SET_MATCH_STATISTIC_INFO            0x10
//////////////////////////////////////////////////////////////////////
//
////// Framework Event Command (Framework 내부에서 발생한 이벤트) /////
//#define FEC_TRY_LOGIN                           0x11
//#define FEC_GET_USER_INFO                       0x12
//#define FEC_TRY_GAME_MATCHING                   0x13
//#define FEC_SEND_CHAT_LOG                       0x14
//#define FEC_TRY_MOVE_CHARACTER                  0x15
//#define FEC_TRY_MOVESTOP_CHARACTER              0x16
//#define FEC_TRY_NORMAL_ATTACK                   0x17
//#define FEC_TRY_USE_SKILL                       0x18
//#define FEC_DONE_CHARACTER_MOTION               0x19
//#define FEC_ACTIVATE_ANIM_NOTIFY                0x1A
//#define FEC_SPAWN_PICKING_EFFECT_OBJ            0x1B // 현재 클라이언트 화면에서만 스폰
//#define FEC_TRY_RETURN_LOBY                     0x1C
///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////



class EventManager
{
public:

	void ReservateEvent_ChangeScene(std::queue<std::unique_ptr<EVENT>>& Events, char Act_Place)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, Act_Place, FEC_CHANGE_SCENE);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnPlayer(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		std::wstring Name,
		CHARACTER_TYPE CharacterType,
		DirectX::XMFLOAT3 Scale, DirectX::XMFLOAT3 RotationEuler, DirectX::XMFLOAT3 Position,
		OBJECT_PROPENSITY Propensity,
		bool IsMainCharacter = false)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_PLAYER);
		auto newEventData = std::make_unique<EVENT_DATA_PLAYER_SPAWN_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::PLAYER_SPAWN_INFO;
		newEventData->Name = Name;
		newEventData->CharacterType = CharacterType;
		newEventData->Scale = Scale;
		newEventData->RotationEuler = RotationEuler;
		newEventData->Position = Position;
		newEventData->Propensity = Propensity;
		newEventData->IsMainCharacter = IsMainCharacter;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnNormalAttackObj(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		CHARACTER_TYPE AttackOrder,
		DirectX::XMFLOAT3 Scale, DirectX::XMFLOAT3 RotationEuler, DirectX::XMFLOAT3 Position,
		OBJECT_PROPENSITY Propensity)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_NORMAL_ATTACK_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_NORMAL_ATTACK_OBJ_SPAWN_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::NORMAL_ATTACK_OBJ_SPAWN_INFO;
		newEventData->AttackOrder = AttackOrder;
		newEventData->Scale = Scale;
		newEventData->RotationEuler = RotationEuler;
		newEventData->Position = Position;
		newEventData->Propensity = Propensity;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnSkillObj(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		SKILL_TYPE SkillType,
		DirectX::XMFLOAT3 Scale, DirectX::XMFLOAT3 RotationEuler, DirectX::XMFLOAT3 Position,
		OBJECT_PROPENSITY Propensity)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_SKILL_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_SKILL_OBJ_SPAWN_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::SKILL_OBJ_SPAWN_INFO;
		newEventData->SkillType = SkillType;
		newEventData->Scale = Scale;
		newEventData->RotationEuler = RotationEuler;
		newEventData->Position = Position;
		newEventData->Propensity = Propensity;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnEffectObj(std::queue<std::unique_ptr<EVENT>>& Events,
		EFFECT_TYPE EffectType,
		DirectX::XMFLOAT3 Position)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_EFFECT_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_EFFECT_OBJ_SPAWN_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::EFFECT_OBJ_SPAWN_INFO;
		newEventData->EffectType = EffectType;
		newEventData->Position = Position;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetTransform(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		DirectX::XMFLOAT3 Scale, DirectX::XMFLOAT3 RotationEuler, DirectX::XMFLOAT3 Position)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_TRANSFORM_WORLD_OBJECT);
		auto newEventData = std::make_unique<EVENT_DATA_OBJ_TRANSFORM>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::OBJ_TRANSFORM;
		newEventData->Scale = Scale;
		newEventData->RotationEuler = RotationEuler;
		newEventData->Position = Position;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetCharacterMotion(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		MOTION_TYPE MotionType, float MotionSpeed = 1.0f, SKILL_TYPE SkillMotionType = SKILL_TYPE::NON)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_CHARACTER_MOTION);
		auto newEventData = std::make_unique<EVENT_DATA_CHARACTER_MOTION_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::CHARACTER_MOTION_INFO;
		newEventData->MotionType = MotionType;
		newEventData->SkillMotionType = SkillMotionType;
		newEventData->MotionSpeed = MotionSpeed;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetPlayerState(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		PLAYER_STATE PlayerState)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_PLAYER_STATE);
		auto newEventData = std::make_unique<EVENT_DATA_PLAYER_STATE_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::PLAYER_STATE_INFO;
		newEventData->PlayerState = PlayerState;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_UpdateDeActPGArea(std::queue<std::unique_ptr<EVENT>>& Events,
		RECT DeActPGArea)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_UPDATE_POISON_FOG_DEACT_AREA);
		auto newEventData = std::make_unique<EVENT_DATA_POISON_FOG_DEACT_AREA>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::DEACT_PGAREA;
		newEventData->Area = DeActPGArea;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_DeactivateObj(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_DEACTIVATE_OBJ);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetUserInfo(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring UserName, int UserRank)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_SET_USER_INFO);
		auto newEventData = std::make_unique<EVENT_DATA_USER_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::USER_INFO;
		newEventData->UserName = UserName;
		newEventData->UserRank = UserRank;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetKDAScore(std::queue<std::unique_ptr<EVENT>>& Events,
		unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_KDA_SCORE);
		auto newEventData = std::make_unique<EVENT_DATA_KDA_SCORE>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::KDA_SCORE;
		newEventData->Count_Kill = Count_Kill;
		newEventData->Count_Death = Count_Death;
		newEventData->Count_Assistance = Count_Assistance;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetKillLog(std::queue<std::unique_ptr<EVENT>>& Events,
		short kill_player_id, short death_player_id)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_KILL_LOG);
		auto newEventData = std::make_unique<EVENT_DATA_KILL_LOG>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::KILL_LOG;
		newEventData->kill_player_id = kill_player_id;
		newEventData->death_player_id = death_player_id;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetChatLog(std::queue<std::unique_ptr<EVENT>>& Events, char Act_Place,
		std::wstring Message)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, Act_Place, FEC_SET_CHAT_LOG);
		auto newEventData = std::make_unique<EVENT_DATA_CHAT_LOG>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::SETTING_CHAT_LOG;
		newEventData->Message = Message;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_GamePlayTimeLimit(std::queue<std::unique_ptr<EVENT>>& Events,
		unsigned int Sec)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_GAME_PLAY_TIME_LIMIT);
		auto newEventData = std::make_unique<EVENT_DATA_GAME_PLAY_TIME_LIMIT>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::GAME_PLAY_TIME_LIMIT;
		newEventData->Sec = Sec;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetPlayerHP(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		int HP)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_PLAYER_HP);
		auto newEventData = std::make_unique<EVENT_DATA_PLAYER_HP>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::PLAYER_HP;
		newEventData->HP = HP;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetMatchStatisticInfo(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring UserName,
		int UserRank,
		unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance,
		int  TotalScore_Damage, int  TotalScore_Heal,
		CHARACTER_TYPE PlayedCharacterType)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_GAMEOVER_SCENE, FEC_SET_MATCH_STATISTIC_INFO);
		auto newEventData = std::make_unique<EVENT_DATA_MATCH_STATISTIC_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::MATCH_STATISTIC_INFO;
		newEventData->UserName = UserName;
		newEventData->UserRank = UserRank;
		newEventData->Count_Kill = Count_Kill;
		newEventData->Count_Death = Count_Death;
		newEventData->Count_Assistance = Count_Assistance;
		newEventData->TotalScore_Damage = TotalScore_Damage;
		newEventData->TotalScore_Heal = TotalScore_Heal;
		newEventData->PlayedCharacterType = PlayedCharacterType;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetInGameTeamScore(std::queue<std::unique_ptr<EVENT>>& Events,
		unsigned char InGameScore_Team)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_IN_GAME_TEAM_SCORE);
		auto newEventData = std::make_unique<EVENT_DATA_IN_GAME_TEAM_SCORE>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::IN_GAME_TEAM_SCORE_INFO;
		newEventData->InGameScore_Team = InGameScore_Team;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryLogin(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring ID, std::wstring Password)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOGIN_SCENE, FEC_TRY_LOBBY_LOGIN);
		auto newEventData = std::make_unique<EVENT_DATA_LOGIN_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::LOGIN_INFO;
		newEventData->ID = ID;
		newEventData->Password = Password;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_GetUserInfo(std::queue<std::unique_ptr<EVENT>>& Events)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_GET_USER_INFO);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryGameMatching(std::queue<std::unique_ptr<EVENT>>& Events,
		CHARACTER_TYPE SelectedCharacter)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_TRY_GAME_MATCHING);
		auto newEventData = std::make_unique<EVENT_DATA_GAME_MATCHING>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::GAME_MATCHING;
		newEventData->SelectedCharacter = SelectedCharacter;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SendChatLog(std::queue<std::unique_ptr<EVENT>>& Events, char Act_Place,
		std::wstring Message)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, Act_Place, FEC_SEND_IN_LOBY_CHAT);
		if (Act_Place == FEP_LOBY_SCENE) newEvent->Command = FEC_SEND_IN_LOBY_CHAT;
		else if (Act_Place == FEP_PLAYGMAE_SCENE) newEvent->Command = FEC_SEND_IN_GAME_CHAT;
		auto newEventData = std::make_unique<EVENT_DATA_SENDING_CHAT_LOG>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::SENDING_CHAT_LOG;
		newEventData->Message = Message;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryMoveCharacter(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		float MoveDirection_Yaw_angle)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_MOVE_CHARACTER);
		auto newEventData = std::make_unique<EVENT_DATA_MOVE_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::MOVE_INFO;
		newEventData->MoveDirection_Yaw_angle = MoveDirection_Yaw_angle;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryMoveStopCharacter(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_MOVESTOP_CHARACTER);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryNormalAttack(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		float Character_Yaw_angle)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_NORMAL_ATTACK);
		auto newEventData = std::make_unique<EVENT_DATA_NORMAL_ATTACK_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::NORMAL_ATTACK_INFO;
		newEventData->Character_Yaw_angle = Character_Yaw_angle;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryUseSkill(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		float Character_Yaw_angle)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_USE_SKILL);
		auto newEventData = std::make_unique<EVENT_DATA_SKILL_USE_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::SKILL_USE_INFO;
		newEventData->Character_Yaw_angle = Character_Yaw_angle;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_DoneCharacterMotion(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		MOTION_TYPE MotionType)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_DONE_CHARACTER_MOTION);
		auto newEventData = std::make_unique<EVENT_DATA_DONE_CHARACTER_MOTION_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::DONE_CHARACTER_MOTION_INFO;
		newEventData->MotionType = MotionType;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_ActivatedAnimNotify(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		ANIM_NOTIFY_TYPE AnimNotifyType)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_ACTIVATE_ANIM_NOTIFY);
		auto newEventData = std::make_unique<EVENT_DATA_ACT_ANIM_NOTIFY>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::ACT_ANIM_NOTIFY_INFO;
		newEventData->AnimNotifyType = AnimNotifyType;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnPickingEffectObj(std::queue<std::unique_ptr<EVENT>>& Events,
		DirectX::XMFLOAT3 Position)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_PICKING_EFFECT_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_EFFECT_OBJ_SPAWN_INFO>();
		newEventData->EventType = FRAMEWORK_EVENT_DATA_TYPE::EFFECT_OBJ_SPAWN_INFO;
		newEventData->EffectType = EFFECT_TYPE::PICKING_EFFECT;
		newEventData->Position = Position;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryReturnLoby(std::queue<std::unique_ptr<EVENT>>& Events)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_GAMEOVER_SCENE, FEC_TRY_RETURN_LOBY);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_MatchEnqueue(std::queue<std::unique_ptr<EVENT>>& Events)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_MATCH_ENQUEUE);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_MatchDequeue(std::queue<std::unique_ptr<EVENT>>& Events)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_MATCH_DEQUEUE);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_AccessMatch(std::queue<std::unique_ptr<EVENT>>& Events, int room_id)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_ACCESS_MATCH);
		auto newEventData = std::make_unique<EVENT_DATA_ACCESS_MATCH>();
		newEventData->room_id = room_id;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryMatchLogin(std::queue<std::unique_ptr<EVENT>>& Events, std::wstring user_name, char character_type)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(FRAMEWORK_EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_TRY_MATCH_LOGIN);
		auto newEventData = std::make_unique<EVENT_DATA_TRY_MATCH_LOGIN>();
		newEventData->character_type = character_type;
		newEventData->user_name = user_name;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}
};
