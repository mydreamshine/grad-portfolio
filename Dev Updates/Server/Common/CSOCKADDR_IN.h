#pragma once
#include <WS2tcpip.h>

class CSOCKADDR_IN
{
private:
	int size;
	SOCKADDR_IN addr;

public:
	CSOCKADDR_IN();
	CSOCKADDR_IN(unsigned long addr, short port);
	CSOCKADDR_IN(const char* addr, short port);

	int* len();
	SOCKADDR* getSockAddr();
};