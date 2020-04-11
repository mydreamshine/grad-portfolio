#pragma once
#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <map>
#include <chrono>
#include <queue>

#include "csockaddr_in.h"
#include "EVENT.h"
#include "OVER_EX.h"
#include "CLIENT.h"
#include "ROOM.h"

namespace BattleArena {
	constexpr auto MAX_ROOM = 100;
	//constexpr auto MATCHUP_NUM = 2;
	constexpr auto UPDATE_INTERVAL = 30; //ms

	void error_display(const char* msg, int err_no);

	//SOCKADDR_IN Wrapping Class
	enum EVENT_TYPE;
	enum CL_STATE;
	//REAL USER
	
	class BATTLESERVER
	{
	private:
		std::atomic<int> player_num;
		HANDLE m_iocp;
		SOCKET m_listenSocket;
		CLIENT m_Lobby;

		ROOM *m_Rooms[MAX_ROOM];
		std::mutex roomListLock;
		std::list<DWORD> roomList;

		//≈∏¿Ã∏”
		std::priority_queue<EVENT<EVENT_TYPE>> timer_queue;
		std::mutex timer_lock;
		std::vector<std::thread> m_threads;

		void InitWSA();
		void InitThreads();
		void InitRooms();

		void add_timer(EVENT<EVENT_TYPE>& ev);
		void add_event(int client, EVENT_TYPE et, int delay_time);
		void do_timer();
		void do_worker();
		
		void send_packet(CLIENT* client, void* buff);
		void send_packet_default(CLIENT* client, int TYPE);
		void send_packet_response_room(int room_id);
		void ProcessPacket(CLIENT* client, void* buffer);
		void ProcessLobbyPacket(void* buffer);
		void ProcessAuthoPacket(CLIENT* client, void* buffer);

		int get_empty_room();
		void set_empty_room(int num);
		ROOM* make_game_mode(int mode);
	public:
		BATTLESERVER();
		~BATTLESERVER();
		void Run();
	};
}