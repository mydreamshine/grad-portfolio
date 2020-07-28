#include "BattleServer.h"

int main()
{
	setlocale(LC_ALL, "");
	BattleArena::BATTLESERVER battle;
	battle.Run();
}