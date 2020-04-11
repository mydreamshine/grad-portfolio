#pragma comment(lib, "ws2_32")
#include "BATTLESERVER.h"
#include <iostream>
#include "battle_protocol.h"
#include "..//..//LobbyServer/LobbyServer/lobby_protocol.h"


using namespace std;
using namespace chrono;

namespace BattleArena {
	enum EVENT_TYPE { EV_RECV, EV_SEND, EV_LOBBY, EV_AUTHO, EV_UPDATE };
	enum CL_STATE { ST_QUEUE, ST_IDLE, ST_PLAY };

	void error_display(const char* msg, int err_no)
	{
		WCHAR* lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err_no,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf, 0, NULL);
		std::cout << msg;
		std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
		while (true);
		LocalFree(lpMsgBuf);
	}

	BATTLESERVER::BATTLESERVER() :
		m_listenSocket(INVALID_SOCKET)
	{
		wprintf(L"[BATTLE SERVER]\n");
		InitRooms();
		InitWSA();
		InitThreads();
	}
	BATTLESERVER::~BATTLESERVER()
	{
		for (UINT i = 0; i < m_threads.size(); ++i)
			m_threads[i].join();
		closesocket(m_listenSocket);
		WSACleanup();
	}

	void BATTLESERVER::InitWSA()
	{
		WSADATA wsa;
		if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsa))
			error_display("Error at WSAStartup()", WSAGetLastError());

		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, 0);

		m_listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		CSOCKADDR_IN serverAddr{ INADDR_ANY, BATTLESERVER_PORT };

		wprintf(L"Initializing ListenSocket...");
		if (SOCKET_ERROR == ::bind(m_listenSocket, serverAddr.getSockAddr(), *serverAddr.len()))
			error_display("Error at Bind()", WSAGetLastError());
		if (SOCKET_ERROR == ::listen(m_listenSocket, SOMAXCONN))
			error_display("Error at Listen()", WSAGetLastError());
		wprintf(L" Done.\n");

		wprintf(L"Waiting LobbyServer...");
		CSOCKADDR_IN LobbyAddr{};
		m_Lobby.socket = accept(m_listenSocket, LobbyAddr.getSockAddr(), LobbyAddr.len());
		m_Lobby.recv_over.init();
		m_Lobby.recv_over.set_event(EV_LOBBY);
		m_Lobby.set_recv();
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_Lobby.socket), m_iocp, reinterpret_cast<ULONG_PTR>(&m_Lobby), 0);
		wprintf(L" Done.\n");
	}
	void BATTLESERVER::InitThreads()
	{
		wprintf(L"Initializing Threads...");
		for (UINT i = 0; i < 4; ++i)
			m_threads.emplace_back(&BATTLESERVER::do_worker, this);
		m_threads.emplace_back(&BATTLESERVER::do_timer, this);
		wprintf(L" Done.\n");
	}
	void BATTLESERVER::InitRooms()
	{
		wprintf(L"Initializing Rooms...");
		roomListLock.lock();
		for (int i = 0; i < MAX_ROOM; ++i) {
			roomList.emplace_back(i);
			m_Rooms[i] = nullptr;
		}
		roomListLock.unlock();
		wprintf(L" Done.\n");
	}
	void BATTLESERVER::Run()
	{
		wprintf(L"[SERVER ON]\n");
		CSOCKADDR_IN clientAddr{};
		SOCKET clientSocket{};
		while (true)
		{
			clientSocket = accept(m_listenSocket, clientAddr.getSockAddr(), clientAddr.len());
			CLIENT* client = new CLIENT{};
			client->socket = clientSocket;
			client->recv_over.init();
			client->recv_over.set_event(EV_AUTHO);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(client->socket), m_iocp, reinterpret_cast<ULONG_PTR>(client), 0);
			client->set_recv();
			std::wcout << L"[CLIENT - " << clientSocket << L"] Accept" << std::endl;
		}
	}
	void BATTLESERVER::do_worker()
	{
		DWORD ReceivedBytes;
		ULONGLONG key;
		OVERLAPPED* over;

		while (true)
		{
			GetQueuedCompletionStatus(m_iocp, &ReceivedBytes, &key, &over, INFINITE);
			CLIENT* client = reinterpret_cast<CLIENT*>(key);
			if (0 == ReceivedBytes) {
				std::wcout << L"[CLIENT - " << key << L"] Disconnected" << std::endl;
				if (client->room != nullptr)
					client->room->disconnect(client);
				if(client->socket != INVALID_SOCKET)
					closesocket(client->socket);
				if(client != &m_Lobby)
					delete client;
				continue;
			}

			OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
			switch (over_ex->event_type())
			{
			case EV_RECV:
				client->room->process_packet(client, ReceivedBytes);
				client->set_recv();
				break;

			case EV_AUTHO:
				ProcessAuthoPacket(client, client->recv_over.data());
				client->set_recv();
				break;

			case EV_LOBBY:
				ProcessLobbyPacket(m_Lobby.recv_over.data());
				m_Lobby.set_recv();
				break;

			case EV_UPDATE:
			{
				//Template Error
				duration<float>fElapsedTime {high_resolution_clock::now() - m_Rooms[key]->last_update_time};
				wprintf(L"[ROOM %lld] - Update / %f sec\n", key, fElapsedTime.count());
				if (true == m_Rooms[key]->update(fElapsedTime.count())) {
					wprintf(L"[ROOM %lld] - GAME END\n", key);
					//m_Rooms[key]->end();
					delete m_Rooms[key];
					set_empty_room(key);
				}
				else add_event(static_cast<int>(key), EV_UPDATE, UPDATE_INTERVAL);
			}
			delete over_ex;
				break;

			case EV_SEND:
				delete over_ex;
				break;

			default:
				std::wcout << "Unknown Event From " << client << std::endl;
				while (true);
				break;
			}
		}
	}

	void BATTLESERVER::do_timer()
	{
		while (true) {
			timer_lock.lock();
			if (true == timer_queue.empty()) {
				timer_lock.unlock();
				this_thread::sleep_for(3ms);
				continue;
			}

			const EVENT<EVENT_TYPE>& ev = timer_queue.top();

			if (ev.wakeup_time > high_resolution_clock::now()) {
				timer_lock.unlock();
				this_thread::sleep_for(3ms);
				continue;
			}
			EVENT<EVENT_TYPE> p_ev = ev;
			timer_queue.pop();
			timer_lock.unlock();

			OVER_EX* over_ex = new OVER_EX;
			::memset(over_ex, 0, sizeof(OVER_EX));
			over_ex->set_event(p_ev.event_type);

			PostQueuedCompletionStatus(m_iocp, 1, p_ev.obj_id, over_ex->overlapped());
		}
	}
	void BATTLESERVER::add_timer(EVENT<EVENT_TYPE>& ev) {
		timer_lock.lock();
		timer_queue.push(ev);
		timer_lock.unlock();
	}
	void BATTLESERVER::add_event(int client, EVENT_TYPE et, int milisec_delay)
	{
		EVENT<EVENT_TYPE> ev{ client, high_resolution_clock::now() + chrono::milliseconds(milisec_delay), et};
		add_timer(ev);
	}

	void BATTLESERVER::send_packet(CLIENT* client, void* buff)
	{
		OVER_EX* send_over = new OVER_EX{EV_SEND, buff};
		WSASend(client->socket, send_over->buffer(), 1, 0, 0, send_over->overlapped(), 0);
	}
	void BATTLESERVER::send_packet_default(CLIENT* client, int TYPE)
	{
		common_default_packet cdp;
		cdp.size = sizeof(cdp);
		cdp.type = TYPE;
		send_packet(client, &cdp);
	}
	void BATTLESERVER::send_packet_response_room(int room_id)
	{
		bs_packet_response_room packet;
		packet.cdp.size = sizeof(packet);
		packet.cdp.type = BS_PACKET_RESPONSE_ROOM;
		packet.room_id = room_id;
		send_packet(&m_Lobby, &packet);
	}

	void BATTLESERVER::ProcessPacket(CLIENT* client, void* buffer)
	{
		common_default_packet* packet = reinterpret_cast<common_default_packet*>(buffer);
		switch (packet->type)
		{
		default:
			printf("[CLIENT %lld] - Unknown Packet\n", client->socket);
			while (true);
			break;
		}
	}
	void BATTLESERVER::ProcessLobbyPacket(void* buffer)
	{
		common_default_packet* packet = reinterpret_cast<common_default_packet*>(buffer);
		switch (packet->type)
		{
		case SB_PACKET_REQUEST_ROOM: {
			sb_packet_request_room* room_packet = reinterpret_cast<sb_packet_request_room*>(buffer);
			int empty_room = get_empty_room();
			if (empty_room != -1) {
				m_Rooms[empty_room] = make_game_mode(room_packet->mode);
			}
			wprintf(L"[LOBBY] - Room Response %d", empty_room);
			send_packet_response_room(empty_room);
		}
			break;

		default:
			wprintf(L"[LOBBY] - Unknown Packet\n");
			while (true);
			break;
		}
	}
	void BATTLESERVER::ProcessAuthoPacket(CLIENT* client, void* buffer)
	{
		common_default_packet* pk = reinterpret_cast<common_default_packet*>(buffer);
		switch (pk->type)
		{
		case CB_PACKET_REQUEST_LOGIN: {
			cb_packet_request_login* packet = reinterpret_cast<cb_packet_request_login*>(buffer);
			client->recv_over.set_event(EV_RECV);
			client->room = m_Rooms[packet->room_id];
			if (true == m_Rooms[packet->room_id]->regist(client)) {
				wprintf(L"[ROOM %d] - GAME START\n", packet->room_id);
				//All user COnnected, Start Game
				m_Rooms[packet->room_id]->last_update_time = high_resolution_clock::now();
				m_Rooms[packet->room_id]->start();
				add_event(packet->room_id, EV_UPDATE, UPDATE_INTERVAL);
			}
		}
			break;

		default:
			wprintf(L"[CLIENT AUTHO] - Unknown Packet\n");
			while (true);
			break;
		}
	}

	//get the empty_room number from roomList
	int BATTLESERVER::get_empty_room()
	{
		int empty_room;
		roomListLock.lock();
		if (false == roomList.empty()) {
			empty_room = roomList.front();
			roomList.pop_front();
		}
		else empty_room = -1;
		roomListLock.unlock();
		return empty_room;
	}
	//set the empty_room number to roomList
	void BATTLESERVER::set_empty_room(int num)
	{
		roomListLock.lock();
		roomList.emplace_back(num);
		roomListLock.unlock();
	}
	//make gameroom(normal, deathmatch, etc...)
	ROOM* BATTLESERVER::make_game_mode(int mode)
	{
		ROOM* newgame = nullptr;
		switch (mode) {
		case GAMEMODE_NGP:
			newgame = new NGPROOM;
		}
		return newgame;
	}
}