#include "stdafx.h"
#include "NWManager.h"


NWManager::NWManager()
{
	ZeroMemory(&serverAddr, sizeof(serverAddr));
}
NWManager::~NWManager()
{
}

void NWManager::init() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);
}
void NWManager::end() {
	WSACleanup();
}
void NWManager::connect(const char* addr, USHORT port)
{
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(addr);

	::connect(client_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
}
void NWManager::disconnect()
{
	closesocket(client_socket);
}