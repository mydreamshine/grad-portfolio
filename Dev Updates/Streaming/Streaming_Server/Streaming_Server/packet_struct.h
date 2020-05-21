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

////////////////////////////////////// 통신 패킷 타입 /////////////////////////////////////
#pragma pack(push, 1)
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
#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////////////////////