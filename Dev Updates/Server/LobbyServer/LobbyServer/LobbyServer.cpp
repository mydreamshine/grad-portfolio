#pragma comment(lib, "ws2_32")
#include "LobbyServer.h"



namespace BattleArena {
	//DB
	thread_local DBMANAGER DBManager{}; ///< class for query to DB for each threads.

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
		InitConfig();
		InitClients();
		InitWSA();
		InitThreads();
		wprintf(L"[SERVER ON]\n");
	}
	LOBBYSERVER::~LOBBYSERVER()
	{
		for (UINT i = 0; i < m_threads.size(); ++i)
			m_threads[i].join();
		closesocket(m_listenSocket);
		closesocket(m_battleSocket);
		WSACleanup();
		delete[] m_clients;
	}

	/**
	@brief Initializing WSA environment.
	@details init wsa, iocp handle, socket, bind, listen.
	*/
	void LOBBYSERVER::InitWSA()
	{
		WSADATA wsa;
		if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsa))
			error_display("Error at WSAStartup()", WSAGetLastError());

		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, 0);

		m_listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		m_battleSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		CSOCKADDR_IN lobbyserverAddr{ INADDR_ANY, LOBBY_PORT };

		wprintf(L"Initializing ListenSocket...");
		if (SOCKET_ERROR == ::bind(m_listenSocket, lobbyserverAddr.getSockAddr(), *lobbyserverAddr.len()))
			error_display("Error at Bind()", WSAGetLastError());
		if (SOCKET_ERROR == ::listen(m_listenSocket, SOMAXCONN))
			error_display("Error at Listen()", WSAGetLastError());
		wprintf(L" Done.\n");

#ifndef BATTLE_OFFLINE
		wprintf(L"Connecting to BattleServer...");
		CSOCKADDR_IN battleserverAddr{ BATTLE_ADDR.c_str(), BATTLE_PORT };
		while (SOCKET_ERROR == ::connect(m_battleSocket, battleserverAddr.getSockAddr(), *battleserverAddr.len()))
			wprintf(L"\n Can't access to BattleServer... Retry...");
		m_clients[BATTLE_KEY].socket = m_battleSocket;
		m_clients[BATTLE_KEY].state = ST_IDLE;
		m_clients[BATTLE_KEY].recv_over.init(RECV_BUFFER_SIZE);
		m_clients[BATTLE_KEY].recv_over.set_event(EV_BATTLE);
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_battleSocket), m_iocp, BATTLE_KEY, 0);
		m_clients[BATTLE_KEY].set_recv();
		wprintf(L" Done.\n");
#else
		wprintf(L"[BATTLE_OFFLINE MODE]\n");
#endif
	}
	/**
	@brief Initializing Threads.
	@details init threads for iocp.
	*/
	void LOBBYSERVER::InitThreads()
	{
		wprintf(L"Initializing Threads...");
		for (UINT i = 0; i < NUM_THREADS; ++i)
			m_threads.emplace_back(&LOBBYSERVER::do_worker, this);
		wprintf(L" Done.\n");
	}
	void LOBBYSERVER::InitClients()
	{
		wprintf(L"Initializing Clients...");
		ID_LOCK.lock();
		for (int i = 0; i < MAX_USER; ++i)
			ID_LIST.emplace_back(i);
		ID_LOCK.unlock();

		m_clients = new CLIENT[MAX_USER + 1];
		wprintf(L" Done.\n");
	}
	void LOBBYSERVER::InitConfig()
	{
		wprintf(L"Initializing Configs...");
		std::ifstream ini{ config_path.c_str() };
		if (false == ini.is_open())
			gen_default_config();
		ini.close();

		wchar_t buffer[512];
		GetPrivateProfileString(L"SERVER", L"LOBBY_PORT", L"15500", buffer, 512, config_path.c_str());
		LOBBY_PORT = std::stoi(buffer);

		GetPrivateProfileString(L"SERVER", L"NUM_THREADS", L"4", buffer, 512, config_path.c_str());
		NUM_THREADS = std::stoi(buffer);

		GetPrivateProfileString(L"SERVER", L"BATTLE_PORT", L"15600", buffer, 512, config_path.c_str());
		BATTLE_PORT = std::stoi(buffer);

		GetPrivateProfileString(L"SERVER", L"BATTLE_ADDR", L"127.0.0.1", buffer, 512, config_path.c_str());
		BATTLE_ADDR = std::wstring(buffer);

		GetPrivateProfileString(L"SERVER", L"MAX_USER", L"100", buffer, 512, config_path.c_str());
		MAX_USER = std::stoi(buffer);
		BATTLE_KEY = MAX_USER;

		GetPrivateProfileString(L"SERVER", L"MATCHMAKING_NUM", L"4", buffer, 512, config_path.c_str());
		MATCHUP_NUM = MAX_USER = std::stoi(buffer);
		wprintf(L" Done.\n");
	}
	void LOBBYSERVER::gen_default_config()
	{
		WritePrivateProfileString(L"SERVER", L"LOBBY_PORT", L"15500", config_path.c_str());
		WritePrivateProfileString(L"SERVER", L"NUM_THREADS", L"4", config_path.c_str());
		WritePrivateProfileString(L"SERVER", L"BATTLE_PORT", L"15600", config_path.c_str());
		WritePrivateProfileString(L"SERVER", L"BATTLE_ADDR", L"127.0.0.1", config_path.c_str());
		WritePrivateProfileString(L"SERVER", L"MAX_USER", L"100", config_path.c_str());
		WritePrivateProfileString(L"SERVER", L"MATCHMAKING_NUM", L"4", config_path.c_str());
	}
	/**
	@brief Run server.
	@details server start accepting clients.
	*/
	void LOBBYSERVER::Run()
	{
		CSOCKADDR_IN clientAddr{};
		SOCKET clientSocket{};
		while (true)
		{
			clientSocket = accept(m_listenSocket, clientAddr.getSockAddr(), clientAddr.len());
			
			int new_id = -1;
			ID_LOCK.lock();
			if (false == ID_LIST.empty()) {
				new_id = ID_LIST.front();  ID_LIST.pop_front();
			}
			ID_LOCK.unlock();
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), m_iocp, new_id, 0);

			if (new_id == -1) {
				csss_packet_login_fail packet{};
				OVER_EX* send_over = new OVER_EX{ EV_SEND, &packet };
				WSASend(clientSocket, send_over->buffer(), 1, 0, 0, send_over->overlapped(), 0);
				closesocket(clientSocket);
				continue;
			}

			m_clients[new_id].socket = clientSocket;
			m_clients[new_id].state = ST_IDLE;
			m_clients[new_id].recv_over.init(RECV_BUFFER_SIZE);
			m_clients[new_id].recv_over.set_event(EV_CLIENT);
			std::wcout << L"[CLIENT - " << new_id << L"] Accept" << std::endl;
			m_clients[new_id].set_recv();
		}
	}
	/**
	@brief function for iocp threads.
	@details process iocp events - recv, send, packet processing.
	*/
	void LOBBYSERVER::do_worker()
	{
		DBManager.init();
		DWORD ReceivedBytes;
		ULONGLONG key;
		OVERLAPPED* over;
		while (true)
		{
			GetQueuedCompletionStatus(m_iocp, &ReceivedBytes, &key, &over, INFINITE);



			OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
			DWORD client = static_cast<DWORD>(key);
			switch (over_ex->event_type())
			{
			case EV_CLIENT:
				if (0 == ReceivedBytes) {
					disconnect_client(key);
					continue;
				}
				else {
					process_client_packet(client, over_ex->data());
					m_clients[client].set_recv();
				}
				break;

			case EV_BATTLE:

				process_battle_packet(client, over_ex->data());
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

	/**
	@brief send packet to client.
	@param client index for client.
	@param buff void* pointing packet.
	*/
	void LOBBYSERVER::send_packet(int client, void* buff)
	{
		OVER_EX* send_over = new OVER_EX{EV_SEND, buff};
		WSASend(m_clients[client].socket, send_over->buffer(), 1, 0, 0, send_over->overlapped(), 0);
	}
	/**
	@brief send default type packet to client.
	@param client index for client.
	@param type packet type.
	*/
	void LOBBYSERVER::send_packet_default(int client, int type)
	{
		packet_inheritance cdp;
		cdp.size = sizeof(cdp);
		cdp.type = type;
		send_packet(client, &cdp);
	}
	/**
@brief notify client to connect battle server with room_id
@param client index for client.
@param room_id key for connecting battle server.
*/
	void LOBBYSERVER::send_packet_room_info(int client, int room_id)
	{
		csss_packet_access_match packet{ room_id };
		send_packet(client, &packet);
	}
	/**
@brief request room to battle server.
@param mode requesting room type.
*/
	void LOBBYSERVER::send_packet_request_room(char mode)
	{
		sb_packet_request_room packet{mode};
		send_packet(BATTLE_KEY, &packet);
	}

#ifndef FRIEND_OFFLINE
	/**
@brief notify client that friends status is changed.
@param client index for client.
@param who index for client who changed status.
@param status change to this status.
*/
	void LOBBYSERVER::send_packet_friend_status(int client, int who, int status)
	{
		//if (status == FRIEND_ONLINE)
		//	m_clients[client].friendlist.emplace(who);
		//else if (status == FRIEND_OFFLINE)
		//	m_clients[client].friendlist.erase(who);

		//sc_packet_friend_status packet;
		//packet.cdp.type = SC_PACKET_FRIEND_STATUS;
		//packet.cdp.size = sizeof(sc_packet_friend_status);
		//packet.status = status;
		//strcpy_s(packet.id, m_clients[who].id);
		//send_packet(client, &packet);
	}
#endif

	/**
@brief process clients packet.
@param client index for client.
@param buffer buffer that need to processing.
*/
	void LOBBYSERVER::process_client_packet(DWORD client, void* buffer)
	{
		packet_inheritance* packet = reinterpret_cast<packet_inheritance*>(buffer);
		switch (packet->type)
		{
		case SSCS_TRY_LOGIN: 
			process_packet_login(client, buffer);
			break;

#ifndef FRIEND_OFFLINE
		case CS_PACKET_REQUEST_FRIEND:
			process_packet_request_friend(client, buffer);
			break;

		case CS_PACKET_ACCEPT_FRIEND:
			process_packet_accept_friend(client, buffer);
			break;
#endif

		case SSCS_REQUEST_USER_INFO: {
			int rank = DBManager.get_rank(m_clients[client].uid);
			csss_packet_send_user_info user_info_packet{ m_clients[client].id, (int)wcslen(m_clients[client].id), rank };
			send_packet(client, &user_info_packet);
			break;
		}

		case SSCS_TRY_GAME_MATCHING: {
			if (ST_IDLE == m_clients[client].state) {
				match_enqueue(client);
				#ifndef BATTLE_OFFLINE
				match_make();
				#endif
			}
			else if (ST_QUEUE == m_clients[client].state)
				match_dequeue(client);
			break;
		}

		case SSCS_SEND_IN_LOBY_CHAT: {
			sscs_packet_send_chat_message* chat = reinterpret_cast<sscs_packet_send_chat_message*>(buffer);
			csss_packet_send_chat_message retmsg{ 1, chat->message, (int)wcslen(chat->message) };
			client_table_lock.lock();
			for (auto& cl : client_table)
				send_packet(cl.second, &retmsg);
			client_table_lock.unlock();
			break;
		}

		default:
			printf("[CLIENT %d] - Unknown Packet : %d\n", client, packet->type);
			while (true);
			break;
		}
	}
/**
@brief process battle server packet.
@param client index for client. - will deprecated.
@param buffer buffer that need to processing.
*/
	void LOBBYSERVER::process_battle_packet(DWORD client, void* buffer)
	{
		packet_inheritance* packet = reinterpret_cast<packet_inheritance*>(buffer);
		switch (packet->type)
		{
		case BS_PACKET_RESPONSE_ROOM:
			process_packet_response_room(buffer);
			break;

		default:
			printf("[Battle] - Unknown Packet\n");
			while (true);
			break;
		}
	}
/**
@brief process response room packet.
@param buffer buffer that need to processing.
*/
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
	/**
@brief process clients login.
@param client index for client.
@param buffer buffer that need to processing.
*/
	void LOBBYSERVER::process_packet_login(int client, void* buffer)
	{
		sscs_packet_try_login* packet = reinterpret_cast<sscs_packet_try_login*>(buffer);
		int uid = DBManager.get_uid(packet->id);

		if (uid == RESULT_NO_ID) {
			csss_packet_login_fail packet{};
			send_packet(client, &packet);
			return;
		}
		else if (isConnect(uid) != -1) {
			csss_packet_login_fail packet{};
			send_packet(client, &packet);
			return;
		}

		m_clients[client].uid = uid;
		wcscpy_s(m_clients[client].id, packet->id);
		insert_client_table(uid, client);
		csss_packet_login_ok login_ok_packet{ (short)uid };
		send_packet(client, &login_ok_packet);

		csss_packet_change_scene change_scene_packet{ 1 };
		send_packet(client, &change_scene_packet);

#ifndef FRIEND_OFFLINE
		std::vector<std::string> friendlist = DBManager.get_friendlist(packet->id);
		for (int i = 0; i < friendlist.size(); ++i) {
			int friend_index = isConnect(friendlist[i].c_str());
			if (friend_index != -1) {
				send_packet_friend_status(friend_index, client, FRIEND_ONLINE);
				send_packet_friend_status(client, friend_index, FRIEND_ONLINE);
			}
		}
#endif
	}

#ifndef FRIEND_OFFLINE
	/**
@brief process add friend request.
@param client index for client.
@param buffer buffer that need to processing.
*/
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
	/**
@brief process answer for add friend request.
@param client index for client.
@param buffer buffer that need to processing.
*/
	void LOBBYSERVER::process_packet_accept_friend(int client, void* buffer)
	{
		cs_packet_accept_friend* packet = reinterpret_cast<cs_packet_accept_friend*>(buffer);
		DBManager.insert_friend(m_clients[client].id, packet->id);

		int friends = isConnect(packet->id);
		if (friends == -1) return;

		send_packet_friend_status(client, friends, FRIEND_ONLINE);
		send_packet_friend_status(friends, client, FRIEND_ONLINE);
	}
#endif

	/**
@brief enqueue client to match pool.
@param client index for client.
*/
	void LOBBYSERVER::match_enqueue(DWORD client)
	{
		queueLock.lock();
		m_queueList.emplace_back(client);
		m_queueMap.emplace(std::make_pair(client, std::prev(m_queueList.end())));
		m_clients[client].state = ST_QUEUE;
		std::list<int> cpyqueue{ m_queueList };
		queueLock.unlock();

		csss_packet_match_enqueue packet{};
		send_packet(client, &packet);

		wprintf(L"[CLIENT %d] - ENQUEUE\n", client);
		wprintf(L"[QUEUE STATUS - ");
		for (auto i = cpyqueue.begin(); i != cpyqueue.end(); ++i)
			wprintf(L"%d ", *i);
		wprintf(L"]\n");
	}
	/**
@brief dequeue client to match pool.
@param client index for client.
*/
	void LOBBYSERVER::match_dequeue(DWORD client)
	{
		queueLock.lock();
		m_queueList.erase(m_queueMap[client]);
		m_queueMap.erase(client);
		m_clients[client].state = ST_IDLE;
		std::list<int> cpyqueue{ m_queueList };
		queueLock.unlock();

		csss_packet_match_dequeue packet{};
		send_packet(client, &packet);

		wprintf(L"[CLIENT %d] - DEQUEUE\n", client);
		wprintf(L"[QUEUE STATUS - ");
		for (auto i = cpyqueue.begin(); i != cpyqueue.end(); ++i)
			wprintf(L"%d ", *i);
		wprintf(L"]\n");
	}
	/**
@brief match-making function.
*/
	void LOBBYSERVER::match_make()
	{
		queueLock.lock();
		if (m_queueList.size() >= MATCHUP_NUM)
		{
			std::list<int>::iterator waiter = m_queueList.begin();
			ROOM_WAITER room_waiter{MATCHUP_NUM};
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
			send_packet_request_room(GAMEMODE_DM);
			wprintf(L"[MATCH MAKE]\n");
		}
		else queueLock.unlock();
	}
	/**
@brief process clients disconnect.
@param client index for client.
*/
	void LOBBYSERVER::disconnect_client(int client)
	{
		if (client < 0) return;

		std::wcout << L"[CLIENT - " << m_clients[client].id << L"] Disconnected" << std::endl;
		if (m_clients[client].state == ST_QUEUE)
			match_dequeue(client);
		delete_client_table(m_clients[client].uid);

		//auto& fl = m_clients[client].friendlist;
		//for (auto i = fl.begin(); i != fl.end(); ++i) {
		//	send_packet_friend_status(*i, client, FRIEND_OFFLINE);
		//}

		m_clients[client].uid = -1;
		memset(&m_clients[client].id, 0, sizeof(m_clients[client].id));
		closesocket(m_clients[client].socket);
		m_clients[client].friendlist.clear();
		--player_num;

		ID_LOCK.lock();
		ID_LIST.emplace_back(client);
		ID_LOCK.unlock();
	}
	/**
@brief insert client to client_table
@param uid uid for client.
@param client index for clinet.
*/
	void LOBBYSERVER::insert_client_table(int uid, int client)
	{
		client_table_lock.lock();
		client_table.emplace(std::make_pair(uid, client));
		client_table_lock.unlock();
	}
	/**
@brief delete client from client_table
@param uid uid for client.
*/
	void LOBBYSERVER::delete_client_table(int uid)
	{
		client_table_lock.lock();
		client_table.erase(uid);
		client_table_lock.unlock();
	}
	/**
@brief return clients index if uid is connected.
@param uid uid for client.
@return clients index. if client is not connected, return -1.
*/
	int LOBBYSERVER::isConnect(int uid)
	{
		client_table_lock.lock();
		std::map<int, int> cpy{ client_table };
		client_table_lock.unlock();
		if (cpy.count(uid) != 0)
			return cpy[uid];
		return -1;
	}
	/**
@brief return clients index if uid is connected.
@param id id for client.
@return clients index. if client is not connected, return -1.
*/
	int LOBBYSERVER::isConnect(const wchar_t* id)
	{
		int uid = DBManager.get_uid(id);
		if (uid == RESULT_NO_ID)
			return -1;
		return isConnect(uid);
	}
}