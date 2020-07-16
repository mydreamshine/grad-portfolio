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

#define SSCS_TRY_LOGIN               10
#define SSCS_REQUEST_USER_INFO       11
#define SSCS_TRY_GAME_MATCHING       12
#define SSCS_SEND_CHAT               13
#define SSCS_TRY_MOVE_CHARACTER      14
#define SSCS_TRY_MOVE_STOP_CHARACTER 15
#define SSCS_TRY_NORMAL_ATTACK       16
#define SSCS_TRY_USE_SKILL           17
#define SSCS_DONE_CHARACTER_MOTION   18
#define SSCS_ACTIVATE_ANIM_NOTIFY    19
#define SSCS_TRY_RETURN_LOBY         20

#define CSSS_LOGIN_OK                     21
#define CSSS_CHANGE_SCENE                 22
#define CSSS_SPAWN_PLAYER                 23
#define CSSS_SPAWN_NORMAL_ATTACK_OBJ      24
#define CSSS_SPAWN_SKILL_OBJ              25
#define CSSS_SPAWN_EFFECT_OBJ             26
#define CSSS_SET_TRANSFORM_WORLD_OBJ      27
#define CSSS_SET_CHARACTER_MOTION         28
#define CSSS_SET_PLAYER_STATE             29
#define CSSS_UPDATE_POISON_FOG_DEACT_AREA 30
#define CSSS_DEACTIVATE_OBJ               31
#define CSSS_SET_USER_INFO                32
#define CSSS_SET_KDA_SCORE                33
#define CSSS_SET_KILL_LOG                 34
#define CSSS_SET_CHAT_LOG                 35
#define CSSS_SET_GAME_PLAY_TIME_LIMIT     36
#define CSSS_SET_PLAYER_HP                37
#define CSSS_SET_MATCH_STATISTIC_INFO     38
///////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////// 통신 패킷 타입 /////////////////////////////////////
#pragma pack(push, 1)
struct packet_inheritance // 
{
	unsigned char size;
	unsigned char type;
};

// Terminal -> Streaming Server
struct tss_packet_keydown
{
	unsigned char size;
	unsigned char type;
};
struct tss_packet_keyup
{
	unsigned char size;
	unsigned char type;
};
struct tss_packet_mouse_button_down
{
	unsigned char size;
	unsigned char type;

	long x;
	long y;
};
struct tss_packet_mouse_button_up
{
	unsigned char size;
	unsigned char type;
};



// Streaming Server -> Contents Server
struct sscs_packet_try_login : packet_inheritance
{
	// contents ref.
	wchar_t id[255];
	wchar_t password[255];

	sscs_packet_try_login()
	{
		size = (unsigned char)sizeof(sscs_packet_try_login);
		type = SSCS_TRY_LOGIN;

		for (unsigned char i = 0; i < 255; ++i)
		{
			id[i] = 0x00;
			password[i] = 0x00;
		}
	}
	sscs_packet_try_login(const wchar_t* id_text, int id_text_size,
		const wchar_t* password_text, int password_text_size)
	{
		size = (unsigned char)sizeof(sscs_packet_try_login);
		type = SSCS_TRY_LOGIN;

		unsigned char ID_Text_size = (id_text_size > 255) ? 255 : (unsigned char)id_text_size;
		unsigned char Password_Text_size = (password_text_size > 255) ? 255 : (unsigned char)id_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (ID_Text_size > i) id[i] = id_text[i];
			else id[i] = 0x00;

			if (Password_Text_size > i) password[i] = password_text[i];
			else password[i] = 0x00;
		}
	}
};
struct sscs_packet_request_user_info : packet_inheritance
{
	short         client_id;

	sscs_packet_request_user_info()
	{
		size = (unsigned char)sizeof(sscs_packet_request_user_info);
		type = SSCS_REQUEST_USER_INFO;
	}
	sscs_packet_request_user_info(short clientID)
	{
		size = (unsigned char)sizeof(sscs_packet_request_user_info);
		type = SSCS_REQUEST_USER_INFO;

		client_id = clientID;
	}
};
// 해당 플레이어가 어떤 캐릭터를 선택했는가에 따라
// PlayGameScene에 생성될 캐릭터들의 구성이 달라진다.
struct sscs_packet_try_game_matching : packet_inheritance
{
	short         client_id;

	// contents ref.
	char selected_character_type;

	sscs_packet_try_game_matching()
	{
		size = (unsigned char)sizeof(sscs_packet_try_game_matching);
		type = SSCS_TRY_GAME_MATCHING;
	}
	sscs_packet_try_game_matching(short clientID, char character_type)
	{
		size = (unsigned char)sizeof(sscs_packet_try_game_matching);
		type = SSCS_TRY_GAME_MATCHING;

		client_id = clientID;
		selected_character_type = character_type;
	}
};
struct sscs_packet_send_chat_message : packet_inheritance
{
	short         client_id;

	// contents ref.
	wchar_t message[255];

	sscs_packet_send_chat_message()
	{
		size = (unsigned char)sizeof(sscs_packet_send_chat_message);
		type = SSCS_SEND_CHAT;

		for (unsigned char i = 0; i < 255; ++i)
			message[i] = 0x00;
	}
	sscs_packet_send_chat_message(short clientID, const wchar_t* message_text, int message_text_size)
	{
		size = (unsigned char)sizeof(sscs_packet_send_chat_message);
		type = SSCS_SEND_CHAT;

		client_id = clientID;
		unsigned char Message_Text_size = (message_text_size > 255) ? 255 : (unsigned char)message_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (Message_Text_size > i) message[i] = message_text[i];
			else message[i] = 0x00;
		}
	}
};
// 해당 클라이언트가 지닌 캐릭터 오브젝트를
// Yaw_angle만큼 회전시키고
// 해당 캐릭터의 고유 Speed값만큼 위치이동시킨다.
struct sscs_packet_try_move_character : packet_inheritance
{
	short         client_id;

	// contents ref.
	float MoveDirection_Yaw_angle;

	sscs_packet_try_move_character()
	{
		size = (unsigned char)sizeof(sscs_packet_try_move_character);
		type = SSCS_TRY_MOVE_CHARACTER;
	}
	sscs_packet_try_move_character(short clientID, float Yaw_angle)
	{
		size = (unsigned char)sizeof(sscs_packet_try_move_character);
		type = SSCS_TRY_MOVE_CHARACTER;

		client_id = clientID;
		MoveDirection_Yaw_angle = Yaw_angle;
	}
};
struct sscs_packet_try_movestop_character : packet_inheritance
{
	short         client_id;

	sscs_packet_try_movestop_character()
	{
		size = (unsigned char)sizeof(sscs_packet_try_movestop_character);
		type = SSCS_TRY_MOVE_STOP_CHARACTER;
	}
	sscs_packet_try_movestop_character(short clientID)
	{
		size = (unsigned char)sizeof(sscs_packet_try_movestop_character);
		type = SSCS_TRY_MOVE_STOP_CHARACTER;

		client_id = clientID;
	}
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
struct sscs_packet_try_normal_attack : packet_inheritance
{
	short         client_id;

	// contents ref.
	float character_yaw_angle;

	sscs_packet_try_normal_attack()
	{
		size = (unsigned char)sizeof(sscs_packet_try_normal_attack);
		type = SSCS_TRY_NORMAL_ATTACK;
	}
	sscs_packet_try_normal_attack(short clientID, float yaw_angle)
	{
		size = (unsigned char)sizeof(sscs_packet_try_normal_attack);
		type = SSCS_TRY_NORMAL_ATTACK;

		client_id = clientID;
		character_yaw_angle = yaw_angle;
	}
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
struct sscs_packet_try_use_skill : packet_inheritance
{
	short         client_id;

	// contents ref.
	float character_yaw_angle;

	sscs_packet_try_use_skill()
	{
		size = (unsigned char)sizeof(sscs_packet_try_use_skill);
		type = SSCS_TRY_USE_SKILL;
	}
	sscs_packet_try_use_skill(short clientID, float yaw_angle)
	{
		size = (unsigned char)sizeof(sscs_packet_try_use_skill);
		type = SSCS_TRY_USE_SKILL;

		client_id = clientID;
		character_yaw_angle = yaw_angle;
	}
};
// 해당 캐릭터를 컨트롤 하는 클라이언트에게서만 전달되는 패킷
// 해당 모션이 끝났음을 알아야 ViewList가 활성화된 클라이언트들에게
// 해당 캐릭터의 새로운 모션을 지정해줄 수 있다.
// Idle이나 Walk 모션같은 경우에는 처음 모션이 지정될 때 Looping으로 지정되기 때문에
// 모션이 끝났음을 서버에게 통보할 필요는 없다.
struct sscs_packet_done_character_motion : packet_inheritance
{
	short         client_id;

	// contents ref.
	char motion_type;

	sscs_packet_done_character_motion()
	{
		size = (unsigned char)sizeof(sscs_packet_done_character_motion);
		type = SSCS_DONE_CHARACTER_MOTION;
	}
	sscs_packet_done_character_motion(short clientID, char MotionType)
	{
		size = (unsigned char)sizeof(sscs_packet_done_character_motion);
		type = SSCS_DONE_CHARACTER_MOTION;

		client_id = clientID;
		motion_type = MotionType;
	}
};
// 해당 캐릭터를 컨트롤 하는 클라이언트에게서만 전달되는 패킷
// anim_notify에 따라 normal attack 오브젝트가 생성되거나
// skill 오브젝트가 생성된다.
struct sscs_packet_activate_anim_notify : packet_inheritance
{
	short         client_id;

	// contents ref.
	char anim_notify_type;

	sscs_packet_activate_anim_notify()
	{
		size = (unsigned char)sizeof(sscs_packet_activate_anim_notify);
		type = SSCS_ACTIVATE_ANIM_NOTIFY;
	}
	sscs_packet_activate_anim_notify(short clientID, char AnimNotifyType)
	{
		size = (unsigned char)sizeof(sscs_packet_activate_anim_notify);
		type = SSCS_ACTIVATE_ANIM_NOTIFY;

		client_id = clientID;
		anim_notify_type = AnimNotifyType;
	}
};
// 해당 패킷을 클라이언트로부터 받으면
// 해당 클라이언트가 matching room에서 빠져나가게 된다.
struct sscs_try_return_loby : packet_inheritance
{
	short         client_id;

	sscs_try_return_loby()
	{
		size = (unsigned char)sizeof(sscs_try_return_loby);
		type = SSCS_TRY_RETURN_LOBY;
	}
	sscs_try_return_loby(short clientID)
	{
		size = (unsigned char)sizeof(sscs_try_return_loby);
		type = SSCS_TRY_RETURN_LOBY;

		client_id = clientID;
	}
};



// Contents Server -> Streaming Server
struct csss_packet_login_ok : packet_inheritance
{
	short client_id;

	csss_packet_login_ok()
	{
		size = (unsigned char)sizeof(csss_packet_login_ok);
		type = CSSS_LOGIN_OK;
	}
	csss_packet_login_ok(short clientID)
	{
		size = (unsigned char)sizeof(csss_packet_login_ok);
		type = CSSS_LOGIN_OK;

		client_id = clientID;
	}
};
struct csss_packet_change_scene : packet_inheritance
{
	// contents ref.
	char scene_type;

	csss_packet_change_scene()
	{
		size = (unsigned char)sizeof(csss_packet_change_scene);
		type = CSSS_CHANGE_SCENE;
	}
	csss_packet_change_scene(char SceneType)
	{
		size = (unsigned char)sizeof(csss_packet_change_scene);
		type = CSSS_CHANGE_SCENE;

		scene_type = SceneType;
	}
};
// is_main_character:
// 메인 캐릭터 여부에 따라
// 카메라 뷰가 달라진다.
struct csss_packet_spawn_player : packet_inheritance
{
	// contents ref.
	int     object_id;
	wchar_t user_name[255]; // nick name
	char    character_type;
	float   scale_x, scale_y, scale_z;
	float   rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float   position_x, position_y, position_z;
	char    propensity;// 성향 (적 오브젝트인지, 아군 오브젝트인지)
	bool    is_main_character;

	csss_packet_spawn_player()
	{
		size = (unsigned char)sizeof(csss_packet_spawn_player);
		type = CSSS_SPAWN_PLAYER;

		for (unsigned char i = 0; i < 255; ++i)
			user_name[i] = 0x00;
	}
	csss_packet_spawn_player(
		int obj_ID,
		const wchar_t* userName_text, int userName_text_size,
		char CharacterType,
		float ScaleX, float ScaleY, float ScaleZ,
		float RotationEulerX, float RotationEulerY, float RotationEulerZ,
		float PositionX, float PositionY, float PositionZ,
		char Propensity,
		bool IsMainCharacter)
	{
		size = (unsigned char)sizeof(csss_packet_spawn_player);
		type = CSSS_SPAWN_PLAYER;

		object_id = obj_ID;
		unsigned char UserName_Text_size = (userName_text_size > 255) ? 255 : (unsigned char)userName_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (UserName_Text_size > i) user_name[i] = userName_text[i];
			else user_name[i] = 0x00;
		}
		character_type = CharacterType;
		scale_x = ScaleX; scale_y = ScaleY; scale_z = ScaleZ;
		rotation_euler_x = RotationEulerX; rotation_euler_y = RotationEulerY; rotation_euler_z = RotationEulerZ;
		position_x = PositionX; position_y = PositionY; position_z = PositionZ;
		propensity = Propensity;
		is_main_character = IsMainCharacter;
	}

};
// 캐릭터 종류에 따라
// Normal Attack 오브젝트의 렌더Item이 달라진다.
// Normal Attack 오브젝트는 Quad형태로
// 지면과 수평한 상태로 짧은 거리를 날라간다.
struct csss_packet_spawn_normal_attack_obj : packet_inheritance
{
	// contents ref.
	int   object_id;
	char  attack_order; // character type
	float scale_x, scale_y, scale_z;
	float rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float position_x, position_y, position_z;
	char  propensity;

	csss_packet_spawn_normal_attack_obj()
	{
		size = (unsigned char)sizeof(csss_packet_spawn_normal_attack_obj);
		type = CSSS_SPAWN_NORMAL_ATTACK_OBJ;
	}
	csss_packet_spawn_normal_attack_obj(
		int obj_ID,
		char AttackOrder,
		float ScaleX, float ScaleY, float ScaleZ,
		float RotationEulerX, float RotationEulerY, float RotationEulerZ,
		float PositionX, float PositionY, float PositionZ,
		char Propensity)
	{
		size = (unsigned char)sizeof(csss_packet_spawn_normal_attack_obj);
		type = CSSS_SPAWN_NORMAL_ATTACK_OBJ;

		object_id = obj_ID;
		attack_order = AttackOrder;
		scale_x = ScaleX; scale_y = ScaleY; scale_z = ScaleZ;
		rotation_euler_x = RotationEulerX; rotation_euler_y = RotationEulerY; rotation_euler_z = RotationEulerZ;
		position_x = PositionX; position_y = PositionY; position_z = PositionZ;
		propensity = Propensity;
	}
};
// 스킬 종류에 따라
// Skill 오브젝트의 렌더Item이 달라진다.
struct csss_packet_spawn_skill_obj : packet_inheritance
{
	// contents ref.
	int   object_id;
	char  skill_type;
	float scale_x, scale_y, scale_z;
	float rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float position_x, position_y, position_z;
	char  propensity;

	csss_packet_spawn_skill_obj()
	{
		size = (unsigned char)sizeof(csss_packet_spawn_skill_obj);
		type = CSSS_SPAWN_SKILL_OBJ;
	}
	csss_packet_spawn_skill_obj(
		int obj_ID,
		char SkillType,
		float ScaleX, float ScaleY, float ScaleZ,
		float RotationEulerX, float RotationEulerY, float RotationEulerZ,
		float PositionX, float PositionY, float PositionZ,
		char Propensity)
	{
		size = (unsigned char)sizeof(csss_packet_spawn_skill_obj);
		type = CSSS_SPAWN_SKILL_OBJ;

		object_id = obj_ID;
		skill_type = SkillType;
		scale_x = ScaleX; scale_y = ScaleY; scale_z = ScaleZ;
		rotation_euler_x = RotationEulerX; rotation_euler_y = RotationEulerY; rotation_euler_z = RotationEulerZ;
		position_x = PositionX; position_y = PositionY; position_z = PositionZ;
		propensity = Propensity;
	}
};
// 이펙트 종류에 따라
// 생성되는 Effect 오브젝트들이 달라진다.
// Normal Attack 오브젝트나 Skill 오브젝트와 달리
// Effect가 생성될 위치만 전달한다.
struct csss_packet_spawn_effect : packet_inheritance
{
	// contents ref.
	char  effect_type;
	float position_x, position_y, position_z;

	csss_packet_spawn_effect()
	{
		size = (unsigned char)sizeof(csss_packet_spawn_effect);
		type = CSSS_SPAWN_EFFECT_OBJ;
	}
	csss_packet_spawn_effect(
		char EffectType,
		float EffectTimePos,
		float PositionX, float PositionY, float PositionZ)
	{
		size = (unsigned char)sizeof(csss_packet_spawn_effect);
		type = CSSS_SPAWN_EFFECT_OBJ;

		effect_type = EffectType;
		position_x = PositionX; position_y = PositionY; position_z = PositionZ;
	}
};
struct csss_packet_set_obj_transform : packet_inheritance
{
	// contents ref.
	int   object_id;
	float scale_x, scale_y, scale_z;
	float rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float position_x, position_y, position_z;

	csss_packet_set_obj_transform()
	{
		size = (unsigned char)sizeof(csss_packet_set_obj_transform);
		type = CSSS_SET_TRANSFORM_WORLD_OBJ;
	}
	csss_packet_set_obj_transform(
		int obj_ID,
		float ScaleX, float ScaleY, float ScaleZ,
		float RotationEulerX, float RotationEulerY, float RotationEulerZ,
		float PositionX, float PositionY, float PositionZ)
	{
		size = (unsigned char)sizeof(csss_packet_set_obj_transform);
		type = CSSS_SET_TRANSFORM_WORLD_OBJ;

		object_id = obj_ID;
		scale_x = ScaleX; scale_y = ScaleY; scale_z = ScaleZ;
		rotation_euler_x = RotationEulerX; rotation_euler_y = RotationEulerY; rotation_euler_z = RotationEulerZ;
		position_x = PositionX; position_y = PositionY; position_z = PositionZ;
	}
};
struct csss_packet_set_character_motion : packet_inheritance
{
	// contents ref.
	int   object_id;
	char  motion_type;
	char  skill_type; // skill motion일 경우 필요.

	csss_packet_set_character_motion()
	{
		size = (unsigned char)sizeof(csss_packet_set_character_motion);
		type = CSSS_SET_CHARACTER_MOTION;
	}
	csss_packet_set_character_motion(
		int obj_ID,
		char MotionType,
		float AnimTimePos,
		char SkillType)
	{
		size = (unsigned char)sizeof(csss_packet_set_character_motion);
		type = CSSS_SET_CHARACTER_MOTION;

		object_id = obj_ID;
		motion_type = MotionType;
		skill_type = SkillType;
	}
};
// 캐릭터 상태에 따라 렌더링 효과가 부여되거나 키입력 처리가 달라진다.
// 반투명해지거나, 깜빡이거나, 붉은 색을 띄거나, etc
// die 상태일 경우 키입력처리를 하지 않는다.
struct csss_pacekt_set_character_state : packet_inheritance
{
	// contents ref.
	int   object_id;
	char character_state;

	csss_pacekt_set_character_state()
	{
		size = (unsigned char)sizeof(csss_pacekt_set_character_state);
		type = CSSS_SET_PLAYER_STATE;
	}
	csss_pacekt_set_character_state(
		int obj_ID,
		char CharacterState)
	{
		size = (unsigned char)sizeof(csss_pacekt_set_character_state);
		type = CSSS_SET_PLAYER_STATE;

		object_id = obj_ID;
		character_state = CharacterState;
	}
};
// 독안개 비활성 영역 크기에 따라
// 독안개 영역에 poison fog effect를 줄 예정
struct csss_packet_update_poison_fog_deact_area : packet_inheritance
{
	// contents ref.
	int left, top, right, bottom;

	csss_packet_update_poison_fog_deact_area()
	{
		size = (unsigned char)sizeof(csss_packet_update_poison_fog_deact_area);
		type = CSSS_UPDATE_POISON_FOG_DEACT_AREA;
	}
	csss_packet_update_poison_fog_deact_area(
		int Left, int Top, int Right, int Bottom)
	{
		size = (unsigned char)sizeof(csss_packet_update_poison_fog_deact_area);
		type = CSSS_UPDATE_POISON_FOG_DEACT_AREA;

		left = Left;
		right = Right;
		top = Top;
		bottom = Bottom;
	}
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
struct csss_packet_deactivate_obj : packet_inheritance
{
	// contents ref.
	int   object_id;

	csss_packet_deactivate_obj()
	{
		size = (unsigned char)sizeof(csss_packet_deactivate_obj);
		type = CSSS_DEACTIVATE_OBJ;
	}
	csss_packet_deactivate_obj(
		int obj_ID)
	{
		size = (unsigned char)sizeof(csss_packet_deactivate_obj);
		type = CSSS_DEACTIVATE_OBJ;

		object_id = obj_ID;
	}
};
struct csss_packet_send_user_info : packet_inheritance
{
	// contents ref.
	wchar_t user_name[255]; // nick name
	int     user_rank;

	csss_packet_send_user_info()
	{
		size = (unsigned char)sizeof(csss_packet_send_user_info);
		type = CSSS_SET_USER_INFO;

		for (unsigned char i = 0; i < 255; ++i)
			user_name[i] = 0x00;
	}
	csss_packet_send_user_info(
		const wchar_t* userName_text, int userName_text_size,
		int userRank)
	{
		size = (unsigned char)sizeof(csss_packet_send_user_info);
		type = CSSS_SET_USER_INFO;

		unsigned char UserName_Text_size = (userName_text_size > 255) ? 255 : (unsigned char)userName_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (UserName_Text_size > i) user_name[i] = userName_text[i];
			else user_name[i] = 0x00;
		}
		user_rank = userRank;
	}
};
struct csss_packet_set_kda_score : packet_inheritance
{
	// contents ref.
	unsigned char score_kill;
	unsigned char score_death;
	unsigned char score_assistance;

	csss_packet_set_kda_score()
	{
		size = (unsigned char)sizeof(csss_packet_set_kda_score);
		type = CSSS_SET_KDA_SCORE;
	}
	csss_packet_set_kda_score(
		unsigned char ScoreKill,
		unsigned char ScoreDeath,
		unsigned char ScoreAssistance)
	{
		size = (unsigned char)sizeof(csss_packet_set_kda_score);
		type = CSSS_SET_KDA_SCORE;

		score_kill = ScoreKill;
		score_death = ScoreDeath;
		score_assistance = ScoreAssistance;
	}
};
// message 예시
// [nick name](이)가 [nick name](을)를 죽였습니다.
struct csss_packet_send_kill_message : packet_inheritance
{
	// contents ref.
	wchar_t message[255];

	csss_packet_send_kill_message()
	{
		size = (unsigned char)sizeof(csss_packet_send_kill_message);
		type = CSSS_SET_KILL_LOG;

		for (unsigned char i = 0; i < 255; ++i)
			message[i] = 0x00;
	}
	csss_packet_send_kill_message(
		const wchar_t* message_text, int message_text_size)
	{
		size = (unsigned char)sizeof(csss_packet_send_kill_message);
		type = CSSS_SET_KILL_LOG;

		unsigned char Message_Text_size = (message_text_size > 255) ? 255 : (unsigned char)message_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (Message_Text_size > i) message[i] = message_text[i];
			else message[i] = 0x00;
		}
	}
};
// message 예시
// [nick name]: 채팅 내용
// scene_type_to_recv: message를 전달받을 Scene
// (LobyScene의 Message인가, PlayGameScene의 Message인가)
struct csss_packet_send_chat_message : packet_inheritance
{
	// contents ref.
	char    scene_type_to_recv;
	wchar_t message[255];

	csss_packet_send_chat_message()
	{
		size = (unsigned char)sizeof(csss_packet_send_chat_message);
		type = CSSS_SET_CHAT_LOG;

		for (unsigned char i = 0; i < 255; ++i)
			message[i] = 0x00;
	}
	csss_packet_send_chat_message(
		char SceneTypeToRecv,
		const wchar_t* message_text, int message_text_size)
	{
		size = (unsigned char)sizeof(csss_packet_send_chat_message);
		type = CSSS_SET_CHAT_LOG;

		scene_type_to_recv = SceneTypeToRecv;
		unsigned char Message_Text_size = (message_text_size > 255) ? 255 : (unsigned char)message_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (Message_Text_size > i) message[i] = message_text[i];
			else message[i] = 0x00;
		}
	}
};
struct csss_packet_set_game_playtime_limit : packet_inheritance
{
	// contents ref.
	unsigned int remain_sec;

	csss_packet_set_game_playtime_limit()
	{
		size = (unsigned char)sizeof(csss_packet_set_game_playtime_limit);
		type = CSSS_SET_GAME_PLAY_TIME_LIMIT;
	}
	csss_packet_set_game_playtime_limit(
		unsigned int RemainSec)
	{
		size = (unsigned char)sizeof(csss_packet_set_game_playtime_limit);
		type = CSSS_SET_GAME_PLAY_TIME_LIMIT;

		remain_sec = RemainSec;
	}
};
struct csss_packet_set_character_hp : packet_inheritance
{
	// contents ref.
	int object_id;
	int hp;

	csss_packet_set_character_hp()
	{
		size = (unsigned char)sizeof(csss_packet_set_character_hp);
		type = CSSS_SET_PLAYER_HP;
	}
	csss_packet_set_character_hp(
		int obj_ID,
		int HP)
	{
		size = (unsigned char)sizeof(csss_packet_set_character_hp);
		type = CSSS_SET_PLAYER_HP;

		object_id = obj_ID;
		hp = HP;
	}
};
// GameOverScene에 출력할 매칭 결과
// played_character_type에 따라
// GameOverScene에 렌더링되는 캐릭터가 달라진다.
struct csss_packet_send_match_statistic : packet_inheritance
{
	// contents ref.
	wchar_t       user_name[255]; // nick name
	int           user_rank;
	unsigned char count_kill;
	unsigned char count_death;
	unsigned char count_assistance;
	int           totalscore_damage;
	int           totalscore_heal;
	char          played_character_type;
	csss_packet_send_match_statistic()
	{
		size = (unsigned char)sizeof(csss_packet_send_match_statistic);
		type = CSSS_SET_MATCH_STATISTIC_INFO;

		for (unsigned char i = 0; i < 255; ++i)
			user_name[i] = 0x00;
	}
	csss_packet_send_match_statistic(
		const wchar_t* userName_text, int userName_text_size,
		int userRank,
		unsigned char CountKill,
		unsigned char CountDeath,
		unsigned char CountAssistance,
		int TotalScoreDamage,
		int TotalScoreHeal,
		char PlayedCharacterType)
	{
		size = (unsigned char)sizeof(csss_packet_send_match_statistic);
		type = CSSS_SET_MATCH_STATISTIC_INFO;

		unsigned char UserName_Text_size = (userName_text_size > 255) ? 255 : (unsigned char)userName_text_size;
		for (unsigned char i = 0; i < 255; ++i)
		{
			if (UserName_Text_size > i) user_name[i] = userName_text[i];
			else user_name[i] = 0x00;
		}
		user_rank = userRank;
		count_kill = CountKill;
		count_death = CountDeath;
		count_assistance = CountAssistance;
		totalscore_damage = TotalScoreDamage;
		totalscore_heal = TotalScoreHeal;
		played_character_type = PlayedCharacterType;
	}
};
#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////////////////////