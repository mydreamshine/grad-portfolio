#include "CSOCKADDR_IN.h"

CSOCKADDR_IN::CSOCKADDR_IN()
{
	size = sizeof(SOCKADDR_IN);
	memset(&addr, 0, sizeof(SOCKADDR_IN));
}
CSOCKADDR_IN::CSOCKADDR_IN(unsigned long address, short port) :
	CSOCKADDR_IN()
{
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);
}
CSOCKADDR_IN::CSOCKADDR_IN(const char* address, short port) :
	CSOCKADDR_IN()
{
	addr.sin_family = AF_INET;
	InetPton(AF_INET, address, &addr.sin_addr);
	addr.sin_port = htons(port);
}

int* CSOCKADDR_IN::len()
{
	return &size;
}
SOCKADDR* CSOCKADDR_IN::getSockAddr()
{
	return reinterpret_cast<SOCKADDR*>(&addr);
}
