#pragma once
#pragma comment(lib, "ws2_32")
#include <winsock2.h>



class NWManager
{
private:
	SOCKET client_socket;
	SOCKADDR_IN serverAddr;
public:
	NWManager();
	~NWManager();

	void init();
	void end();

	void connect(const char* addr, USHORT port);
	void disconnect();
};

