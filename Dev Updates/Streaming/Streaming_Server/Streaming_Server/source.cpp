#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <fstream>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <functional>
#include <vector>

#include "stdafx.h"
#include "Global_Config.h"
#include "packet_struct.h"
#include "EventProcessor.h"
#include "ID_POOLER.h"
#include "Framework.h"
#include "encoder/encoder.h"
#include "..\..\..\Server\Common\OVER_EX.h"
#include <Windows.h>

using namespace std;

constexpr auto FRAME_DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
int MAX_USER = 5;

enum EVENT_TYPE{EV_SEND, EV_RECV, EV_LOBBY_NOTIFY, EV_MATCH_NOTIFY, EV_UPDATE, EV_ENCODE};

struct CLIENT {
	OVER_EX *recv_over;
	int id;
	bool connect;
	SOCKET socket;
	//UDP.
	SOCKET udpSocket;
	int frame_number{ 0 };
	SOCKADDR_IN addr;

	Framework GameFramework;
	EventProcessor FrameworkEventProcessor;
	std::queue<std::unique_ptr<packet_inheritance>> sscs_packetList; // Streaming -> Server
	ENCODER encoder{ SCREEN_WIDTH, SCREEN_HEIGHT, 6000 * 1000, 60 };

	chrono::steady_clock::time_point prev_time;
	chrono::steady_clock::time_point cur_time;
};

bool UDP_MODE;
short server_port;
int NUM_THREADS;
SOCKET listenSocket;
ID_POOLER *IdPooler;
HANDLE g_iocp;
CLIENT *clients;
vector<HANDLE> fence_events;
ResourceManager resourceManager;
std::string additionalAssetPath;
std::wstring config_path{ L".\\STREAMING_CONFIG.ini" };
function<void(int, BYTE*)> send_frame;
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
	wcout << L"���� " << lpMsgBuf << endl;
	while (true);
	LocalFree(lpMsgBuf);
}

void InitializeExternalResource(ResourceManager* ExternalResource)
{
	wcout << "Load Asset... ";
	ExternalResource->OnInit();
	ExternalResource->LoadAsset(&additionalAssetPath);
	wcout << "Done.\n";
}

void InitializeClients()
{
	wcout << "Initializing PLAYERs... ";
	clients = new CLIENT[MAX_USER];

	for (int user_id = 0; user_id < MAX_USER; ++user_id) {
		clients[user_id].id = user_id;
		clients[user_id].socket = INVALID_SOCKET;
		clients[user_id].udpSocket = INVALID_SOCKET;
		clients[user_id].connect = false;

		clients[user_id].recv_over = new OVER_EX{ EV_RECV, MAX_BUFFER_SIZE };
		clients[user_id].FrameworkEventProcessor.nw_module.InitConfig();
		clients[user_id].FrameworkEventProcessor.nw_module.set_iocp(g_iocp, user_id, EV_SEND, EV_LOBBY_NOTIFY, EV_MATCH_NOTIFY);
		clients[user_id].GameFramework.OnInit(0, WND_WIDTH, WND_HEIGHT, L"Framework-" + std::to_wstring(user_id), &resourceManager, &additionalAssetPath);
	}
	wcout << "Done.\n";
}

void gen_default_config()
{
	WritePrivateProfileString(L"STREAMING", L"SERVER_PORT", L"15700", config_path.c_str());
	WritePrivateProfileString(L"STREAMING", L"NUM_THREADS", L"4", config_path.c_str());
	WritePrivateProfileString(L"STREAMING", L"MAX_USER", L"5", config_path.c_str());
	WritePrivateProfileString(L"STREAMING", L"UDP_MODE", L"1", config_path.c_str());
}
void InitConfig()
{
	wprintf(L"Initializing Configs...");
	std::ifstream ini{ config_path.c_str() };
	if (false == ini.is_open())
		gen_default_config();
	ini.close();

	wchar_t buffer[512];
	GetPrivateProfileString(L"STREAMING", L"SERVER_PORT", L"15700", buffer, 512, config_path.c_str());
	server_port = std::stoi(buffer);

	GetPrivateProfileString(L"STREAMING", L"NUM_THREADS", L"4", buffer, 512, config_path.c_str());
	NUM_THREADS = std::stoi(buffer);

	GetPrivateProfileString(L"STREAMING", L"MAX_USER", L"5", buffer, 512, config_path.c_str());
	MAX_USER = std::stoi(buffer);

	GetPrivateProfileString(L"STREAMING", L"UDP_MODE", L"1", buffer, 512, config_path.c_str());
	UDP_MODE = std::stoi(buffer);
	wprintf(L" Done.\n");
}
void PostEvent(int client, int ev_type)
{
	OVER_EX* over_ex = new OVER_EX{ ev_type };
	PostQueuedCompletionStatus(g_iocp, 1, client, over_ex->overlapped());
}

void SendFramePacket(int client, BYTE* frame)
{
	clients[client].encoder.flush();
	OVER_EX* send_over = new OVER_EX{ EV_SEND, (size_t)clients[client].encoder.size() + sizeof(Lpacket_inheritance) };
	Lpacket_inheritance* packet = reinterpret_cast<Lpacket_inheritance*>(send_over->data());
	packet->size = clients[client].encoder.size() + sizeof(Lpacket_inheritance);
	packet->type = SST_TCP_FRAME;
	memcpy(send_over->data() + sizeof(Lpacket_inheritance), clients[client].encoder.buffer(), clients[client].encoder.size());
	WSASend(clients[client].socket, send_over->buffer(), 1, nullptr, 0, send_over->overlapped(), nullptr);
}
void UdpSendFramePacket(int client, BYTE* frame)
{
	clients[client].encoder.flush();

	int total_size = clients[client].encoder.size();
	int buffer_size = 0;
	int split_count = (total_size / PACKET_SPLIT_SIZE) + 1;
	for (int i = 0; i < split_count; ++i) {
		buffer_size = (total_size >= PACKET_SPLIT_SIZE) ? PACKET_SPLIT_SIZE : total_size;
		total_size -= PACKET_SPLIT_SIZE;
		video_packet packet{ clients[client].frame_number, i, split_count, clients[client].encoder.size() , clients[client].encoder.buffer() + i * PACKET_SPLIT_SIZE, buffer_size};
		
		OVER_EX* send_over = new OVER_EX{EV_SEND, &packet, sizeof(packet)};
		WSASend(clients[client].udpSocket, send_over->buffer(), 1, nullptr, 0, send_over->overlapped(), nullptr);
	}
}

void ProcessPacket(int client, void* packet) 
{
	PACKET_TYPE packet_type = reinterpret_cast<packet_inheritance*>(packet)->type;

	switch (packet_type)
	{
	case TSS_KEYUP: {
		tss_packet_keyup* data = reinterpret_cast<tss_packet_keyup*>(packet);
		clients[client].GameFramework.OnKeyUp(data->key);
		break;
	}
	case TSS_KEYDOWN: {
		tss_packet_keydown* data = reinterpret_cast<tss_packet_keydown*>(packet);
		clients[client].GameFramework.OnKeyDown(data->key);
		break;
	}

	case TSS_MOUSEDOWN: {
		tss_packet_mouse_button_down* mouse_button_down_packet = reinterpret_cast<tss_packet_mouse_button_down*>(packet);
		POINT OldCursorPos{ mouse_button_down_packet->x, mouse_button_down_packet->y };
		clients[client].GameFramework.OnKeyDown(mouse_button_down_packet->key, &OldCursorPos); // VK_LBUTTON
		break;
	}

	case TSS_READY_TO_GO: {
		if (UDP_MODE) {
			tss_packet_ready_to_go* port_packet = reinterpret_cast<tss_packet_ready_to_go*>(packet);
			clients[client].addr.sin_port = htons(port_packet->port);
			::connect(clients[client].udpSocket, (sockaddr*)&clients[client].addr, sizeof(clients[client].addr));
		}
		PostEvent(client, EV_UPDATE);
		break;
	}

	case TSS_REQ_RTT: {
		tss_packet_req_rtt* rtt_packet = reinterpret_cast<tss_packet_req_rtt*>(packet);
		sst_packet_ack_rtt ack_packet{ rtt_packet->current_time };
		OVER_EX* send_over = new OVER_EX{ EV_SEND, &ack_packet, (size_t)ack_packet.size};
		WSASend(clients[client].socket, send_over->buffer(), 1, nullptr, 0, send_over->overlapped(), nullptr);
		break;
	}
	}
};

void DisconnectPlayer(int client)
{
	clients[client].connect = false;
	closesocket(clients[client].socket);
	closesocket(clients[client].udpSocket);
	clients[client].GameFramework.OnInitAllSceneProperties();
	clients[client].FrameworkEventProcessor.disconnect();
	IdPooler->DeleteClientId(client);
	wcout << "[CLIENT] >> [Disconnect] >> [" << client << "]" << endl;
}

void DestroyResourceAndClient(ResourceManager* ExternalResource)
{
	for (int user_id = 0; user_id < MAX_USER; ++user_id)
	{
		DisconnectPlayer(user_id);
		clients[user_id].GameFramework.OnDestroy();
	}

	ExternalResource->OnDestroy();
}

void do_worker() {
	DWORD num_byte;
	ULONGLONG key;
	PULONG_PTR p_key = &key;
	WSAOVERLAPPED* p_over;
	while (true)
	{
		GetQueuedCompletionStatus(g_iocp, &num_byte, p_key, &p_over, INFINITE);
		OVER_EX* over_ex = reinterpret_cast<OVER_EX*> (p_over);

		switch (over_ex->event_type())
		{
		case EV_RECV:
		{
			if (num_byte == 0) {
				DisconnectPlayer((int)key);
				continue;
			}
			else {
				ProcessPacket((int)key, over_ex->data());
				DWORD flags = 0;
				over_ex->reset();
				WSARecv(clients[key].socket, over_ex->buffer(), 1, 0, &flags, over_ex->overlapped(), 0);
			}
		}
		break;

		case EV_SEND:
			delete over_ex;
			break;

		case EV_LOBBY_NOTIFY:
			clients[key].FrameworkEventProcessor.nw_module.notify_lobby_recv(num_byte);
			break;

		case EV_MATCH_NOTIFY:
			clients[key].FrameworkEventProcessor.nw_module.notify_battle_recv(num_byte);
			break;

		case EV_UPDATE:
		{
			if (clients[key].connect != true) {
				delete over_ex;
				break;
			}
			clients[key].FrameworkEventProcessor.GenerateExternalEventsFrom();
			clients[key].GameFramework.ProcessEvents(clients[key].FrameworkEventProcessor.GetExternalEvents());
			std::queue<std::unique_ptr<EVENT>> GeneratedEvents;
			clients[key].GameFramework.OnUpdate(GeneratedEvents);
			clients[key].GameFramework.OnRender();
			clients[key].FrameworkEventProcessor.ProcessGeneratedEvents(GeneratedEvents);
			++clients[key].frame_number;
			PostEvent((int)key, EV_ENCODE);
			delete over_ex;
		}
		break;

		case EV_ENCODE:
			if (clients[key].connect != true) {
				delete over_ex;
				break;
			}
			BYTE* Frame = clients[key].GameFramework.GetFrameData();
			clients[key].encoder.encode((const char*)Frame);
			send_frame((int)key, Frame);
			PostEvent((int)key, EV_UPDATE);
			delete over_ex;
			break;
		}
	}
}

void LoadadditionalAssetPathTXT()
{
	ifstream reader("additionalAssetPath.txt");
	if(reader.is_open() != true) error_display("LoadadditionalAssetPathTXT()", GetLastError());
	getline(reader, additionalAssetPath);
	reader.close();
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_BREAK_EVENT:
	default:
		DestroyResourceAndClient(&resourceManager);
		closesocket(listenSocket);
		WSACleanup();
		cout << "Streaming Close\n";
		//system("pause");
		exit(0);
	}
	return FALSE;
}

_Use_decl_annotations_
#define MAX_CORES 4
int main()
{
	wcout.imbue(std::locale("korean"));
	setlocale(LC_ALL, "korean");
	InitConfig();

	if (UDP_MODE)
		send_frame = UdpSendFramePacket;
	else
		send_frame = SendFramePacket;

	IdPooler = new ID_POOLER{ MAX_USER };
	cout << "Concurrent Threads Supported By H/W Cores: " << NUM_THREADS << endl;

	BOOL fSuccess = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	LoadadditionalAssetPathTXT();
	InitializeExternalResource(&resourceManager);
	InitializeClients();
	
	//Setup SOCKET
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) == SOCKET_ERROR) 
		error_display("WSAStartup()", WSAGetLastError());

	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) 
		error_display("ListenSocket()", WSAGetLastError());

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(server_port);

	if(::bind(listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		error_display("bind()", WSAGetLastError());
	if(listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		error_display("listen()", WSAGetLastError());

	SOCKADDR_IN clientAddr;
	memset(&clientAddr, 0, sizeof(SOCKADDR_IN));
	int addrlen = sizeof(SOCKADDR_IN);

	vector<thread> threads;
	for (int i = 0; i < (int)NUM_THREADS; ++i)
		threads.emplace_back(do_worker);

	if(UDP_MODE)
		wprintf(L"[Server On - UDP]\n");
	else
		wprintf(L"[Server On - TCP]\n");
	while (true)
	{
		//Accept Clients
		SOCKET clientSocket = accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrlen);
		if (clientSocket == INVALID_SOCKET)
			error_display("accept()", WSAGetLastError());

		int new_id = IdPooler->GetNewClientId();
		clients[new_id].connect = true;
		clients[new_id].socket = clientSocket;

		//Setup UDP.
		if (UDP_MODE) {
			clients[new_id].addr = clientAddr;
			clients[new_id].udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
			clients[new_id].frame_number = 0;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(clients[new_id].udpSocket), g_iocp, static_cast<ULONG_PTR>(new_id), 0);
		}

		wcout << "[CLIENT] >> [Accept] >> [" << new_id << "]" << endl;

		DWORD flags = 0;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, static_cast<ULONG_PTR>(new_id), 0);

		//Recv ���
		clients[new_id].recv_over->reset();
		int ret = WSARecv(clientSocket, clients[new_id].recv_over->buffer(), 1, NULL,
			&flags, clients[new_id].recv_over->overlapped(), NULL);
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
	delete[] clients;
	delete IdPooler;
}