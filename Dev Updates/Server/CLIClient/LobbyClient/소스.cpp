#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <conio.h>
#include <string>
#include "../..//LobbyServer/LobbyServer/lobby_protocol.h"
#include "..//..//BattleServer/BattleServer/battle_protocol.h"

#define BUF_SIZE 200

using namespace std;
bool state = false;
void battleFunc(int room_id);

int ProcessPacket(void* ptr)
{
	common_default_packet* packet = reinterpret_cast<common_default_packet*>(ptr);

	switch (packet->type)
	{
	case SC_PACKET_LOGIN_OK:
		return SC_PACKET_LOGIN_OK;

	case SC_PACKET_LOGIN_FAIL:
		return SC_PACKET_LOGIN_FAIL;

	case SC_PACKET_MATCH_ENQUEUE:
		cout << "[CHANGE STATE - MATCH ENQUEUE]" << endl;
		state = true;
		break;

	case SC_PACKET_MATCH_DEQUEUE:
		cout << "[CHANGE STATE - MATCH DEQUEUE]" << endl;
		state = false;
		break;

	case SC_PACKET_MATCH_ROOM_INFO: {
		sc_packet_match_room_info* room_info = reinterpret_cast<sc_packet_match_room_info*>(ptr);
		cout << "[CHANGE STATE - Get GameRoom, Access To Battle Server] - " << room_info->room_id << endl;
		thread t{ battleFunc, room_info->room_id };
		t.detach();
		state = false;
	}
		break;

	default:
		printf("Unknown PACKET type [%d]\n", packet->type);
		break;
	}

	return -1;
}

int process_data(void* net_buf, size_t io_byte)
{
	char* ptr = reinterpret_cast<char*>(net_buf);
	static common_default_packet* packet;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	int ret = 0;

	while (0 != io_byte) {
		if (0 == in_packet_size) {
			packet = reinterpret_cast<common_default_packet*>(ptr);
			in_packet_size = packet->size;
		}
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ret = ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
	return ret;
}

void recvFunc(SOCKET serverSocket)
{
	char ReceivedPacket[BUF_SIZE];
	int ReceivedBytes = 0;
	while (true)
	{
		ReceivedBytes = recv(serverSocket, ReceivedPacket, BUF_SIZE, 0);
		if (ReceivedBytes == 0 || ReceivedBytes == SOCKET_ERROR) {
			cout << "[Connection Closed]" << endl;
			return;
		}
		process_data(ReceivedPacket, ReceivedBytes);
	}
}
void loginFunc(SOCKET serverSocket)
{
	string id;
	cs_packet_request_login cprl;
	cprl.cdp.size = sizeof(cs_packet_request_login);
	cprl.cdp.type = CS_PACKET_REQUEST_LOGIN;
	char ReceivedPacket[BUF_SIZE];
	int ReceivedBytes = 0;

	while (true)
	{
		cout << "로그인 아이디 : ";
		cin >> id;
		strcpy_s(cprl.id, 10, id.c_str());
		send(serverSocket, reinterpret_cast<const char*>(&cprl), cprl.cdp.size, 0);

		ReceivedBytes = recv(serverSocket, ReceivedPacket, sizeof(common_default_packet), 0);
		if (ReceivedBytes == 0 || ReceivedBytes == SOCKET_ERROR) {
			cout << "[Connection Closed]" << endl;
			return;
		}
		int ret = process_data(ReceivedPacket, sizeof(common_default_packet));
		
		if (ret == SC_PACKET_LOGIN_OK) {
			cout << "로그인 성공" << endl;
			break;
		}
		else if (ret == SC_PACKET_LOGIN_FAIL)
			cout << "로그인 실패" << endl;
	}
}

void battleFunc(int room_id)
{
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(BATTLESERVER_PORT);
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

	SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	::connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));

	cb_packet_request_login pk;
	pk.cdp.size = sizeof(cb_packet_request_login);
	pk.cdp.type = CB_PACKET_REQUEST_LOGIN;
	pk.room_id = room_id;
	send(serverSocket, reinterpret_cast<char*>(&pk), pk.cdp.size, 0);

	char ReceivedPacket[BUF_SIZE];
	int ReceivedBytes = 0;
	while (true)
	{
		ReceivedBytes = recv(serverSocket, ReceivedPacket, BUF_SIZE, 0);
		if (ReceivedBytes == 0 || ReceivedBytes == SOCKET_ERROR) {
			cout << "[Connection Closed]" << endl;
			closesocket(serverSocket);
			return;
		}
		//process_data(ReceivedPacket, ReceivedBytes);
	}
}

void send_packet_default(SOCKET socket, int type)
{
	common_default_packet cdp;
	cdp.size = sizeof(common_default_packet);
	cdp.type = type;
	send(socket, (const char*)(&cdp), cdp.size, 0);
}

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(LOBBYSERVER_PORT);
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

	SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	::connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	cout << "[CLI CLIENT]" << endl;
	loginFunc(serverSocket);
	thread recvThread{ recvFunc, serverSocket };

	while (true)
	{
		string chat, command;
		cin >> chat;
		
		if (chat[0] == '/')
		{
			command = chat.substr(1, 4);
			if (command == "enq")
				send_packet_default(serverSocket, CS_PACKET_MATCH_ENQUEUE);
			else if (command == "deq")
				send_packet_default(serverSocket, CS_PACKET_MATCH_DEQUEUE);
		}
	}

	recvThread.join();
	closesocket(serverSocket);
	WSACleanup();
}