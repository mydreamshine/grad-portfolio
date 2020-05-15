#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <queue>

#include <vector>
#include "Global_Config.h"
#include "ID_POOLER.h"
#include "Client/GameFramework.h"
#include "encoder.h"

using namespace std;

constexpr auto FRAME_DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
constexpr auto MAX_BUFFER = 100;
constexpr auto MAX_USER = 4;

enum EVENT_TYPE{EV_SEND, EV_RECV, EV_UPDATE, EV_ENCODE};

struct OVER_EX {
	WSAOVERLAPPED over;
	WSABUF	wsabuf[1];
	char	net_buf[FRAME_DATA_SIZE];
	EVENT_TYPE	event_type;
};
struct CLIENT {
	OVER_EX *recv_over;
	int id;
	bool connect;
	SOCKET socket;
	CGameFramework GameFramework;
	ENCODER encoder{ SCREEN_WIDTH, SCREEN_HEIGHT, 6000 * 1000, 60 };
};

HANDLE g_iocp;
ID_POOLER IdPooler(MAX_USER);
CLIENT clients[MAX_USER];
vector<HANDLE> fence_events;

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << L"에러 " << lpMsgBuf << endl;
	while (true);
	LocalFree(lpMsgBuf);
}

void InitializeClients()
{
	wcout << "Initializing PLAYERs... ";
	for (int user_id = 0; user_id < MAX_USER; ++user_id) {
		clients[user_id].id = user_id;
		clients[user_id].socket = INVALID_SOCKET;
		clients[user_id].connect = false;

		clients[user_id].recv_over = new OVER_EX;
		memset(clients[user_id].recv_over, 0, sizeof(OVER_EX));
		clients[user_id].recv_over->event_type = EV_RECV;
		clients[user_id].recv_over->wsabuf->buf = clients[user_id].recv_over->net_buf;
		clients[user_id].recv_over->wsabuf->len = sizeof(clients[user_id].recv_over->net_buf);

		clients[user_id].GameFramework.OnCreate();
		fence_events.emplace_back(clients[user_id].GameFramework.GetFenceEvent());
	}
	wcout << "Done.\n";
}

void SendFramePacket(int client, BYTE* frame)
{
	OVER_EX* send_over = new OVER_EX;
	::memset(send_over, 0x00, sizeof(OVER_EX));
	send_over->event_type = EV_SEND;

	int* size = (int*)send_over->net_buf;
	*size = clients[client].encoder.flush(send_over->net_buf + 4);
	//memcpy(send_over->net_buf, frame, FRAME_DATA_SIZE);

	send_over->wsabuf[0].buf = send_over->net_buf;
	send_over->wsabuf[0].len = *size + 4;
	WSASend(clients[client].socket, send_over->wsabuf, 1, nullptr, 0, &send_over->over, nullptr);
}
void ProcessPacket(int client, void* packet) 
{
	char data = *(char*)packet;
	switch (data)
	{
	case 0:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYDOWN, 'W', 0);
		break;
	case 1:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYDOWN, 'S', 0);
		break;
	case 2:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYUP, 'W', 0);
		break;
	case 3:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYUP, 'S', 0);
		break;
	case 4:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYDOWN, 'A', 0);
		break;
	case 5:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYDOWN, 'D', 0);
		break;
	case 6:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYUP, 'A', 0);
		break;
	case 7:
		clients[client].GameFramework.OnProcessingKeyboardMessage(0, WM_KEYUP, 'D', 0);
		break;
	}
};


void PostEvent(int client, EVENT_TYPE ev_type)
{
	OVER_EX* over_ex = new OVER_EX;
	over_ex->event_type = ev_type;
	PostQueuedCompletionStatus(g_iocp, 1, client, &over_ex->over);
}
void DisconnectPlayer(int client)
{
	closesocket(clients[client].socket);
	clients[client].connect = false;
	IdPooler.DeleteClientId(client);
	wcout << "[CLIENT] >> [Disconnect] >> [" << client << "]" << endl;
}
void do_worker() {
	while (true)
	{
		DWORD num_byte;
		ULONGLONG key;
		PULONG_PTR p_key = &key;
		WSAOVERLAPPED* p_over;

		GetQueuedCompletionStatus(g_iocp, &num_byte, p_key, &p_over, INFINITE);
		OVER_EX* over_ex = reinterpret_cast<OVER_EX*> (p_over);

		if (num_byte == 0) {
			DisconnectPlayer(key);
			if (over_ex->event_type == EV_SEND) delete over_ex;
			continue;
		}

		switch (over_ex->event_type)
		{
		case EV_RECV:
		{
			ProcessPacket(key, over_ex->net_buf);
			DWORD flags = 0;
			::memset(&over_ex->over, 0x00, sizeof(WSAOVERLAPPED));
			WSARecv(clients[key].socket, over_ex->wsabuf, 1, 0, &flags, &over_ex->over, 0);
		}
		break;

		case EV_SEND:
			delete over_ex;
			break;

		case EV_UPDATE:
			if (clients[key].connect != true) {
				delete over_ex;
				break;
			}
			clients[key].GameFramework.FrameAdvance();
			delete over_ex;
			PostEvent(key, EV_ENCODE);
			break;

		case EV_ENCODE:
			if (clients[key].connect != true) {
				delete over_ex;
				break;
			}
			BYTE* Frame = clients[key].GameFramework.GetFrameData();
			clients[key].encoder.encode((const char*)Frame);
			SendFramePacket(key, Frame);
			PostEvent(key, EV_UPDATE);
			delete over_ex;
			break;
		}
	}
}
void do_checker()
{
	while (true)
	{
		int client = WaitForMultipleObjects(MAX_USER, fence_events.data(), false, INFINITE);
		ResetEvent(fence_events[client]);
		PostEvent(client, EV_ENCODE);
	}
}

int main()
{
	wcout.imbue(std::locale("korean"));
	setlocale(LC_ALL, "korean");

	InitializeClients();
	//InitializeGraphicsPipeLine();
	
	//Setup SOCKET
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) == SOCKET_ERROR) 
		error_display("WSAStartup()", WSAGetLastError());

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) 
		error_display("ListenSocket()", WSAGetLastError());

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);

	if(::bind(listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		error_display("bind()", WSAGetLastError());
	if(listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		error_display("listen()", WSAGetLastError());

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	SOCKADDR_IN clientAddr;
	memset(&clientAddr, 0, sizeof(SOCKADDR_IN));
	int addrlen = sizeof(SOCKADDR_IN);

	vector<thread> threads;
	for (int i = 0; i < 4; ++i)
		threads.emplace_back(do_worker);
	//threads.emplace_back(do_checker);

	while (true)
	{
		//Accept Clients
		SOCKET clientSocket = accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrlen);
		if (clientSocket == INVALID_SOCKET)
			error_display("accept()", WSAGetLastError());

		int new_id = IdPooler.GetNewClientId();
		clients[new_id].connect = true;
		clients[new_id].socket = clientSocket;
		wcout << "[CLIENT] >> [Accept] >> [" << new_id << "]" << endl;

		DWORD flags = 0;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, static_cast<ULONG_PTR>(new_id), 0);

		PostEvent(new_id, EV_UPDATE);

		//Recv 등록
		::memset(&clients[new_id].recv_over->over, 0, sizeof(WSAOVERLAPPED));
		int ret = WSARecv(clientSocket, clients[new_id].recv_over->wsabuf, 1, NULL,
			&flags, &(clients[new_id].recv_over->over), NULL);
		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("WSARecv Error :", err_no);
		}
	}

	for (int i = 0; i < threads.size(); ++i)
		threads[i].join();

	closesocket(listenSocket);
	WSACleanup();
}