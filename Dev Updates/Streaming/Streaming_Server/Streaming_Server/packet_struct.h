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

////////////////////////////////////// ��� ��Ŷ Ÿ�� /////////////////////////////////////
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
// �ش� �÷��̾ � ĳ���͸� �����ߴ°��� ����
// PlayGameScene�� ������ ĳ���͵��� ������ �޶�����.
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
// �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ��
// Yaw_angle��ŭ ȸ����Ű��
// �ش� ĳ������ ���� Speed����ŭ ��ġ�̵���Ų��.
struct sscs_packet_try_move_character
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	float MoveDirection_Yaw_angle;
};
// �ش� Ŭ���̾�Ʈ�� ���� ĳ���� ������Ʈ��
// Yaw_angle��ŭ ȸ����Ų��.
struct sscs_packet_try_rotation_character
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	float Yaw_angle;
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
struct sscs_packet_try_normal_attack
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;
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
struct sscs_packet_try_use_skill
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;
};
// �ش� ĳ���͸� ��Ʈ�� �ϴ� Ŭ���̾�Ʈ���Լ��� ���޵Ǵ� ��Ŷ
// �ش� ����� �������� �˾ƾ� ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡��
// �ش� ĳ������ ���ο� ����� �������� �� �ִ�.
// Idle�̳� Walk ��ǰ��� ��쿡�� ó�� ����� ������ �� Looping���� �����Ǳ� ������
// ����� �������� �������� �뺸�� �ʿ�� ����.
struct sscs_packet_done_character_motion
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	char motion_type;
};
// �ش� ĳ���͸� ��Ʈ�� �ϴ� Ŭ���̾�Ʈ���Լ��� ���޵Ǵ� ��Ŷ
// anim_notify�� ���� normal attack ������Ʈ�� �����ǰų�
// skill ������Ʈ�� �����ȴ�.
struct sscs_packet_activate_anim_notify
{
	PACKET_SIZE size;
	PACKET_TYPE type;
	short         client_id;

	// contents ref.
	char anim_notify_type;
};
// �ش� ��Ŷ�� Ŭ���̾�Ʈ�κ��� ������
// �ش� Ŭ���̾�Ʈ�� matching room���� ���������� �ȴ�.
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
// ���� ĳ���� ���ο� ����
// ī�޶� �䰡 �޶�����.
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
	char    propensity;// ���� (�� ������Ʈ����, �Ʊ� ������Ʈ����)
	bool    is_main_character;

	csss_packet_spawn_player()
	{
		for (unsigned char i = 0; i < string_len; ++i)
			user_name[i] = 0x00;
	}
};
// ĳ���� ������ ����
// Normal Attack ������Ʈ�� ����Item�� �޶�����.
// Normal Attack ������Ʈ�� Quad���·�
// ����� ������ ���·� ª�� �Ÿ��� ���󰣴�.
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
// ��ų ������ ����
// Skill ������Ʈ�� ����Item�� �޶�����.
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
// ����Ʈ ������ ����
// �����Ǵ� Effect ������Ʈ���� �޶�����.
// Normal Attack ������Ʈ�� Skill ������Ʈ�� �޸�
// Effect�� ������ ��ġ�� �����Ѵ�.
// �ش� Effect Action TimePos�� �����ϴٰ�
// ViewList�� Ȱ��ȭ�� Ŭ���̾�Ʈ�鿡�� �����Ѵ�.
// (���� Effect Action�� ��� ������ �����ϰ� �ִ��İ� �ʿ��ϱ� ����.)
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
// �ش� ����� �����ϰ� �ִ� ���߿�
// ViewList�� ���� ���� �ֱ� ������
// ��� ������ ����� �����ϰ� �ִ°��� �ʿ��ϴ�.
// �̸� anim_time_pos�� ��ü�Ѵ�.
// �������� anim_time_pos�� �� ��Ŷ�� ���� ������ ����Ѵ�.
// ��� ����� �Ʒ� 2������ ������.
// 
// �ش� ������Ʈ�� ó�� ����� �����ϰ� ����
// �� ��Ŷ�� ������ �������� ����ð��� ����ϰ�
// A) �� ����ð��� �ش� ����� EndTime���� ������ ���� ��������
//    �� anim_time_pos�� �ȴ�.
//    (Idle�̳� Walk ����� ��쿡�� �ش�.)
// B) �� ����ð��� �� anim_time_pos�� �ȴ�.
//    ��, ����ð��� �ش� ����� EndTime�� �ʰ��� ���
//    anim_time_pos�� �ش� ����� EndTiem�� �ȴ�.
//    (Idle�̳� Walk ����� �ƴ� ��쿡�� �ش�.)
struct csss_packet_set_character_motion
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	char  motion_type;
	float anim_time_pos;
	char  skill_type; // skill motion�� ��� �ʿ�.
};
// ĳ���� ���¿� ���� ������ ȿ���� �ο��ǰų� Ű�Է� ó���� �޶�����.
// �����������ų�, �����̰ų�, ���� ���� ��ų�, etc
// die ������ ��� Ű�Է�ó���� ���� �ʴ´�.
struct csss_pacekt_set_character_state
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int   object_id;
	char character_state;
};
// ���Ȱ� ��Ȱ�� ���� ũ�⿡ ����
// ���Ȱ� ������ poison fog effect�� �� ����
struct csss_packet_update_poison_fog_deact_area
{
	PACKET_SIZE size;
	PACKET_TYPE type;

	// contents ref.
	int left, top, right, bottom;
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

// message ����
// [nick name](��)�� [nick name](��)�� �׿����ϴ�.
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
// message ����
// [nick name]: ä�� ����
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
// GameOverScene�� ����� ��Ī ���
// played_character_type�� ����
// GameOverScene�� �������Ǵ� ĳ���Ͱ� �޶�����.
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