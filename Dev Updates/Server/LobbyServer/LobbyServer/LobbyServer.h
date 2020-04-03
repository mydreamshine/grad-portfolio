#pragma once
#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <map>

namespace BattleArena {
	constexpr auto MAX_BUFFER_SIZE = 200;
	constexpr auto MAX_USER = 100;
	constexpr auto BATTLE_KEY = MAX_USER;
	constexpr auto MATCHUP_NUM = 3;

	//SOCKADDR_IN Wrapping Class
	enum EVENT_TYPE {EV_RECV, EV_SEND, EV_BATTLE};
	class CSOCKADDR_IN
	{
	private:
		int size;
		SOCKADDR_IN addr;

	public:
		CSOCKADDR_IN();
		CSOCKADDR_IN(unsigned long addr, short port);
		CSOCKADDR_IN(const char* addr, short port);

		int* len() { return &size; };
		SOCKADDR* getSockAddr() { return reinterpret_cast<SOCKADDR*>(&addr); };
	};

	//Extend OVERLAPPED
	class OVER_EX
	{
	private:
		WSAOVERLAPPED over;
		WSABUF wsabuf;
		char packet[MAX_BUFFER_SIZE];
		EVENT_TYPE ev_type;

	public:
		OVER_EX();
		OVER_EX(EVENT_TYPE ev);
		OVER_EX(EVENT_TYPE ev, void* buff);
		WSABUF* buffer() { return &wsabuf; }
		char* data() { return packet; }
		WSAOVERLAPPED* overlapped() { return &over; }
		EVENT_TYPE event_type() { return ev_type; }
		void set_event(EVENT_TYPE ev) { ev_type = ev; }
		void reset();
		void init();
	};

	//REAL USER
	enum CL_STATE {ST_QUEUE, ST_IDLE, ST_PLAY};
	struct CLIENT
	{
		OVER_EX recv_over{};
		SOCKET socket{ INVALID_SOCKET };
		CL_STATE state{ ST_IDLE };
	};
	struct ROOM_WAITER
	{
		int waiter[MATCHUP_NUM];
	};

	class LOBBYSERVER
	{
	private:
		std::atomic<int> player_num;
		HANDLE m_iocp;
		SOCKET m_listenSocket;
		SOCKET m_battleSocket;
		CLIENT m_clients[MAX_USER + 1];
		
		//Match Queue
		std::mutex queueLock;
		std::list<int> m_queueList;
		std::map<int, std::list<int>::iterator> m_queueMap;
		
		std::mutex waiterLock;
		std::list<ROOM_WAITER> m_waiters;
		
		std::vector<std::thread> m_threads;

		void error_display(const char* msg, int err_no);
		void InitWSA();
		void InitThreads();

		void do_worker();
		void send_packet(int client, void* buff);
		void send_packet_default(int client, int TYPE);
		void send_packet_room_info(int client, int room_id);
		void send_packet_request_room();
		void ProcessPacket(DWORD client, void* packet);

	public:
		LOBBYSERVER();
		~LOBBYSERVER();
		void Run();
	};
}