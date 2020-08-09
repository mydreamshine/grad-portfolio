#pragma comment(lib, "ws2_32")
#include "BATTLESERVER.h"
#include "BBManager.h"
#include <iostream>


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
		LoadConfig();
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
		CSOCKADDR_IN serverAddr{ INADDR_ANY, SERVER_PORT };

		wprintf(L"Initializing ListenSocket...");
		if (SOCKET_ERROR == ::bind(m_listenSocket, serverAddr.getSockAddr(), *serverAddr.len()))
			error_display("Error at Bind()", WSAGetLastError());
		if (SOCKET_ERROR == ::listen(m_listenSocket, SOMAXCONN))
			error_display("Error at Listen()", WSAGetLastError());
		wprintf(L" Done.\n");

#ifndef TEST_FIELD
		wprintf(L"Waiting LobbyServer...");
		CSOCKADDR_IN LobbyAddr{};
		m_Lobby.socket = accept(m_listenSocket, LobbyAddr.getSockAddr(), LobbyAddr.len());
		m_Lobby.recv_over.init(RECV_BUF_SIZE);
		m_Lobby.recv_over.set_event(EV_LOBBY);
		m_Lobby.set_recv();
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_Lobby.socket), m_iocp, reinterpret_cast<ULONG_PTR>(&m_Lobby), 0);
		wprintf(L" Done.\n");
#endif
		
	}
	void BATTLESERVER::InitThreads()
	{
		wprintf(L"Initializing Threads...");
		for (UINT i = 0; i < NUM_THREADS; ++i)
			m_threads.emplace_back(&BATTLESERVER::do_worker, this);
		m_threads.emplace_back(&BATTLESERVER::do_timer, this);
		wprintf(L" Done.\n");
	}
	void BATTLESERVER::InitRooms()
	{
		wprintf(L"Initializing Rooms...");
		m_Rooms = new ROOM*[MAX_ROOM];
		roomListLock.lock();
		for (int i = 0; i < MAX_ROOM; ++i) {
			roomList.emplace_back(i);
			m_Rooms[i] = nullptr;
		}

#ifdef TEST_FIELD
		roomList.pop_front();
		m_Rooms[0] = make_game_mode(GAMEMODE_DM);
		m_Rooms[0]->init();
#endif

		roomListLock.unlock();
		wprintf(L" Done.\n");
	}

	void BATTLESERVER::LoadConfig()
	{
		wprintf(L"Load Config...");
		BBManager::instance().load_bb();
		BBManager::instance().load_config();
		
		SERVER_PORT = BBManager::instance().SERVER_PORT;
		NUM_THREADS = BBManager::instance().NUM_THREADS;
		UPDATE_INTERVAL = BBManager::instance().UPDATE_INTERVAL;
		MAX_ROOM = BBManager::instance().MAX_ROOM;
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
			client->recv_over.init(RECV_BUF_SIZE);
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
			OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
			if (0 == ReceivedBytes) {
				if (EV_SEND == over_ex->event_type())
					delete over_ex;
				else
					disconnect_player(client);
				continue;
			}

			switch (over_ex->event_type())
			{
			case EV_RECV:
				client->room_lock.lock();
				if(client->room != nullptr)
					client->room->process_packet(client, ReceivedBytes);
				client->room_lock.unlock();
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
				//if game is done.
				if (true == m_Rooms[key]->update(0.0f)) {
					wprintf(L"[ROOM %lld] - GAME END\n", key);
					m_Rooms[key]->end();

					disconnect_player_from_room(key);

					delete m_Rooms[key];
					m_Rooms[key] = nullptr;
					set_empty_room(key);
				}
				//else push next update event
				else add_event(static_cast<int>(key), EV_UPDATE, UPDATE_INTERVAL);
				delete over_ex;
				break;
			}

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

	void BATTLESERVER::disconnect_player(CLIENT* client)
	{
		std::wcout << L"[CLIENT - " << (client->socket) << L"] Disconnected" << std::endl;

		client->room_lock.lock();
		if (client->state == ST_INGAME) {
			client->room->disconnect(client->socket);
			client->state = ST_ENDGAME;
			if (client->socket != INVALID_SOCKET) {
				closesocket(client->socket);
				client->socket = INVALID_SOCKET;
			}
			client->room_lock.unlock();
		}
		else {
			client->room_lock.unlock();
			if (client != &m_Lobby)
				delete client;
		}
	}

	void BATTLESERVER::disconnect_player_from_room(int key)
	{
		m_Rooms[key]->client_lock.lock();
		for (auto& cl : m_Rooms[key]->clients)
		{
			cl.second->room_lock.lock();
			if (cl.second->state == ST_INGAME) {
				cl.second->state = ST_ENDGAME;
				cl.second->room_lock.unlock();
				closesocket(cl.second->socket);
			} 
			else if (cl.second->state == ST_ENDGAME) {
				cl.second->room_lock.unlock();
				delete cl.second;
			}
		}
		m_Rooms[key]->client_lock.unlock();
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

			const EVENT& ev = timer_queue.top();

			if (ev.wakeup_time > high_resolution_clock::now()) {
				timer_lock.unlock();
				this_thread::sleep_for(3ms);
				continue;
			}
			EVENT p_ev = ev;
			timer_queue.pop();
			timer_lock.unlock();

			OVER_EX* over_ex = new OVER_EX;
			::memset(over_ex, 0, sizeof(OVER_EX));
			over_ex->set_event(p_ev.event_type);

			PostQueuedCompletionStatus(m_iocp, 1, p_ev.obj_id, over_ex->overlapped());
		}
	}
	void BATTLESERVER::add_timer(EVENT& ev) {
		timer_lock.lock();
		timer_queue.push(ev);
		timer_lock.unlock();
	}
	void BATTLESERVER::add_event(int client, EVENT_TYPE et, int milisec_delay)
	{
		EVENT ev{ client, et, high_resolution_clock::now() + chrono::milliseconds(milisec_delay)};
		add_timer(ev);
	}

	void BATTLESERVER::send_packet(CLIENT* client, void* buff)
	{
		OVER_EX* send_over = new OVER_EX {EV_SEND, buff};
		WSASend(client->socket, send_over->buffer(), 1, 0, 0, send_over->overlapped(), 0);
	}
	void BATTLESERVER::send_packet_default(CLIENT* client, int TYPE)
	{
		packet_inheritance cdp;
		cdp.size = sizeof(cdp);
		cdp.type = TYPE;
		send_packet(client, &cdp);
	}
	void BATTLESERVER::send_packet_response_room(int room_id)
	{
		bs_packet_response_room packet{ room_id };
		send_packet(&m_Lobby, &packet);
	}

	void BATTLESERVER::ProcessLobbyPacket(void* buffer)
	{
		packet_inheritance* packet = reinterpret_cast<packet_inheritance*>(buffer);
		switch (packet->type)
		{
		case SB_PACKET_REQUEST_ROOM: {
			sb_packet_request_room* room_packet = reinterpret_cast<sb_packet_request_room*>(buffer);
			int empty_room = get_empty_room();
			if (empty_room != -1) {
				m_Rooms[empty_room] = make_game_mode(room_packet->mode);
				m_Rooms[empty_room]->init();
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
		packet_inheritance* pk = reinterpret_cast<packet_inheritance*>(buffer);
		switch (pk->type)
		{
		case SSCS_TRY_MATCH_LOGIN: {
			sscs_packet_try_match_login* packet = reinterpret_cast<sscs_packet_try_match_login*>(buffer);
			client->recv_over.set_event(EV_RECV);

			m_Rooms[packet->room_id]->client_lock.lock();
			m_Rooms[packet->room_id]->clients.emplace(make_pair(packet->uid, client));

			client->room_lock.lock();
			client->room = m_Rooms[packet->room_id];
			client->state = ST_INGAME;
			client->room_lock.unlock();
			m_Rooms[packet->room_id]->client_lock.unlock();

			//if all users are Connected, Start Game
			if (true == m_Rooms[packet->room_id]->regist(packet->uid, client->socket, client->recv_over.data())) {
				wprintf(L"[ROOM %d] - GAME START\n", packet->room_id);
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
		default:
			newgame = new DMRoom{};
			break;
		}
		return newgame;
	}
}