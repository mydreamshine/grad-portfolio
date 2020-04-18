#pragma comment(lib, "ws2_32")
#include "LobbyServer.h"
#include "..//..//BattleServer/BattleServer/battle_protocol.h"



namespace BattleArena {
	//DB
	thread_local DBMANAGER DBManager{};

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
		std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
		while (true);
		LocalFree(lpMsgBuf);
	}

	LOBBYSERVER::LOBBYSERVER() :
		m_listenSocket(INVALID_SOCKET),
		m_battleSocket(INVALID_SOCKET)
	{
		std::wcout.imbue(std::locale("korean"));
		setlocale(LC_ALL, "korean");
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

#ifndef BATTLE_OFFLINE
		wprintf(L"Connecting to BattleServer...");
		while (SOCKET_ERROR == ::connect(m_battleSocket, battleserverAddr.getSockAddr(), *battleserverAddr.len()))
			wprintf(L"\n Can't access to BattleServer... Retry...");
		m_clients[BATTLE_KEY].socket = m_battleSocket;
		m_clients[BATTLE_KEY].state = ST_IDLE;
		m_clients[BATTLE_KEY].recv_over.init();
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_battleSocket), m_iocp, BATTLE_KEY, 0);
		m_clients[BATTLE_KEY].set_recv();
		wprintf(L" Done.\n");
#else
		wprintf(L"[BATTLE_OFFLINE MODE]\n");
#endif
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
			m_clients[new_id].set_recv();
		}
	}
	void LOBBYSERVER::do_worker()
	{
		DBManager.init();
		DWORD ReceivedBytes;
		ULONGLONG key;
		OVERLAPPED* over;
		while (true)
		{
			GetQueuedCompletionStatus(m_iocp, &ReceivedBytes, &key, &over, INFINITE);

			if (0 == ReceivedBytes) {
				disconnect_client(key);
				continue;
			}

			OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
			DWORD client = static_cast<DWORD>(key);
			switch (over_ex->event_type())
			{
			case EV_RECV:
				process_packet(client, over_ex->data());
				m_clients[client].set_recv();
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
	void LOBBYSERVER::send_packet_request_room(char mode)
	{
		sb_packet_request_room packet;
		packet.cdp.size = sizeof(packet);
		packet.cdp.type = SB_PACKET_REQUEST_ROOM;
		packet.mode = mode;
		send_packet(BATTLE_KEY, &packet);
	}
	void LOBBYSERVER::send_packet_friend_status(int client, int who, int status)
	{
		if (status == FRIEND_ONLINE)
			m_clients[client].friendlist.emplace(who);
		else if (status == FRIEND_OFFLINE)
			m_clients[client].friendlist.erase(who);

		sc_packet_friend_status packet;
		packet.cdp.type = SC_PACKET_FRIEND_STATUS;
		packet.cdp.size = sizeof(sc_packet_friend_status);
		packet.status = status;
		strcpy_s(packet.id, m_clients[who].id);
		send_packet(client, &packet);
	}
	

	void LOBBYSERVER::process_packet(DWORD client, void* buffer)
	{
		common_default_packet* packet = reinterpret_cast<common_default_packet*>(buffer);
		switch (packet->type)
		{
		case CS_PACKET_REQUEST_LOGIN: 
			process_packet_login(client, buffer);
			break;

		case CS_PACKET_REQUEST_FRIEND:
			process_packet_request_friend(client, buffer);
			break;

		case CS_PACKET_ACCEPT_FRIEND:
			process_packet_accept_friend(client, buffer);
			break;

		case CS_PACKET_MATCH_ENQUEUE:
			if (ST_QUEUE == m_clients[client].state) break;
			match_enqueue(client);
#ifndef BATTLE_OFFLINE
			match_make();
#endif
			break;

		case CS_PACKET_MATCH_DEQUEUE:
			if (ST_IDLE == m_clients[client].state) break;
			match_dequeue(client);
			break;

		case BS_PACKET_RESPONSE_ROOM:
			process_packet_response_room(buffer);
			break;

		default:
			printf("[CLIENT %d] - Unknown Packet\n", client);
			while (true);
			break;
		}
	}
	void LOBBYSERVER::process_packet_response_room(void* buffer)
	{
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
	void LOBBYSERVER::process_packet_login(int client, void* buffer)
	{
		cs_packet_request_login* packet = reinterpret_cast<cs_packet_request_login*>(buffer);
		int uid = DBManager.get_uid(packet->id);
		if (uid == RESULT_NO_ID) {
			send_packet_default(client, SC_PACKET_LOGIN_FAIL);
			return;
		}

		m_clients[client].uid = uid;
		strcpy_s(m_clients[client].id, packet->id);
		insert_client_table(uid, client);
		send_packet_default(client, SC_PACKET_LOGIN_OK);
		std::vector<std::string> friendlist = DBManager.get_friendlist(packet->id);
		for (int i = 0; i < friendlist.size(); ++i) {
			int friend_index = isConnect(friendlist[i].c_str());
			if (friend_index != -1) {
				send_packet_friend_status(friend_index, client, FRIEND_ONLINE);
				send_packet_friend_status(client, friend_index, FRIEND_ONLINE);
			}
		}
	}
	void LOBBYSERVER::process_packet_request_friend(int client, void* buffer)
	{
		cs_packet_request_friend* packet = reinterpret_cast<cs_packet_request_friend*>(buffer);
		int uid = DBManager.get_uid(packet->id);
		if (uid == RESULT_NO_ID)
			return;
		int receiver = isConnect(uid);
		strcpy_s(packet->id, m_clients[client].id);
		if (receiver != -1)
			send_packet(receiver, buffer);
	}
	void LOBBYSERVER::process_packet_accept_friend(int client, void* buffer)
	{
		cs_packet_accept_friend* packet = reinterpret_cast<cs_packet_accept_friend*>(buffer);
		DBManager.insert_friend(m_clients[client].id, packet->id);

		int friends = isConnect(packet->id);
		if (friends == -1) return;

		send_packet_friend_status(client, friends, FRIEND_ONLINE);
		send_packet_friend_status(friends, client, FRIEND_ONLINE);
	}
	
	void LOBBYSERVER::match_enqueue(DWORD client)
	{
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
	}
	void LOBBYSERVER::match_dequeue(DWORD client)
	{
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
	void LOBBYSERVER::match_make()
	{
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
			send_packet_request_room(GAMEMODE_NGP);
			wprintf(L"[MATCH MAKE]\n");
		}
		else queueLock.unlock();
	}
	void LOBBYSERVER::disconnect_client(int client)
	{
		std::wcout << L"[CLIENT - " << m_clients[client].id << L"] Disconnected" << std::endl;
		delete_client_table(m_clients[client].uid);
		auto& fl = m_clients[client].friendlist;
		for (auto i = fl.begin(); i != fl.end(); ++i) {
			send_packet_friend_status(*i, client, FRIEND_OFFLINE);
		}
		m_clients[client].uid = -1;
		memset(&m_clients[client].id, 0, sizeof(m_clients[client].id));
		closesocket(m_clients[client].socket);
		m_clients[client].friendlist.clear();
		m_clients[client].socket = INVALID_SOCKET;
	}
	void LOBBYSERVER::insert_client_table(int uid, int client)
	{
		client_table_lock.lock();
		client_table.emplace(std::make_pair(uid, client));
		client_table_lock.unlock();
	}
	void LOBBYSERVER::delete_client_table(int uid)
	{
		client_table_lock.lock();
		client_table.erase(uid);
		client_table_lock.unlock();
	}
	int LOBBYSERVER::isConnect(int uid)
	{
		client_table_lock.lock();
		std::map<int, int> cpy{ client_table };
		client_table_lock.unlock();
		if (cpy.count(uid) != 0)
			return cpy[uid];
		return -1;
	}
	int LOBBYSERVER::isConnect(const char* id)
	{
		int uid = DBManager.get_uid(id);
		if (uid == RESULT_NO_ID)
			return -1;
		return isConnect(uid);
	}
}