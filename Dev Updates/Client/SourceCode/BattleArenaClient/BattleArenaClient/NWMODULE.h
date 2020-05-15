#pragma once
#pragma comment(lib, "ws2_32")

#include <Windows.h>
#include <WS2tcpip.h>

class NWMODULE
{
private:


public:
	NWMODULE();
	NWMODULE(HANDLE iocp);
	~NWMODULE();

	void connect_lobby();
	void connect_battle();
};

