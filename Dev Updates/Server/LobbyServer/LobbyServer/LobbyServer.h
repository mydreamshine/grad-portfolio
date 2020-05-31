#pragma once
#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include <set>
#include "lobby_protocol.h"
#include "DBMANAGER.h"

//Commons
#include "..\..\Common\CSOCKADDR_IN.h"
#include "..\..\Common\OVER_EX.h"

#define BATTLE_OFFLINE


namespace BattleArena {
	constexpr auto MAX_BUFFER_SIZE = 200;
	constexpr auto MAX_USER = 100;
	constexpr auto BATTLE_KEY = MAX_USER;
	constexpr auto MATCHUP_NUM = 3;

	enum EVENT_TYPE {EV_CLIENT, EV_SEND, EV_BATTLE};
	enum CL_STATE { ST_QUEUE, ST_IDLE, ST_PLAY };

	class CLIENT
	{
	public:
		OVER_EX recv_over{};
		SOCKET socket{ INVALID_SOCKET };
		CL_STATE state{ ST_IDLE };

		int uid;
		char id[ID_LENGTH];
		std::set<int> friendlist; //(ONLY index for client_table, show ONLY ONLINE friends)

		void set_recv()
		{
			recv_over.reset();
			DWORD flag = 0;
			if (SOCKET_ERROR == WSARecv(socket, recv_over.buffer(), 1, nullptr, &flag, recv_over.overlapped(), nullptr))
			{
				int err_no = WSAGetLastError();
				if (WSA_IO_PENDING != err_no)
					error_display("Error at WSARecv()", err_no);
			}
		}
		void error_display(const char* msg, int err_no)
		{
			WCHAR* lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, err_no,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, NULL);
			std::cout << msg;
			std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
			while (true);
			LocalFree(lpMsgBuf);
		}
	};
	struct ROOM_WAITER
	{
		int waiter[MATCHUP_NUM];
	};

	class LOBBYSERVER
	{
	private:
		std::vector<std::thread> m_threads;
		std::atomic<int> player_num;
		HANDLE m_iocp;
		SOCKET m_listenSocket;
		SOCKET m_battleSocket;
		
		CLIENT m_clients[MAX_USER + 1];
		std::mutex client_table_lock;
		std::map<int, int> client_table; //connected user table that consist of (user UID, m_clients's INDEX)

		//Match Queue
		std::mutex queueLock;
		std::list<int> m_queueList;
		std::map<int, std::list<int>::iterator> m_queueMap;
		
		std::mutex waiterLock;
		std::list<ROOM_WAITER> m_waiters;

		//Func
		void error_display(const char* msg, int err_no);
		void InitWSA();
		void InitThreads();

		void do_worker();

		void send_packet(int client, void* buff);
		void send_packet_default(int client, int TYPE);
		void send_packet_room_info(int client, int room_id);
		void send_packet_request_room(char mode);
		void send_packet_friend_status(int client, int who, int status);
		void process_client_packet(DWORD client, void* packet);
		void process_battle_packet(DWORD client, void* packet);
		void process_packet_response_room(void* buffer);
		void process_packet_login(int client, void* buffer);
		void process_packet_request_friend(int client, void* buffer);
		void process_packet_accept_friend(int client, void* buffer);

		void match_enqueue(DWORD client);
		void match_dequeue(DWORD client);
		void match_make();

		void disconnect_client(int client);
		void insert_client_table(int uid, int clinet);
		void delete_client_table(int uid);

		//is client connect?
		int isConnect(int uid); //uid ver
		int isConnect(const char* id); //char ver
	public:
		LOBBYSERVER();
		~LOBBYSERVER();
		void Run();
	};
}