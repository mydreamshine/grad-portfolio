#pragma once
#pragma pack(push, 1)

enum PACKET_TYPE {
    //SERVER TO CLIENT
    SC_PLAYER_UID,           //플레이어 UID 통보, sc_player_uid 구조체 사용
    SC_CREATE_PLAYER,        //플레이어 생성(리스폰), sc_create_player 구조체 사용
    SC_CREATE_BULLET,        //총알 생성, sc_create_bullet 구조체 사용
    SC_DESTROY_PLAYER,       //플레이어 파괴(죽음), sc_destroy_player 구조체 사용
    SC_DESTROY_BULLET,       //총알 오브젝트 파괴, sc_destroy_bullet 구조체 사용
    SC_PLAYER_INFO,          //유저의 위치정보 패킷, sc_player_info 구조체 사용
    SC_BULLET_INFO,          //총알의 위치정보 패킷, sc_bullet_info 구조체 사용
    SC_HIT,                  //플레이어 타격, sc_hit 구조체 사용
    SC_GAME_START,           //게임 시작, default_packet 구조체 사용
    SC_GAME_END,             //게임 종료, default_packet 구조체 사용

    //CLIENT TO SERVER
    CS_KEYUP,                //플레이어 키보드 입력(UP), cs_key_info 구조체 사용
    CS_KEYDOWN,              //플레이어 키보드 입력(DOWN), cs_key_info 구조체 사용
    CS_ATTACK                //플레이어의 공격, cs_attack 구조체 사용
};

struct default_packet
{
    char size;
    char type;
};

struct sc_player_uid
{
    char size;
    char type;
    char uid;   //플레이어의 UID
};

struct sc_create_player
{
    char size;
    char type;
    char uid;       //생성될 플레이어의 UID
    char hero;      //생성될 HERO 타입
    int hp;         //체력
    float pos[3];   //위치 벡터
    float rot[3];   //방향 벡터 - Degree
};

struct sc_create_bullet
{
    char size;
    char type;
    char shooter;   //쏜 사람
    char uid;       //생성될 총알의 UID
    float pos[3];   //위치 벡터
    float rot[3];   //방향 벡터 - Degree
};

struct sc_destroy_player
{
    char size;
    char type;
    char uid;       //해당 UID의 플레이어 사망
};

struct sc_destroy_bullet
{
    char size;
    char type;
    char uid;       //해당 UID의 총알 파괴
};

struct sc_player_info
{
    char size;
    char type;
    char uid;       //해당 UID 플레이어 위치정보
    float pos[3];   //위치 벡터
    float rot[3];   //방향 벡터 - Degree
};

struct sc_bullet_info
{
    char size;
    char type;
    char uid;       //해당 UID 총알의 위치정보
    float pos[3];   //위치 벡터
    float rot[3];   //방향 벡터 - Degree
};

struct cs_key_info
{
    char size;
    char type;      //CS_KEYUP, CS_KEYDOWN
    char uid;       //해당 UID 플레이어 키 입력
    char key;       //키(ASCII)
};

struct cs_attack
{
    char size;
    char type;
    char uid;       //해당 UID 플레이어의 공격
    float dir[3];   //Normalize된 공격방향
};

struct sc_hit
{
    char size;
    char type;
    char uid;       //해당 UID 플레이어의 피격
    int damage;     //데미지
};

#pragma pack(pop)