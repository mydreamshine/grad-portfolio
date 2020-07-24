#pragma once
#include "..\..\Streaming\Streaming_Server\Streaming_Server\packet_struct.h"

class SCOREBOARD
{
public:
    wchar_t user_name[string_len];
	unsigned char count_kill;
	unsigned char count_death;
	unsigned char count_assistance;
	int           totalscore_damage;
	int           totalscore_heal;

	SCOREBOARD();
	SCOREBOARD(wchar_t* user_name);
};

