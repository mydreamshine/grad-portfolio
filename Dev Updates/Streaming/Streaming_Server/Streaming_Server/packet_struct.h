#pragma once

//////////////////// 통신 데이터 타입 + 이벤트 타입(혹은 State Machine) ///////////////////
#define TSS_KEYDOWN_W          0
#define TSS_KEYDOWN_S          1
#define TSS_KEYDOWN_A          4
#define TSS_KEYDOWN_D          5
#define TSS_KEYUP_W            2
#define TSS_KEYUP_S            3
#define TSS_KEYUP_A            6
#define TSS_KEYUP_D            7
#define TSS_MOUSE_LBUTTON_DOWN 8
#define TSS_MOUSE_LBUTTON_UP   9


///////////////////////////////////////////////////////////////////////////////////////////

using PACKET_SIZE = unsigned char;
using PACKET_TYPE = unsigned char;
constexpr int string_len = 32;

enum SSCS_PACKET {
	SSCS_TRY_LOGIN,
	SSCS_REQUEST_USER_INFO,
	SSCS_TRY_GAME_MATCHING,
	SSCS_SEND_CHAT_MESSAGE,
	SSCS_TRY_MOVE_CHARACTER,		//Done.
	SSCS_TRY_ROTATION_CHARACTER,	//Done.
	SSCS_TRY_NORMAL_ATTACK,			//DOne.
	SSCS_TRY_USE_SKILL,				//Done.
	SSCS_DONE_CHARACTER_MOTION,		//Done.
	SSCS_ACTIVATE_ANIM_NOTIFY,		//Done.
	SSCS_TRY_RETURN_LOBBY,

	SSCS_PACKET_COUNT
};

enum CSSS_PACKET {
	CSSS_LOGIN_OK,					//Done. but need to check player's hero pick.
	CSSS_CHANGE_SCENE,				//Need check Type.
	CSSS_SPAWN_PLAYER,				//Done.
	CSSS_SPAWN_NORMAL_ATTACK_OBJ,	//Done.
	CSSS_SPAWN_SKILL_OBJ,			//Done.
	CSSS_SPAWN_EFFECT,				//Need to work?
	CSSS_SET_OBJ_TRANSFORM,			//Done.
	CSSS_SET_CHARACTER_MOTION,		//Done.
	CSSS_SET_CHARACTER_STATE,		//Done.
	CSSS_DEACTIVATE_OBJ,			//Done.
	CSSS_SET_CHARACTER_HP,			//Done.
	CSSS_UPDATE_POISON_FOG_DEACT_AREA,
	CSSS_SET_GAME_PLAYTIME_LIMIT,	//When start game, send once.

	CSSS_SET_KDA_SCORE,				//UI will be updated after implement main logic
	CSSS_SEND_KILL_MESSAGE,
	CSSS_SEND_CHAT_MESSAGE,
	CSSS_SEND_MATCH_STATISTIC,
	CSSS_SEND_USER_INFO,

	CSSS_PACKET_COUNT
};

////////////////////////////////////// 통신 패킷 타입 /////////////////////////////////////
#pragma pack(push, 1)
// Terminal -> Streaming Server
struct tss_packet_keydown
{
	PACKET_SIZE size;
	PACKET_TYPE type;
};
struct tss_packet_keyup
{
	PACKET_SIZE size;
	PACKET_TYPE type;
};
struct tss_packet_mouse_button_down
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	long x;
	long y;
};
struct tss_packet_mouse_button_up
{
	PACKET_SIZE size;
	PACKET_TYPE type;
};


struct default_packet
{
	PACKET_SIZE size;
	PACKET_TYPE type;
};

// Streaming Server -> Contents Server
struct sscs_packet_try_login
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	wchar_t id[string_len];
	wchar_t password[string_len];

	sscs_packet_try_login()
	{
		for (unsigned char i = 0; i < string_len; ++i)
		{
			id[i] = 0x00;
			password[i] = 0x00;
		}
	}
};
struct sscs_packet_request_user_info
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short       client_id;
};
// 해당 플레이어가 어떤 캐릭터를 선택했는가에 따라
// PlayGameScene에 생성될 캐릭터들의 구성이 달라진다.
struct sscs_packet_try_game_matching
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short       client_id;

	// contents ref.
	char selected_character_type;
};
struct sscs_packet_send_chat_message
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	wchar_t message[string_len];

	sscs_packet_send_chat_message()
	{
		for (unsigned char i = 0; i < string_len; ++i)
			message[i] = 0x00;
	}
};
// 해당 클라이언트가 지닌 캐릭터 오브젝트를
// Yaw_angle만큼 회전시키고
// 해당 캐릭터의 고유 Speed값만큼 위치이동시킨다.
struct sscs_packet_try_move_character
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	float MoveDirection_Yaw_angle;
};
// 해당 클라이언트가 지닌 캐릭터 오브젝트를
// Yaw_angle만큼 회전시킨다.
struct sscs_packet_try_rotation_character
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	float Yaw_angle;
};
// 1. 해당 클라이언트가 지닌 캐릭터 오브젝트에 따라
//    Normal Attack 오브젝트를 생성한다.
// 2. 생성된 Normal Attack 오브젝트는
//    캐릭터 오브젝트가 바라보는 방향으로
//    아주 짧은 거리를 날아간다.
// 3. ViewList가 활성화된 클라이언트들에게
//    해당 캐릭터의 Normal Attack 모션을 실행하도록 명령한다.
//    해당 모션의 anim_timePos를 같이 전달한다.
//    (해당 모션을 실행하고 있는 도중에 ViewList에 들어올 수도 있기 때문.)
// 3. 적 플레이어와의 충돌처리를 요구한다.
//    A) 피격당한 플레이어의 캐릭터는 상태가
//       SEM_INVINCIBILITY(일시무적, 3초동안)로 바뀌고,
//    B) 3초동안 어떠한 피해도 받지 않는다.
//    C) 일시무적 상태를 렌더링하기 위해
//       ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 상태를 전달한다.
//    C) ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 IMPACT 모션을 실행하도록 명령한다.
//       해당 모션의 anim_timePos를 같이 전달한다.
//       (해당 모션을 실행하고 있는 도중에 ViewList에 들어올 수도 있기 때문.)
//    D) ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 감소한 HP를 전달한다.
//    E) 3초 후엔 캐릭터 상태가 NON으로 바뀌고 이또한
//       ViewList가 활성화된 클라이언트들에게 전달한다.
struct sscs_packet_try_normal_attack
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;
};
// 해당 클라이언트가 지닌 캐릭터 오브젝트에 따라
// Skill 오브젝트를 생성한다.
// Skill 오브젝트는 다음과 같은 처리를 요구한다.
//
// Sword Wave:
//    1. 캐릭터가 바라보는 방향과
//       바라보는 방향을 기준으로 좌/우 30도 방향으로
//       총 3개의 Sword Wave 오브젝트가 날라간다.
//    2. 해당 Sword Wave 오브젝트들은 10초동안 날라간다.
//    3. ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 Sword Wave 모션을 실행하도록 명령한다.
//       해당 모션의 anim_timePos를 같이 전달한다.
//       (해당 모션을 실행하고 있는 도중에 ViewList에 들어올 수도 있기 때문.)
//    4. 적 플레이어와의 충돌처리를 요구한다.
//       (Impact Damage: 30, Character State: SEMI_INVINCIBILITY)
//       A) 피격당한 플레이어의 캐릭터는 상태가
//          SEM_INVINCIBILITY(일시무적, 3초동안)로 바뀌고,
//       B) 3초동안 어떠한 피해도 받지 않는다.
//       C) 일시무적 상태를 렌더링하기 위해
//          ViewList가 활성화된 클라이언트들에게
//          해당 캐릭터의 상태를 전달한다.
//       C) ViewList가 활성화된 클라이언트들에게
//          해당 캐릭터의 IMPACT 모션을 실행하도록 명령한다.
//          IMPACT 모션의 anim_timePos를 같이 전달한다.
//          (IMPACT 모션을 실행하고 있는 도중에 ViewList에 들어올 수도 있기 때문.)
//       D) ViewList가 활성화된 클라이언트들에게
//          해당 캐릭터의 감소한 HP를 전달한다.
//       E) 3초 후엔 캐릭터 상태가 NON으로 바뀌고 이또한
//          ViewList가 활성화된 클라이언트들에게 전달한다.
//
// Holy Area:
//    1. 해당 클라이언트가 지닌 캐릭터 오브젝트 위치에
//       Holy Area 오브젝트를 생성한다.
//    2. 해당 Holy Area 오브젝트는 10초동안 유지된다.
//    3. Holy Area 오브젝트는 Holy Effect 오브젝트의 생성을 동반하지만
//       서버에선 관리하진 않는다.
//       다만 ViewList가 활성화된 클라이언트들에게 Holy Effect 오브젝트를 생성하도록 명령한다.
//    3. 해당 Effect Action TimePos를 관리하다가
//       ViewList가 활성화된 클라이언트들에게 전달한다.
//       (현재 Effect Action이 어느 순간을 연출하고 있느냐가 필요하기 때문.)
//    4. ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 Holy Area 모션을 실행하도록 명령한다.
//       해당 모션의 anim_timePos를 같이 전달한다.
//       (해당 모션을 실행하고 있는 도중에 ViewList에 들어올 수도 있기 때문.)
//    3. 아군 플레이어와의 충돌처리를 요구한다.
//       (Healing: 15 (every 1sec))
//       A) 캐릭터들의 HP가 회복될 때마다
//          ViewList가 활성화된 클라이언트들에게 갱신된 HP 정보를 전달한다.
//
// Fury Roar:
//    1. 해당 클라이언트가 지닌 캐릭터 오브젝트 위치에
//       Fury Roar Effect 오브젝트를 생성한다.
//    2. 해당 Effect Action TimePos를 관리하다가
//       ViewList가 활성화된 클라이언트들에게 전달한다.
//       (현재 Effect Action이 어느 순간을 연출하고 있느냐가 필요하기 때문.)
//    3. ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 Fury Roar 모션을 실행하도록 명령한다.
//       해당 모션의 anim_timePos를 같이 전달한다.
//       (해당 모션을 실행하고 있는 도중에 ViewList에 들어올 수도 있기 때문.)
//    3. 해당 클라이언트가 지닌 캐릭터 오브젝트의
//       이동속도와 공격속도를 2배 빠르게 한다.
//    4. 해당 스킬의 효과는 8초동안 유지된다.
//
// Stealth:
//    1. 해당 클라이언트가 지닌 캐릭터 오브젝트 위치에
//       Stealth Effect 오브젝트를 생성한다.
//    2. 해당 Stealth 상태는 7초동안 유지된다.
//    3. 해당 캐릭터가 공격을 하면 NON으로,
//       피격을 받으면 SEM_INVINCIBILITY로 바뀌며,
//       마찬가지로 ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 상태를 전달한다.
//       (Character State: STEALTH)
//    4. 해당 캐릭터의 투명도를 조절하기 위해
//       캐릭터의 상태를 STEALTH로 바꿔주고,
//       ViewList가 활성화된 클라이언트들에게
//       해당 캐릭터의 상태를 전달한다.
//       (아군 플레이어의 캐릭터면 반투명,
//        적 플레이어의 캐릭터면 투명)
struct sscs_packet_try_use_skill
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;
};
// 해당 캐릭터를 컨트롤 하는 클라이언트에게서만 전달되는 패킷
// 해당 모션이 끝났음을 알아야 ViewList가 활성화된 클라이언트들에게
// 해당 캐릭터의 새로운 모션을 지정해줄 수 있다.
// Idle이나 Walk 모션같은 경우에는 처음 모션이 지정될 때 Looping으로 지정되기 때문에
// 모션이 끝났음을 서버에게 통보할 필요는 없다.
struct sscs_packet_done_character_motion
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	char motion_type;
};
// 해당 캐릭터를 컨트롤 하는 클라이언트에게서만 전달되는 패킷
// anim_notify에 따라 normal attack 오브젝트가 생성되거나
// skill 오브젝트가 생성된다.
struct sscs_packet_activate_anim_notify
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	char anim_notify_type;
};
// 해당 패킷을 클라이언트로부터 받으면
// 해당 클라이언트가 matching room에서 빠져나가게 된다.
struct sscs_try_return_lobby
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;
};

//////////////////////////////////////////////////////////////////////////////////////

// Contents Server -> Streaming Server
struct csss_packet_login_ok
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short client_id;
};
struct csss_packet_change_scene
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	char scene_type;
};
// is_main_character:
// 메인 캐릭터 여부에 따라
// 카메라 뷰가 달라진다.
struct csss_packet_spawn_player
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int     object_id;
	wchar_t user_name[string_len]; // nick name
	char    character_type;
	float   scale_x, scale_y, scale_z;
	float   rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float   position_x, position_y, position_z;
	char    propensity;// 성향 (적 오브젝트인지, 아군 오브젝트인지)
	bool    is_main_character;

	csss_packet_spawn_player()
	{
		for (unsigned char i = 0; i < string_len; ++i)
			user_name[i] = 0x00;
	}
};
// 캐릭터 종류에 따라
// Normal Attack 오브젝트의 렌더Item이 달라진다.
// Normal Attack 오브젝트는 Quad형태로
// 지면과 수평한 상태로 짧은 거리를 날라간다.
struct csss_packet_spawn_normal_attack_obj
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	char  attack_order; // character type
	float scale_x, scale_y, scale_z;
	float rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float position_x, position_y, position_z;
	char  propensity;
};
// 스킬 종류에 따라
// Skill 오브젝트의 렌더Item이 달라진다.
struct csss_packet_spawn_skill_obj
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	char  skill_type;
	float scale_x, scale_y, scale_z;
	float rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float position_x, position_y, position_z;
	char  propensity;
};
// 이펙트 종류에 따라
// 생성되는 Effect 오브젝트들이 달라진다.
// Normal Attack 오브젝트나 Skill 오브젝트와 달리
// Effect가 생성될 위치만 전달한다.
// 해당 Effect Action TimePos를 관리하다가
// ViewList가 활성화된 클라이언트들에게 전달한다.
// (현재 Effect Action이 어느 순간을 연출하고 있느냐가 필요하기 때문.)
struct csss_packet_spawn_effect
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	char  effect_type;
	float effect_action_time_pos;
	float position_x, position_y, position_z;
};
struct csss_packet_set_obj_transform
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	float scale_x, scale_y, scale_z;
	float rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float position_x, position_y, position_z;
};
// anim_time_pos:
// 해당 모션을 실행하고 있는 도중에
// ViewList에 들어올 수도 있기 때문에
// 어느 순간의 모션을 실행하고 있는가가 필요하다.
// 이를 anim_time_pos가 대체한다.
// 서버에선 anim_time_pos를 이 패킷을 보낼 때마다 계산한다.
// 계산 방식은 아래 2가지로 나뉜다.
// 
// 해당 오브젝트가 처음 모션을 실행하고 나서
// 이 패킷을 구성할 때마다의 경과시간을 계산하고
// A) 그 경과시간을 해당 모션의 EndTime으로 나눴을 때의 나머지가
//    곧 anim_time_pos가 된다.
//    (Idle이나 Walk 모션일 경우에만 해당.)
// B) 그 경과시간이 곧 anim_time_pos가 된다.
//    단, 경과시간이 해당 모션의 EndTime를 초과할 경우
//    anim_time_pos는 해당 모션의 EndTiem이 된다.
//    (Idle이나 Walk 모션이 아닐 경우에만 해당.)
struct csss_packet_set_character_motion
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	char  motion_type;
	float anim_time_pos;
	char  skill_type; // skill motion일 경우 필요.
};
// 캐릭터 상태에 따라 렌더링 효과가 부여되거나 키입력 처리가 달라진다.
// 반투명해지거나, 깜빡이거나, 붉은 색을 띄거나, etc
// die 상태일 경우 키입력처리를 하지 않는다.
struct csss_pacekt_set_character_state
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	char character_state;
};
// 독안개 비활성 영역 크기에 따라
// 독안개 영역에 poison fog effect를 줄 예정
struct csss_packet_update_poison_fog_deact_area
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int left, top, right, bottom;
};
// 해당 오브젝트를 클라이언트 화면에서 비활성시킨다.
// 그리고 서버에선 해당 오브젝트 id를 폐기(혹은 비활성)한다.
// 단, 아군 클라이언트들의 ViewList에 존재하는
// Stealth 상태의 오브젝트는 폐기하지 않는다.
// 반대로 적군 클라이언트들의 화면에선 완전투명으로 되기 때문에
// 굳이 적군 ViewList에 Stealth 상태의 오브젝트를 취급할 필요는 없다.
// 고로 적군 클라이언트들의 ViewList에 존재하는
// 오브젝트들 중에 Stealth 상태를 갖는 오브젝트가 있으면
// 해당 오브젝트를 ViewList에서 제외함과 동시에
// 적군 클라이언트들에게 해당 오브젝트를 비활성화하도록 명령한다.
struct csss_packet_deactivate_obj
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
};
struct csss_packet_set_character_hp
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int object_id;
	int hp;
};

//UI
struct csss_packet_set_kda_score
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	unsigned char score_kill;
	unsigned char score_death;
	unsigned char score_assistance;
};

// message 예시
// [nick name](이)가 [nick name](을)를 죽였습니다.
struct csss_packet_send_kill_message
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	wchar_t message[string_len];

	csss_packet_send_kill_message()
	{
		for (char i = 0; i < string_len; ++i)
			message[i] = 0x00;
	}
};
// message 예시
// [nick name]: 채팅 내용
struct csss_packet_send_chat_message
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	wchar_t message[string_len];

	csss_packet_send_chat_message()
	{
		for (char i = 0; i < string_len; ++i)
			message[i] = 0x00;
	}
};
struct csss_packet_set_game_playtime_limit
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	unsigned int remain_sec;
};
// GameOverScene에 출력할 매칭 결과
// played_character_type에 따라
// GameOverScene에 렌더링되는 캐릭터가 달라진다.
struct csss_packet_send_match_statistic
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	wchar_t       user_name[string_len]; // nick name
	int           user_rank;
	unsigned char count_kill;
	unsigned char count_death;
	unsigned char count_assistance;
	int           totalscore_damage;
	int           totalscore_heal;
	char          played_character_type;
};
struct csss_packet_send_user_info
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	wchar_t user_name[string_len]; // nick name
	int     user_rank;

	csss_packet_send_user_info()
	{
		for (unsigned char i = 0; i < string_len; ++i)
			user_name[i] = 0x00;
	}
};
#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////////////////////