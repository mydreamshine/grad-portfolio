#pragma once
#include <WS2tcpip.h>

class CSOCKADDR_IN
{
private:
	int size;
	SOCKADDR_IN addr;

public:
	CSOCKADDR_IN();
	CSOCKADDR_IN(long addr, short port);

	int* len();
	SOCKADDR* getSockAddr();
};