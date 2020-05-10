#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <conio.h>
#include <string>
#include <vector>
#include <sstream>
#include "../..//LobbyServer/LobbyServer/lobby_protocol.h"
#include "..//..//BattleServer/BattleServer/battle_protocol.h"

#define BUF_SIZE 200

using namespace std;
bool state = false;

bool friend_flag = false;
char last_id[ID_LENGTH];

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

	case CS_PACKET_REQUEST_FRIEND: {
		cs_packet_request_friend* cprf = reinterpret_cast<cs_packet_request_friend*>(ptr);
		strcpy_s(last_id, cprf->id);
		friend_flag = true;
		printf("%s로부터 친구요청이 왔습니다.\n", cprf->id);
	}
		break;

	case SC_PACKET_FRIEND_STATUS: {
		sc_packet_friend_status* packet = reinterpret_cast<sc_packet_friend_status*>(ptr);
		if (packet->status == FRIEND_ONLINE)
			printf("%s가 접속했습니다.\n", packet->id);
		else
			printf("%s가 게임을 종료했습니다.\n", packet->id);
	}
		break;

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
void send_packet_request_friend(SOCKET socket, const char* id)
{
	cs_packet_request_friend packet;
	packet.cdp.size = sizeof(cs_packet_request_friend);
	packet.cdp.type = CS_PACKET_REQUEST_FRIEND;
	strcpy_s(packet.id, 10, id);
	send(socket, (const char*)(&packet), packet.cdp.size, 0);
}
void send_packet_accept_friend(SOCKET socket, const char* id)
{
	cs_packet_accept_friend packet;
	packet.cdp.size = sizeof(cs_packet_accept_friend);
	packet.cdp.type = CS_PACKET_ACCEPT_FRIEND;
	strcpy_s(packet.id, 10, id);
	send(socket, (const char*)(&packet), packet.cdp.size, 0);
}

vector<string> split(string& str, char delimiter) {
	vector<string> internal;
	stringstream ss(str);
	string temp;

	if (getline(ss, temp, delimiter))
		internal.push_back(temp);
	if (getline(ss, temp))
		internal.push_back(temp);
	return internal;
}

void print_help()
{
	printf("명령어 : /enqueue, /dequeue, /add friend_id, /accept\n");
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
	print_help();

	while (true)
	{
		string chat;
		getline(cin, chat);
		
		if (chat[0] == '/')
		{
			vector<string> token = split(chat, ' ');
			if (token[0] == "/enqueue")
				send_packet_default(serverSocket, CS_PACKET_MATCH_ENQUEUE);
			else if (token[0] == "/dequeue")
				send_packet_default(serverSocket, CS_PACKET_MATCH_DEQUEUE);
			else if (token[0] == "/add") {
				if (friend_flag == false) continue;
				send_packet_request_friend(serverSocket, token[1].c_str());
				token[1] += '\0';
				printf("%s에게 친구 요청을 보냈습니다.\n", token[1].c_str());
			}
			else if (token[0] == "/accept") {
				send_packet_accept_friend(serverSocket, last_id);
			}
			else if (token[0] == "/help") {
				print_help();
			}
		}
	}

	recvThread.join();
	closesocket(serverSocket);
	WSACleanup();
}