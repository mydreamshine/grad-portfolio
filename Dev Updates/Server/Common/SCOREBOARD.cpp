#include "SCOREBOARD.h"
#include <wchar.h>

SCOREBOARD::SCOREBOARD() :
    user_name(),
    count_kill(0),
    count_death(0),
    count_assistance(0),
    totalscore_damage(0),
    totalscore_heal(0)
{
}

SCOREBOARD::SCOREBOARD(wchar_t* user_name) :
    user_name(),
    count_kill(0),
    count_death(0),
    count_assistance(0),
    totalscore_damage(0),
    totalscore_heal(0)
{
    wcscpy_s(this->user_name, user_name);
}
