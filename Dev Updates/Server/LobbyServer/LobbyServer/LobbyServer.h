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
#include "..\..\..\Streaming\Streaming_Server\Streaming_Server\packet_struct.h"
#include "DBMANAGER.h"

//Commons
#include "..\..\Common\CSOCKADDR_IN.h"
#include "..\..\Common\OVER_EX.h"

#define BATTLE_OFFLINE
#define FRIEND_OFFLINE

namespace BattleArena {
	constexpr auto RECV_BUFFER_SIZE = 512; /// max buffer size for socket communication

	enum EVENT_TYPE {EV_CLIENT, EV_SEND, EV_BATTLE}; /// iocp events, EV_CLIENT : recv from clients, EV_SEND : send is done, EV_BATTLE : recv from battle server
	enum CL_STATE { ST_QUEUE, ST_IDLE, ST_PLAY }; /// client status, ST_QUEUE : client get queued, ST_IDLE : client do nothing, ST_PLAY : playing game.

/**
@brief Class for Client
@details this class have client information. socket, state, uid, id, friends
@author Gurnwoo Kim
*/
	class CLIENT
	{
	public:
		OVER_EX recv_over{}; ///< struct for async communication.
		SOCKET socket{ INVALID_SOCKET }; ///< socket for communication.
		CL_STATE state{ ST_IDLE }; ///< client status.

		int uid{-1}; ///< clients uid
		wchar_t id[string_len]{0}; ///< clients id
		std::set<int> friendlist; ///< clients friend list, ONLY index for client_table, show ONLY ONLINE friends

		/**
		@brief set client socket to recv
		@details init overlapped struct with 0, and start async recv.
		*/
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
		
		/**
		@brief Function for error_display while socket communication.
		@details will display error with WSAGetLastError() and process will stop that position.
		*/
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

/**
@brief Class for client who Wait matchs(after matchmaking, before match)
@details group for match-maked client. after match making, there's a short term, because battle server and lobby server communicate for initiating new match. 
@author Gurnwoo Kim
*/
	struct ROOM_WAITER
	{
		int *waiter; ///< array for clients index.

		ROOM_WAITER(int nums) {
			waiter = new int[nums];
		}
		~ROOM_WAITER() {
			delete[] waiter;
		}
	};

/**
@brief Lobby Server Class
@details Process communication func - login, friends, match making...
@author Gurnwoo Kim
*/
	class LOBBYSERVER
	{
	public:
		LOBBYSERVER();
		~LOBBYSERVER();

		/**
		@brief Run Server. Server start accepting clients.
		*/
		void Run();

	private:
		int MAX_USER = 100; /// max user count that lobby server can hold
		int BATTLE_KEY = MAX_USER; /// last index for battle server
		int MATCHUP_NUM = 1; /// will deprecated. at least count for match-making.

		std::wstring config_path{ L".\\LOBBY_CONFIG.ini" };
		short LOBBY_PORT{0};
		int NUM_THREADS{ 0 };
		short BATTLE_PORT{0};
		std::wstring BATTLE_ADDR{};
		
		std::vector<std::thread> m_threads; ///< threads for server.
		std::atomic<int> player_num; ///< current player nums.
		HANDLE m_iocp; ///< handle for iocp.
		SOCKET m_listenSocket; ///< socket for listening client.
		SOCKET m_battleSocket; ///< socket for connecting battle server.
		
		std::list<int> ID_LIST;
		std::mutex ID_LOCK;
		CLIENT* m_clients; ///< array for clients.
		std::mutex client_table_lock; ///< lock for m_clients.
		std::map<int, int> client_table; ///<connected user table that consist of (user UID, m_clients's INDEX).

		//Match Queue
		std::mutex queueLock; ///< lock for match-making queue.
		std::list<int> m_queueList; ///< array for clients who waiting match-making.
		std::map<int, std::list<int>::iterator> m_queueMap; ///< map for fast access to m_queueList.
		
		std::mutex waiterLock; ///< lock for m_waiters.
		std::list<ROOM_WAITER> m_waiters; ///< array for match-waiters

		//Func
		void error_display(const char* msg, int err_no); ///< error display function for socket communication.
		void InitWSA(); ///< Initializing WSA environment.
		void InitThreads(); ///< Initializing threads.
		void InitClients(); ///< Initializing Clients.
		void InitConfig();
		void gen_default_config();

		void do_worker(); ///< Function for iocp thread.

		void send_packet(int client, void* buff); ///< send packet to client.
		void send_packet_default(int client, int type); ///< send default type packet to client.
		void send_packet_room_info(int client, int room_id); ///< notify client to connect battle server with room_id
		void send_packet_request_room(char mode); ///< request room to battle server.
#ifndef FRIEND_OFFLINE
		void send_packet_friend_status(int client, int who, int status); ///< notify client that friends status is changed.

		void process_packet_request_friend(int client, void* buffer); ///< process add friend request.
		void process_packet_accept_friend(int client, void* buffer); ///< process answer for add friend request.
#endif

		void process_client_packet(DWORD client, void* packet); ///< process clients packet.
		void process_battle_packet(DWORD client, void* packet); ///< process battle server packet.
		void process_packet_response_room(void* buffer); ///< process response room packet.
		void process_packet_login(int client, void* buffer); ///< process clients login.

		void match_enqueue(DWORD client); ///< enqueue client to match pool.
		void match_dequeue(DWORD client); ///< dequeue client to match pool.
		void match_make(); ///< match-making function.

		void disconnect_client(int client); ///< process clients disconnect.
		void insert_client_table(int uid, int clinet); ///< insert client to client_table
		void delete_client_table(int uid); ///<  delete client from client_table

		//is client connect?
		int isConnect(int uid); ///< return clients index if uid is connected.
		int isConnect(const wchar_t* id); ///< return clients index if id is connected.
	};
}