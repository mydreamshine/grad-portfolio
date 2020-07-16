#pragma once

//////////////////// ��� ������ Ÿ�� + �̺�Ʈ Ÿ��(Ȥ�� State Machine) ///////////////////
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

////////////////////////////////////// ��� ��Ŷ Ÿ�� /////////////////////////////////////
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
// �ش� �÷��̾ � ĳ���͸� �����ߴ°��� ����
// PlayGameScene�� ������ ĳ���͵��� ������ �޶�����.
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
// �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ��
// Yaw_angle��ŭ ȸ����Ű��
// �ش� ĳ������ ���� Speed����ŭ ��ġ�̵���Ų��.
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
// 1. �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ�� ����
//    Normal Attack ������Ʈ�� �����Ѵ�.
// 2. ������ Normal Attack ������Ʈ��
//    ĳ���� ������Ʈ�� �ٶ󺸴� ��������
//    ���� ª�� �Ÿ��� ���ư���.
// 3. ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//    �ش� ĳ������ Normal Attack ����� �����ϵ��� ����Ѵ�.
//    �ش� ����� anim_timePos�� ���� �����Ѵ�.
//    (�ش� ����� �����ϰ� �ִ� ���߿� ViewList�� ���� ���� �ֱ� ����.)
// 3. �� �÷��̾���� �浹ó���� �䱸�Ѵ�.
//    A) �ǰݴ��� �÷��̾��� ĳ���ʹ� ���°�
//       SEM_INVINCIBILITY(�Ͻù���, 3�ʵ���)�� �ٲ��,
//    B) 3�ʵ��� ��� ���ص� ���� �ʴ´�.
//    C) �Ͻù��� ���¸� �������ϱ� ����
//       ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ ���¸� �����Ѵ�.
//    C) ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ IMPACT ����� �����ϵ��� ����Ѵ�.
//       �ش� ����� anim_timePos�� ���� �����Ѵ�.
//       (�ش� ����� �����ϰ� �ִ� ���߿� ViewList�� ���� ���� �ֱ� ����.)
//    D) ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ ������ HP�� �����Ѵ�.
//    E) 3�� �Ŀ� ĳ���� ���°� NON���� �ٲ�� �̶���
//       ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� �����Ѵ�.
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
// �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ�� ����
// Skill ������Ʈ�� �����Ѵ�.
// Skill ������Ʈ�� ������ ���� ó���� �䱸�Ѵ�.
//
// Sword Wave:
//    1. ĳ���Ͱ� �ٶ󺸴� �����
//       �ٶ󺸴� ������ �������� ��/�� 30�� ��������
//       �� 3���� Sword Wave ������Ʈ�� ���󰣴�.
//    2. �ش� Sword Wave ������Ʈ���� 10�ʵ��� ���󰣴�.
//    3. ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ Sword Wave ����� �����ϵ��� ����Ѵ�.
//       �ش� ����� anim_timePos�� ���� �����Ѵ�.
//       (�ش� ����� �����ϰ� �ִ� ���߿� ViewList�� ���� ���� �ֱ� ����.)
//    4. �� �÷��̾���� �浹ó���� �䱸�Ѵ�.
//       (Impact Damage: 30, Character State: SEMI_INVINCIBILITY)
//       A) �ǰݴ��� �÷��̾��� ĳ���ʹ� ���°�
//          SEM_INVINCIBILITY(�Ͻù���, 3�ʵ���)�� �ٲ��,
//       B) 3�ʵ��� ��� ���ص� ���� �ʴ´�.
//       C) �Ͻù��� ���¸� �������ϱ� ����
//          ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//          �ش� ĳ������ ���¸� �����Ѵ�.
//       C) ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//          �ش� ĳ������ IMPACT ����� �����ϵ��� ����Ѵ�.
//          IMPACT ����� anim_timePos�� ���� �����Ѵ�.
//          (IMPACT ����� �����ϰ� �ִ� ���߿� ViewList�� ���� ���� �ֱ� ����.)
//       D) ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//          �ش� ĳ������ ������ HP�� �����Ѵ�.
//       E) 3�� �Ŀ� ĳ���� ���°� NON���� �ٲ�� �̶���
//          ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� �����Ѵ�.
//
// Holy Area:
//    1. �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ ��ġ��
//       Holy Area ������Ʈ�� �����Ѵ�.
//    2. �ش� Holy Area ������Ʈ�� 10�ʵ��� �����ȴ�.
//    3. Holy Area ������Ʈ�� Holy Effect ������Ʈ�� ������ ����������
//       �������� �������� �ʴ´�.
//       �ٸ� ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� Holy Effect ������Ʈ�� �����ϵ��� ����Ѵ�.
//    3. �ش� Effect Action TimePos�� �����ϴٰ�
//       ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� �����Ѵ�.
//       (���� Effect Action�� ��� ������ �����ϰ� �ִ��İ� �ʿ��ϱ� ����.)
//    4. ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ Holy Area ����� �����ϵ��� ����Ѵ�.
//       �ش� ����� anim_timePos�� ���� �����Ѵ�.
//       (�ش� ����� �����ϰ� �ִ� ���߿� ViewList�� ���� ���� �ֱ� ����.)
//    3. �Ʊ� �÷��̾���� �浹ó���� �䱸�Ѵ�.
//       (Healing: 15 (every 1sec))
//       A) ĳ���͵��� HP�� ȸ���� ������
//          ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� ���ŵ� HP ������ �����Ѵ�.
//
// Fury Roar:
//    1. �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ ��ġ��
//       Fury Roar Effect ������Ʈ�� �����Ѵ�.
//    2. �ش� Effect Action TimePos�� �����ϴٰ�
//       ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� �����Ѵ�.
//       (���� Effect Action�� ��� ������ �����ϰ� �ִ��İ� �ʿ��ϱ� ����.)
//    3. ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ Fury Roar ����� �����ϵ��� ����Ѵ�.
//       �ش� ����� anim_timePos�� ���� �����Ѵ�.
//       (�ش� ����� �����ϰ� �ִ� ���߿� ViewList�� ���� ���� �ֱ� ����.)
//    3. �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ��
//       �̵��ӵ��� ���ݼӵ��� 2�� ������ �Ѵ�.
//    4. �ش� ��ų�� ȿ���� 8�ʵ��� �����ȴ�.
//
// Stealth:
//    1. �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ ��ġ��
//       Stealth Effect ������Ʈ�� �����Ѵ�.
//    2. �ش� Stealth ���´� 7�ʵ��� �����ȴ�.
//    3. �ش� ĳ���Ͱ� ������ �ϸ� NON����,
//       �ǰ��� ������ SEM_INVINCIBILITY�� �ٲ��,
//       ���������� ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ ���¸� �����Ѵ�.
//       (Character State: STEALTH)
//    4. �ش� ĳ������ ������ �����ϱ� ����
//       ĳ������ ���¸� STEALTH�� �ٲ��ְ�,
//       ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
//       �ش� ĳ������ ���¸� �����Ѵ�.
//       (�Ʊ� �÷��̾��� ĳ���͸� ������,
//        �� �÷��̾��� ĳ���͸� ����)
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
// �ش� ĳ���͸� ��Ʈ�� �ϴ� Ŭ���̾�Ʈ���Լ��� ���޵Ǵ� ��Ŷ
// �ش� ����� �������� �˾ƾ� ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
// �ش� ĳ������ ���ο� ����� �������� �� �ִ�.
// Idle�̳� Walk ��ǰ��� ��쿡�� ó�� ����� ������ �� Looping���� �����Ǳ� ������
// ����� �������� �������� �뺸�� �ʿ�� ����.
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
// �ش� ĳ���͸� ��Ʈ�� �ϴ� Ŭ���̾�Ʈ���Լ��� ���޵Ǵ� ��Ŷ
// anim_notify�� ���� normal attack ������Ʈ�� �����ǰų�
// skill ������Ʈ�� �����ȴ�.
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
// �ش� ��Ŷ�� Ŭ���̾�Ʈ�κ��� ������
// �ش� Ŭ���̾�Ʈ�� matching room���� ���������� �ȴ�.
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
// ���� ĳ���� ���ο� ����
// ī�޶� �䰡 �޶�����.
struct csss_packet_spawn_player : packet_inheritance
{
	// contents ref.
	int     object_id;
	wchar_t user_name[255]; // nick name
	char    character_type;
	float   scale_x, scale_y, scale_z;
	float   rotation_euler_x, rotation_euler_y, rotation_euler_z;
	float   position_x, position_y, position_z;
	char    propensity;// ���� (�� ������Ʈ����, �Ʊ� ������Ʈ����)
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
// ĳ���� ������ ����
// Normal Attack ������Ʈ�� ����Item�� �޶�����.
// Normal Attack ������Ʈ�� Quad���·�
// ����� ������ ���·� ª�� �Ÿ��� ���󰣴�.
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
// ��ų ������ ����
// Skill ������Ʈ�� ����Item�� �޶�����.
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
// ����Ʈ ������ ����
// �����Ǵ� Effect ������Ʈ���� �޶�����.
// Normal Attack ������Ʈ�� Skill ������Ʈ�� �޸�
// Effect�� ������ ��ġ�� �����Ѵ�.
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
	char  skill_type; // skill motion�� ��� �ʿ�.

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
// ĳ���� ���¿� ���� ������ ȿ���� �ο��ǰų� Ű�Է� ó���� �޶�����.
// �����������ų�, �����̰ų�, ���� ���� ��ų�, etc
// die ������ ��� Ű�Է�ó���� ���� �ʴ´�.
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
// ���Ȱ� ��Ȱ�� ���� ũ�⿡ ����
// ���Ȱ� ������ poison fog effect�� �� ����
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
// �ش� ������Ʈ�� Ŭ���̾�Ʈ ȭ�鿡�� ��Ȱ����Ų��.
// �׸��� �������� �ش� ������Ʈ id�� ���(Ȥ�� ��Ȱ��)�Ѵ�.
// ��, �Ʊ� Ŭ���̾�Ʈ���� ViewList�� �����ϴ�
// Stealth ������ ������Ʈ�� ������� �ʴ´�.
// �ݴ�� ���� Ŭ���̾�Ʈ���� ȭ�鿡�� ������������ �Ǳ� ������
// ���� ���� ViewList�� Stealth ������ ������Ʈ�� ����� �ʿ�� ����.
// ��� ���� Ŭ���̾�Ʈ���� ViewList�� �����ϴ�
// ������Ʈ�� �߿� Stealth ���¸� ���� ������Ʈ�� ������
// �ش� ������Ʈ�� ViewList���� �����԰� ���ÿ�
// ���� Ŭ���̾�Ʈ�鿡�� �ش� ������Ʈ�� ��Ȱ��ȭ�ϵ��� ����Ѵ�.
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
// message ����
// [nick name](��)�� [nick name](��)�� �׿����ϴ�.
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
// message ����
// [nick name]: ä�� ����
// scene_type_to_recv: message�� ���޹��� Scene
// (LobyScene�� Message�ΰ�, PlayGameScene�� Message�ΰ�)
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
// GameOverScene�� ����� ��Ī ���
// played_character_type�� ����
// GameOverScene�� �������Ǵ� ĳ���Ͱ� �޶�����.
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