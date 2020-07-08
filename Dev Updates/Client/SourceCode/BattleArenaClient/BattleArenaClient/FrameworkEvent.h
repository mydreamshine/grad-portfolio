#pragma once
#include <chrono>
#include <queue>
#include <memory>

// Event Type
enum class EVENT_TYPE : char
{
	DO_DIRECT,          // ��� ����
	DO_AFTER_SOME_TIME  // ���� �ð� �� ����
};

enum class EVENT_DATA_TYPE : char
{
	OBJ_TRANSFORM,
	CHARACTER_SPAWN_INFO,
	CHARACTER_MOTION_INFO,
	DONE_CHARACTER_MOTION_INFO,
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
	SKILL_USE_INFO,
	SKILL_OBJ_SPAWN_INFO,
	EFFECT_OBJ_SPAWN_INFO,
	NORMAL_ATTACK_OBJ_SPAWN_INFO,
	PLAYER_HP,
	MATCH_STATISTIC_INFO,
	MOVE_INFO,
	ROTATION_INFO
};

// ������Ʈ ����
// ���� ����� ���ݽ�ų ������Ʈ�� ��쿡�� ���ظ� �Դ´ٴ���
// �Ʊ��� ����� ����ų ������Ʈ�� ��쿡�� ������ �ȴٴ���.
enum class OBJECT_PROPENSITY : char
{
	NON,
	ALLIES, // �Ʊ�
	ENEMY   // ��
};

enum class CHARACTER_TYPE : char
{
	NON,
	WARRIOR,
	BERSERKER,
	ASSASSIN,
	PRIEST
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

enum class MOTION_TYPE : char
{
	NON,
	IDLE,
	WALK,
	ATTACK,
	IMPACT, // be attacked
	SKILL_POSE,
	DIEING
};

// ĳ���� ����
// ����, ����, �Ͻù���(���ؿ� ����)
enum class PLAYER_STATE : char
{
	NON,
	ACT_STEALTH,
	ACT_INVINCIBILITY,
	ACT_SEMI_INVINCIBILITY,
};

#pragma pack(push, 1)
struct EVENT_DATA
{
	EVENT_DATA_TYPE EventType;
	//BYTE* DataByteStream = nullptr;
};

// Scale, Rotation(Euler), Position
struct EVENT_DATA_OBJ_TRANSFORM : EVENT_DATA
{
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT3 RotationEuler;
	DirectX::XMFLOAT3 Position;
};

struct EVENT_DATA_CHARACTER_SPAWN_INFO : EVENT_DATA_OBJ_TRANSFORM
{
	CHARACTER_TYPE    CharacterType;
	OBJECT_PROPENSITY Propensity;
	bool              IsMainCharacter;
};

struct EVENT_DATA_SKILL_USE_INFO : EVENT_DATA
{
	SKILL_TYPE        SkillType;
};

struct EVENT_DATA_SKILL_OBJ_SPAWN_INFO : EVENT_DATA_OBJ_TRANSFORM
{
	SKILL_TYPE        SkillType;
	OBJECT_PROPENSITY Propensity;
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
};

struct EVENT_DATA_DONE_CHARACTER_MOTION_INFO : EVENT_DATA
{
	MOTION_TYPE MotionType;
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
	int         UserRank;
};

struct EVENT_DATA_KDA_SCORE : EVENT_DATA
{
	unsigned char Count_Kill;
	unsigned char Count_Death;
	unsigned char Count_Assistance;
};

struct EVENT_DATA_KILL_LOG : EVENT_DATA
{
	std::wstring Do_UserName;
	std::wstring Target_UserName;
};

struct EVENT_DATA_CHAT_LOG : EVENT_DATA
{
	std::wstring UserName;
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
	std::wstring UserName;
	int  UserRank;
	char Count_Kill;
	char Count_Death;
	char Count_Assistance;
	int  TotalScore_Damage;
	int  TotalScore_Heal;
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

struct EVENT_DATA_MOVE_INFO : EVENT_DATA
{
	float MoveDirection_Yaw_angle;
};

struct EVENT_DATA_ROTATION_INFO : EVENT_DATA
{
	float Yaw_angle;
};

#pragma pack(pop)

// Type, Act_Object, Ref_Object, Act_Place, Command
// � ��������, ����, ������(�������κ���), ���, ��� �ϴ°�
struct EVENT
{
	EVENT_TYPE Type;       // ��� �����ؾ� �� �̺�Ʈ(DIRECT)�ΰ�, Ư�� �ð� �� �����ؾ� �� �̺�Ʈ(TIME)�ΰ�
	int        Act_Object; // �̺�Ʈ�� ������ ������Ʈ      (Who, ����)
	int        Ref_Object; // �̺�Ʈ�� �����Ǵ� ������Ʈ    (What, ������) �Ǵ� (From, �������κ���)
	char       Act_Place;  // �̺�Ʈ�� ������ ���          (Where, ���)
	char       Command;    // �����ؾ� �� �̺�Ʈ�� �����ΰ� (How Do, ��� �ϴ� ��)
	std::unique_ptr<EVENT_DATA> Data = nullptr; // �̺�Ʈ�� ���� ������

	EVENT() = default;
	EVENT(EVENT_TYPE type, int act_object, int ref_object, char act_place, char command) :
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
// � ��������, ����, ������(�������κ���), ���, ���, ���� �ϴ°�
struct TIME_EVENT : EVENT
{
	std::chrono::high_resolution_clock::time_point ExecutionTime; // �̺�Ʈ�� ������ �ð�

	TIME_EVENT() = default;
	TIME_EVENT(
		EVENT_TYPE type, int act_object, int ref_object, char act_place, char command,
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


///////////////////////////////////// �̺�Ʈ �÷��̽� /////////////////////////////////////
// Framework Event Execution Place
#define FEP_LOGIN_SCENE    0x0
#define FEP_LOBY_SCENE     0x1
#define FEP_PLAYGMAE_SCENE 0x2
#define FEP_GAMEOVER_SCENE 0x3
///////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////// �̺�Ʈ Ŀ�ǵ� //////////////////////////////////////
//// Framework Event Command (Framework �ܺο��� �߻��� �̺�Ʈ) ////
#define FEC_CHANGE_SCENE                        0x00

// Scene Object Ref
#define FEC_SPAWN_CHARACTER                     0x01
#define FEC_SPAWN_NORMAL_ATTACK_OBJ             0x02
#define FEC_SPAWN_SKILL_OBJ                     0x03
#define FEC_SPAWN_EFFECT_OBJ                    0x04
#define FEC_SET_TRANSFORM_WORLD_OBJECT          0x05
#define FEC_SET_CHARACTER_MOTION                0x06
#define FEC_SET_PLAYER_STATE                    0x07
#define FEC_UPDATE_POISON_FOG_DEACT_AREA        0x08
#define FEC_DEACTIVATE_OBJ                      0x09

// UI Info Ref
#define FEC_SET_USER_INFO                       0x0A
#define FEC_SET_KDA_SCORE                       0x0B
#define FEC_SET_KILL_LOG                        0x0C
#define FEC_SET_CHAT_LOG                        0x0D
#define FEC_SET_GAME_PLAY_TIME_LIMIT            0x0E
#define FEC_SET_PLAYER_HP                       0x0F
#define FEC_SET_MATCH_STATISTIC_INFO            0x10
////////////////////////////////////////////////////////////////////

//// Framework Event Command (Framework ���ο��� �߻��� �̺�Ʈ) /////
#define FEC_TRY_LOGIN                           0x11
#define FEC_GET_USER_INFO                       0x12
#define FEC_TRY_GAME_MATCHING                   0x13
#define FEC_SEND_CHAT_LOG                       0x14
#define FEC_TRY_MOVE_CHARACTER                  0x15
#define FEC_TRY_ROTATION_CHARACTER              0x16
#define FEC_TRY_NORMAL_ATTACK                   0x17
#define FEC_TRY_USE_SKILL                       0x18
#define FEC_DONE_CHARACTER_MOTION               0x19
#define FEC_ACTIVATE_ANIM_NOTIFY                0x1A
#define FEC_SPAWN_PICKING_EFFECT_OBJ            0x1B // ���� Ŭ���̾�Ʈ ȭ�鿡���� ����
#define FEC_TRY_RETURN_LOBY                     0x1C
/////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////



class EventManager
{
public:

	void ReservateEvent_ChangeScene(std::queue<std::unique_ptr<EVENT>>& Events, char Act_Place)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, Act_Place, FEC_CHANGE_SCENE);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnCharacter(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		CHARACTER_TYPE CharacterType,
		DirectX::XMFLOAT3 Scale, DirectX::XMFLOAT3 RotationEuler, DirectX::XMFLOAT3 Position,
		OBJECT_PROPENSITY Propensity,
		bool IsMainCharacter = false)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_CHARACTER);
		auto newEventData = std::make_unique<EVENT_DATA_CHARACTER_SPAWN_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::CHARACTER_SPAWN_INFO;
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
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_NORMAL_ATTACK_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_NORMAL_ATTACK_OBJ_SPAWN_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::NORMAL_ATTACK_OBJ_SPAWN_INFO;
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
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_SKILL_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_SKILL_OBJ_SPAWN_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::SKILL_OBJ_SPAWN_INFO;
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
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_EFFECT_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_EFFECT_OBJ_SPAWN_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::EFFECT_OBJ_SPAWN_INFO;
		newEventData->EffectType = EffectType;
		newEventData->Position = Position;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetTransform(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		DirectX::XMFLOAT3 Scale, DirectX::XMFLOAT3 RotationEuler, DirectX::XMFLOAT3 Position)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_TRANSFORM_WORLD_OBJECT);
		auto newEventData = std::make_unique<EVENT_DATA_OBJ_TRANSFORM>();
		newEventData->EventType = EVENT_DATA_TYPE::OBJ_TRANSFORM;
		newEventData->Scale = Scale;
		newEventData->RotationEuler = RotationEuler;
		newEventData->Position = Position;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetCharacterMotion(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		MOTION_TYPE MotionType, SKILL_TYPE SkillMotionType = SKILL_TYPE::NON)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_CHARACTER_MOTION);
		auto newEventData = std::make_unique<EVENT_DATA_CHARACTER_MOTION_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::CHARACTER_MOTION_INFO;
		newEventData->MotionType = MotionType;
		newEventData->SkillMotionType = SkillMotionType;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetPlayerState(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		PLAYER_STATE PlayerState)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_PLAYER_STATE);
		auto newEventData = std::make_unique<EVENT_DATA_PLAYER_STATE_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::PLAYER_STATE_INFO;
		newEventData->PlayerState = PlayerState;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_UpdateDeActPGArea(std::queue<std::unique_ptr<EVENT>>& Events,
		RECT DeActPGArea)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_UPDATE_POISON_FOG_DEACT_AREA);
		auto newEventData = std::make_unique<EVENT_DATA_POISON_FOG_DEACT_AREA>();
		newEventData->EventType = EVENT_DATA_TYPE::DEACT_PGAREA;
		newEventData->Area = DeActPGArea;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_DeactivateObj(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_DEACTIVATE_OBJ);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SetUserInfo(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring UserName, int UserRank)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_SET_USER_INFO);
		auto newEventData = std::make_unique<EVENT_DATA_USER_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::USER_INFO;
		newEventData->UserName = UserName;
		newEventData->UserRank = UserRank;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ResrvateEvent_SetKDAScore(std::queue<std::unique_ptr<EVENT>>& Events,
		unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_KDA_SCORE);
		auto newEventData = std::make_unique<EVENT_DATA_KDA_SCORE>();
		newEventData->EventType = EVENT_DATA_TYPE::KDA_SCORE;
		newEventData->Count_Kill = Count_Kill;
		newEventData->Count_Death = Count_Death;
		newEventData->Count_Assistance = Count_Assistance;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ResrvateEvent_SetKillLog(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring Do_UserName, std::wstring Target_UserName)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_KILL_LOG);
		auto newEventData = std::make_unique<EVENT_DATA_KILL_LOG>();
		newEventData->EventType = EVENT_DATA_TYPE::KILL_LOG;
		newEventData->Do_UserName = Do_UserName;
		newEventData->Target_UserName = Target_UserName;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ResrvateEvent_SetChatLog(std::queue<std::unique_ptr<EVENT>>& Events, char Act_Place,
		std::wstring UserName, std::wstring Message)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, Act_Place, FEC_SET_CHAT_LOG);
		auto newEventData = std::make_unique<EVENT_DATA_CHAT_LOG>();
		newEventData->EventType = EVENT_DATA_TYPE::SETTING_CHAT_LOG;
		newEventData->UserName = UserName;
		newEventData->Message = Message;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ResrvateEvent_GamePlayTimeLimit(std::queue<std::unique_ptr<EVENT>>& Events,
		unsigned int Sec)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SET_GAME_PLAY_TIME_LIMIT);
		auto newEventData = std::make_unique<EVENT_DATA_GAME_PLAY_TIME_LIMIT>();
		newEventData->EventType = EVENT_DATA_TYPE::GAME_PLAY_TIME_LIMIT;
		newEventData->Sec = Sec;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ResrvateEvent_SetPlayerHP(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		int HP)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_SET_PLAYER_HP);
		auto newEventData = std::make_unique<EVENT_DATA_PLAYER_HP>();
		newEventData->EventType = EVENT_DATA_TYPE::PLAYER_HP;
		newEventData->HP = HP;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ResrvateEvent_SetMatchStatisticInfo(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring UserName,
		int UserRank,
		unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance,
		int  TotalScore_Damage, int  TotalScore_Heal,
		CHARACTER_TYPE PlayedCharacterType)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_GAMEOVER_SCENE, FEC_SET_MATCH_STATISTIC_INFO);
		auto newEventData = std::make_unique<EVENT_DATA_MATCH_STATISTIC_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::MATCH_STATISTIC_INFO;
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

	void ReseravateEvent_TryLogin(std::queue<std::unique_ptr<EVENT>>& Events,
		std::wstring ID, std::wstring Password)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOGIN_SCENE, FEC_TRY_LOGIN);
		auto newEventData = std::make_unique<EVENT_DATA_LOGIN_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::LOGIN_INFO;
		newEventData->ID = ID;
		newEventData->Password = Password;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReseravateEvent_GetUserInfo(std::queue<std::unique_ptr<EVENT>>& Events)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_GET_USER_INFO);
		Events.push(std::move(newEvent));
	}

	void ReseravateEvent_TryGameMatching(std::queue<std::unique_ptr<EVENT>>& Events,
		CHARACTER_TYPE SelectedCharacter)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_LOBY_SCENE, FEC_TRY_GAME_MATCHING);
		auto newEventData = std::make_unique<EVENT_DATA_GAME_MATCHING>();
		newEventData->EventType = EVENT_DATA_TYPE::GAME_MATCHING;
		newEventData->SelectedCharacter = SelectedCharacter;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReseravateEvent_SendChatLog(std::queue<std::unique_ptr<EVENT>>& Events, char Act_Place,
		std::wstring Message)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, Act_Place, FEC_SEND_CHAT_LOG);
		auto newEventData = std::make_unique<EVENT_DATA_SENDING_CHAT_LOG>();
		newEventData->EventType = EVENT_DATA_TYPE::SENDING_CHAT_LOG;
		newEventData->Message = Message;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryMoveCharacter(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		float MoveDirection_Yaw_angle)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_MOVE_CHARACTER);
		auto newEventData = std::make_unique<EVENT_DATA_MOVE_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::MOVE_INFO;
		newEventData->MoveDirection_Yaw_angle = MoveDirection_Yaw_angle;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryRotationCharacter(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		float Yaw_angle)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_ROTATION_CHARACTER);
		auto newEventData = std::make_unique<EVENT_DATA_ROTATION_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::MOVE_INFO;
		newEventData->Yaw_angle = Yaw_angle;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryNormalAttack(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_NORMAL_ATTACK);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_TryUseSkill(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		SKILL_TYPE SkillType)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_TRY_USE_SKILL);
		auto newEventData = std::make_unique<EVENT_DATA_SKILL_USE_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::SKILL_USE_INFO;
		newEventData->SkillType = SkillType;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_DoneCharacterMotion(std::queue<std::unique_ptr<EVENT>>& Events, int Act_Object,
		MOTION_TYPE MotionType)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, Act_Object, -1, FEP_PLAYGMAE_SCENE, FEC_DONE_CHARACTER_MOTION);
		auto newEventData = std::make_unique<EVENT_DATA_DONE_CHARACTER_MOTION_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::DONE_CHARACTER_MOTION_INFO;
		newEventData->MotionType = MotionType;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReservateEvent_SpawnPickingEffectObj(std::queue<std::unique_ptr<EVENT>>& Events,
		DirectX::XMFLOAT3 Position)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_PLAYGMAE_SCENE, FEC_SPAWN_PICKING_EFFECT_OBJ);
		auto newEventData = std::make_unique<EVENT_DATA_EFFECT_OBJ_SPAWN_INFO>();
		newEventData->EventType = EVENT_DATA_TYPE::EFFECT_OBJ_SPAWN_INFO;
		newEventData->EffectType = EFFECT_TYPE::PICKING_EFFECT;
		newEventData->Position = Position;
		newEvent->Data = std::move(newEventData);
		Events.push(std::move(newEvent));
	}

	void ReseravateEvent_TryReturnLoby(std::queue<std::unique_ptr<EVENT>>& Events)
	{
		std::unique_ptr<EVENT> newEvent = std::make_unique<EVENT>(EVENT_TYPE::DO_DIRECT, -1, -1, FEP_GAMEOVER_SCENE, FEC_TRY_RETURN_LOBY);
		Events.push(std::move(newEvent));
	}
};
