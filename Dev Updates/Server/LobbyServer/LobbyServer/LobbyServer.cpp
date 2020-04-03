#pragma comment(lib, "ws2_32")
#include "LobbyServer.h"
#include <iostream>
#include "lobby_protocol.h"
#include "..//..//BattleServer/BattleServer/battle_protocol.h"

namespace BattleArena {
	void LOBBYSERVER::error_display(const char* msg, int err_no)
	{
		WCHAR* lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err_no,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf, 0, NULL);
		std::cout << msg;
		std::wcout << L"���� " << lpMsgBuf << std::endl;
		while (true);
		LocalFree(lpMsgBuf);
	}

	LOBBYSERVER::LOBBYSERVER() :
		m_listenSocket(INVALID_SOCKET),
		m_battleSocket(INVALID_SOCKET)
	{
		wprintf(L"[LOBBY SERVER]\n");
		InitWSA();
		InitThreads();
	}
	LOBBYSERVER::~LOBBYSERVER()
	{
		for (UINT i = 0; i < m_threads.size(); ++i)
			m_threads[i].join();
		closesocket(m_listenSocket);
		closesocket(m_battleSocket);
		WSACleanup();
	}

	void LOBBYSERVER::InitWSA()
	{
		WSADATA wsa;
		if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsa))
			error_display("Error at WSAStartup()", WSAGetLastError());

		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, 0);

		m_listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		m_battleSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		CSOCKADDR_IN lobbyserverAddr{ INADDR_ANY, LOBBYSERVER_PORT };
		CSOCKADDR_IN battleserverAddr{ "127.0.0.1", BATTLESERVER_PORT };

		wprintf(L"Initializing ListenSocket...");
		if (SOCKET_ERROR == ::bind(m_listenSocket, lobbyserverAddr.getSockAddr(), *lobbyserverAddr.len()))
			error_display("Error at Bind()", WSAGetLastError());
		if (SOCKET_ERROR == ::listen(m_listenSocket, SOMAXCONN))
			error_display("Error at Listen()", WSAGetLastError());
		wprintf(L" Done.\n");

		wprintf(L"Connecting to BattleServer...");
		while (SOCKET_ERROR == ::connect(m_battleSocket, battleserverAddr.getSockAddr(), *battleserverAddr.len()))
			wprintf(L"\n Can't access to BattleServer... Retry...");
		m_clients[BATTLE_KEY].socket = m_battleSocket;
		m_clients[BATTLE_KEY].state = ST_IDLE;
		m_clients[BATTLE_KEY].recv_over.init();
		/*m_clients[BATTLE_KEY].recv_over.set_event(EV_BATTLE);*/
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_battleSocket), m_iocp, BATTLE_KEY, 0);
		DWORD flag = 0;
		if (SOCKET_ERROR == WSARecv(m_clients[BATTLE_KEY].socket, m_clients[BATTLE_KEY].recv_over.buffer(), 1, nullptr, &flag, m_clients[BATTLE_KEY].recv_over.overlapped(), nullptr))
		{
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Error at WSARecv()", err_no);
		}

		wprintf(L" Done.\n");
	}
	void LOBBYSERVER::InitThreads()
	{
		wprintf(L"Initializing Threads...");
		for (UINT i = 0; i < 4; ++i)
			m_threads.emplace_back(&LOBBYSERVER::do_worker, this);
		wprintf(L" Done.\n");
	}
	void LOBBYSERVER::Run()
	{
		CSOCKADDR_IN clientAddr{};
		SOCKET clientSocket{};
		while (true)
		{
			clientSocket = accept(m_listenSocket, clientAddr.getSockAddr(), clientAddr.len());
			
			int new_id = player_num++;
			m_clients[new_id].socket = clientSocket;
			m_clients[new_id].state = ST_IDLE;
			m_clients[new_id].recv_over.init();
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_clients[new_id].socket), m_iocp, new_id, 0);
			std::wcout << L"[CLIENT - " << new_id << L"] Accept" << std::endl;

			DWORD flag = 0;
			if (0 != WSARecv(m_clients[new_id].socket, m_clients[new_id].recv_over.buffer(), 1, NULL, &flag, m_clients[new_id].recv_over.overlapped(), NULL))
			{
				int err_no = WSAGetLastError();
				if (WSA_IO_PENDING != err_no)
					error_display("Error at WSARecv()", err_no);
			}
		}
	}
	void LOBBYSERVER::do_worker()
	{
		DWORD ReceivedBytes;
		ULONGLONG key;
		OVERLAPPED* over;

		while (true)
		{
			GetQueuedCompletionStatus(m_iocp, &ReceivedBytes, &key, &over, INFINITE);
			if (0 == ReceivedBytes) {
				std::wcout << L"[CLIENT - " << key << L"] Disconnected" << std::endl;
				closesocket(m_clients[key].socket);
				//DisconnectPlayer(key);
				continue;
			}

			OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
			DWORD client = static_cast<DWORD>(key);
			switch (over_ex->event_type())
			{
			case EV_RECV: {
				ProcessPacket(client, over_ex->data());
				DWORD flag = 0;
				if (SOCKET_ERROR == WSARecv(m_clients[client].socket, m_clients[client].recv_over.buffer(), 1, nullptr, &flag, m_clients[client].recv_over.overlapped(), nullptr))
				{
					int err_no = WSAGetLastError();
					if (WSA_IO_PENDING != err_no)
						error_display("Error at WSARecv()", err_no);
				}
			}
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

	void LOBBYSERVER::send_packet(int client, void* buff)
	{
		OVER_EX* send_over = new OVER_EX{EV_SEND, buff};
		WSASend(m_clients[client].socket, send_over->buffer(), 1, 0, 0, send_over->overlapped(), 0);
	}
	void LOBBYSERVER::send_packet_default(int client, int TYPE)
	{
		common_default_packet cdp;
		cdp.size = sizeof(cdp);
		cdp.type = TYPE;
		send_packet(client, &cdp);
	}
	void LOBBYSERVER::send_packet_room_info(int client, int room_id)
	{
		sc_packet_match_room_info mri;
		mri.cdp.size = sizeof(mri);
		mri.cdp.type = SC_PACKET_MATCH_ROOM_INFO;
		mri.room_id = room_id;
		send_packet(client, &mri);
	}
	void LOBBYSERVER::send_packet_request_room()
	{
		sb_packet_request_room packet;
		packet.cdp.size = sizeof(packet);
		packet.cdp.type = SB_PACKET_REQUEST_ROOM;
		send_packet(BATTLE_KEY, &packet);
	}


	void LOBBYSERVER::ProcessPacket(DWORD client, void* buffer)
	{
		common_default_packet* packet = reinterpret_cast<common_default_packet*>(buffer);
		switch (packet->type)
		{
		case CS_PACKET_MATCH_ENQUEUE: {
			if (ST_QUEUE == m_clients[client].state) break;
			queueLock.lock();
			m_queueList.emplace_back(client);
			m_queueMap.emplace(std::make_pair(client, std::prev(m_queueList.end())));
			m_clients[client].state = ST_QUEUE;
			std::list<int> cpyqueue{ m_queueList };
			queueLock.unlock();
			send_packet_default(client, SC_PACKET_MATCH_ENQUEUE);

			wprintf(L"[CLIENT %d] - ENQUEUE\n", client);
			wprintf(L"[QUEUE STATUS - ");
			for (auto i = cpyqueue.begin(); i != cpyqueue.end(); ++i)
				wprintf(L"%d ", *i);
			wprintf(L"]\n");

			/*If Players enough to make match,
			Make Room_Waiter and 
			Request Battle server to Empty room*/
			queueLock.lock();
			if (m_queueList.size() >= MATCHUP_NUM)
			{
				std::list<int>::iterator waiter = m_queueList.begin();
				ROOM_WAITER room_waiter;
				for (int i = 0; i < MATCHUP_NUM; ++i)
				{
					room_waiter.waiter[i] = *waiter;
					m_clients[*waiter].state = ST_PLAY;
					m_queueMap.erase(*waiter);
					m_queueList.erase(waiter++);
				}
				queueLock.unlock();

				waiterLock.lock();
				m_waiters.emplace_back(room_waiter);
				waiterLock.unlock();
				send_packet_request_room();
				wprintf(L"[MATCH MAKE]\n");
			}
			else queueLock.unlock();
		}
			break;

		case CS_PACKET_MATCH_DEQUEUE: {
			if (ST_IDLE == m_clients[client].state) break;
			queueLock.lock();
			m_queueList.erase(m_queueMap[client]);
			m_queueMap.erase(client);
			m_clients[client].state = ST_IDLE;
			std::list<int> cpyqueue{ m_queueList };
			queueLock.unlock();
			send_packet_default(client, SC_PACKET_MATCH_DEQUEUE);

			wprintf(L"[CLIENT %d] - DEQUEUE\n", client);
			wprintf(L"[QUEUE STATUS - ");
			for (auto i = cpyqueue.begin(); i != cpyqueue.end(); ++i)
				wprintf(L"%d ", *i);
			wprintf(L"]\n");
		}
			break;

		case BS_PACKET_RESPONSE_ROOM: {
			//ProcessRequestRoomPacket();
			bs_packet_response_room* packet = reinterpret_cast<bs_packet_response_room*>(buffer);
			waiterLock.lock();
			std::list<ROOM_WAITER>::iterator w = m_waiters.begin();
			for (int i = 0; i < MATCHUP_NUM; ++i)
			{
				m_clients[w->waiter[i]].state = ST_IDLE;
				send_packet_room_info(w->waiter[i], packet->room_id);
			}
			m_waiters.erase(w);
			waiterLock.unlock();
		}
			break;

		default:
			printf("[CLIENT %d] - Unknown Packet\n", client);
			while (true);
			break;
		}
	}

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
	OVER_EX::OVER_EX() {}
	OVER_EX::OVER_EX(EVENT_TYPE ev) :
		ev_type(ev)
	{
		init();
	}
	OVER_EX::OVER_EX(EVENT_TYPE ev, void* buff) :
		OVER_EX(ev)
	{
		common_default_packet* cdp = reinterpret_cast<common_default_packet*>(buff);
		memcpy(packet, cdp, cdp->size);
		wsabuf.len = cdp->size;
	}
	void OVER_EX::init()
	{
		memset(&over, 0, sizeof(WSAOVERLAPPED));
		wsabuf.buf = packet;
		wsabuf.len = sizeof(packet);
	}
	void OVER_EX::reset()
	{
		memset(&over, 0, sizeof(WSAOVERLAPPED));
	}
}