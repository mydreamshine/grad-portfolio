//#pragma comment(lib, "ws2_32")

//#include <WS2tcpip.h>
//#include <thread>
//#include "../..//LobbyServer/LobbyServer/lobby_protocol.h"
//#include "..//..//BattleServer/BattleServer/battle_protocol.h"

#include "NWMODULE.h"
#include "NWMODULE.cpp"

#include <iostream>
#include <conio.h>
#include <string>
#include <vector>
#include <sstream>
#define BUF_SIZE 200

using namespace std;
//bool state = false;
//
//bool friend_flag = false;
//char last_id[ID_LENGTH];

//void battleFunc(int room_id);
//
//int ProcessPacket(void* ptr)
//{
//	common_default_packet* packet = reinterpret_cast<common_default_packet*>(ptr);
//
//	switch (packet->type)
//	{
//	case SC_PACKET_LOGIN_OK:
//		return SC_PACKET_LOGIN_OK;
//
//	case SC_PACKET_LOGIN_FAIL:
//		return SC_PACKET_LOGIN_FAIL;
//
//	case CS_PACKET_REQUEST_FRIEND: {
//		cs_packet_request_friend* cprf = reinterpret_cast<cs_packet_request_friend*>(ptr);
//		strcpy_s(last_id, cprf->id);
//		friend_flag = true;
//		printf("%s로부터 친구요청이 왔습니다.\n", cprf->id);
//	}
//		break;
//
//	case SC_PACKET_FRIEND_STATUS: {
//		sc_packet_friend_status* packet = reinterpret_cast<sc_packet_friend_status*>(ptr);
//		if (packet->status == FRIEND_ONLINE)
//			printf("%s가 접속했습니다.\n", packet->id);
//		else
//			printf("%s가 게임을 종료했습니다.\n", packet->id);
//	}
//		break;
//
//	case SC_PACKET_MATCH_ENQUEUE:
//		cout << "[CHANGE STATE - MATCH ENQUEUE]" << endl;
//		state = true;
//		break;
//
//	case SC_PACKET_MATCH_DEQUEUE:
//		cout << "[CHANGE STATE - MATCH DEQUEUE]" << endl;
//		state = false;
//		break;
//
//	case SC_PACKET_MATCH_ROOM_INFO: {
//		sc_packet_match_room_info* room_info = reinterpret_cast<sc_packet_match_room_info*>(ptr);
//		cout << "[CHANGE STATE - Get GameRoom, Access To Battle Server] - " << room_info->room_id << endl;
//		thread t{ battleFunc, room_info->room_id };
//		t.detach();
//		state = false;
//	}
//		break;
//
//	default:
//		printf("Unknown PACKET type [%d]\n", packet->type);
//		break;
//	}
//
//	return -1;
//}

//void battleFunc(int room_id)
//{
//	SOCKADDR_IN serverAddr;
//	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
//	serverAddr.sin_family = AF_INET;
//	serverAddr.sin_port = htons(BATTLESERVER_PORT);
//	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);
//
//	SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
//	::connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
//
//	cb_packet_request_login pk;
//	pk.cdp.size = sizeof(cb_packet_request_login);
//	pk.cdp.type = CB_PACKET_REQUEST_LOGIN;
//	pk.room_id = room_id;
//	send(serverSocket, reinterpret_cast<char*>(&pk), pk.cdp.size, 0);
//
//	char ReceivedPacket[BUF_SIZE];
//	int ReceivedBytes = 0;
//	while (true)
//	{
//		ReceivedBytes = recv(serverSocket, ReceivedPacket, BUF_SIZE, 0);
//		if (ReceivedBytes == 0 || ReceivedBytes == SOCKET_ERROR) {
//			cout << "[Connection Closed]" << endl;
//			closesocket(serverSocket);
//			return;
//		}
//		//process_data(ReceivedPacket, ReceivedBytes);
//	}
//}

class testfunc
{
public:
	void login_ok() { cout << "LOGIN OK" << endl; };
	void enque() { cout << "ENQUEUED" << endl; };
	void deque() { cout << "DEQUEUED" << endl; };
};

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
	testfunc a;
	NWMODULE<testfunc> nw(a);
	
	nw.enroll_callback(6, &testfunc::login_ok);
	nw.enroll_callback(8, &testfunc::enque);
	nw.enroll_callback(9, &testfunc::deque);

	nw.connect_lobby(0);
	nw.request_login();

	cout << "[CLI CLIENT]" << endl;

	print_help();

	while (true)
	{
		nw.update();
		string chat;
		getline(cin, chat);
		
		if (chat[0] == '/')
		{
			vector<string> token = split(chat, ' ');
			if (token[0] == "/enqueue") { nw.match_enqueue(); }
			else if (token[0] == "/dequeue") { nw.match_dequeue(); }
			else if (token[0] == "/add") { nw.add_friend(token[1].c_str()); }
			else if (token[0] == "/accept") {
				//if (friend_flag == false) continue;
				//nw.accept_friend();
			}
			else if (token[0] == "/help") {
				print_help();
			}
		}
		nw.update();
	}
}