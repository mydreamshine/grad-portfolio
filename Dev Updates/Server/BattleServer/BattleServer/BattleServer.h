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
	constexpr auto MAX_ROOM = 100; ///< Max room count of server.
	constexpr auto UPDATE_INTERVAL = 30; ///< Interval of updateing time. Millisecond.
	constexpr auto RECV_BUF_SIZE = 256;

	void error_display(const char* msg, int err_no);

	enum EVENT_TYPE;
	enum CL_STATE;


	/**
	@brief Battle server.
	@author Gurnwoo Kim
	*/
	class BATTLESERVER
	{
	private:
		std::atomic<int> player_num;	///< current number of user.
		HANDLE m_iocp;					///< handle for iocp.
		SOCKET m_listenSocket;			///< socket for listining.
		CLIENT m_Lobby;					///< client struct for lobby.

		ROOM *m_Rooms[MAX_ROOM];		///< rooms.
		std::mutex roomListLock;		///< lock for rooms.
		std::list<DWORD> roomList;		///< idx for availiable rooms.

		//Å¸ÀÌ¸Ó
		std::priority_queue<EVENT> timer_queue; ///< queue for events.
		std::mutex timer_lock;					///< lock for timer_queue.
		std::vector<std::thread> m_threads;		///< threads for server.

		/**
		@brief Init WSA environment.
		*/
		void InitWSA();

		/**
		@brief Init timer, iocp threads.
		*/
		void InitThreads();
		/**
		@brief Init rooms.
		*/
		void InitRooms();

		/**
		@brief Add new event to queue.
		@param ev event will insert to queue.
		*/
		void add_timer(EVENT& ev);

		/**
		@brief Add new event to queue.
		@param client target client.
		@param et event type.
		@param delay_time after this time event will execute.
		*/
		void add_event(int client, EVENT_TYPE et, int delay_time);

		/**
		@brief Function for timer threads.
		*/
		void do_timer();
		/**
		@brief Function for iocp threads.
		*/
		void do_worker();
		
		/**
		@brief Process player disconnect.
		*/
		void disconnect_player(CLIENT* client);

		/**
		@brief Send packet to client.
		@param client target client.
		@param buff data.
		*/
		void send_packet(CLIENT* client, void* buff);

		/**
		@brief Send default type packet to client.
		@param client target client.
		@param TYPE packet type.
		*/
		void send_packet_default(CLIENT* client, int TYPE);

		/**
		@brief Send available room id to lobby.
		@param room_id available room id.
		*/
		void send_packet_response_room(int room_id);

		/**
		@brief Process packet received from clients.
		@param client sender.
		@param buffer packet data.
		*/
		void ProcessPacket(CLIENT* client, void* buffer);

		/**
		@brief Process packet received from lobby.
		@param buffer packet data.
		*/
		void ProcessLobbyPacket(void* buffer);

		/**
		@brief Process authorize packet received from clients.
		@param client sender.
		@param buffer packet data.
		*/
		void ProcessAuthoPacket(CLIENT* client, void* buffer);

		/**
		@brief Get empty room index from roomList.
		@return empty rooms index.
		*/
		int get_empty_room();

		/**
		@brief Set empty room index to roomList.
		@param num rooms index.		
		*/
		void set_empty_room(int num);

		/**
		@brief make room and return it.
		@param mode room type.
		@return allocated room pointer.
		*/
		ROOM* make_game_mode(int mode);
	public:
		BATTLESERVER();
		~BATTLESERVER();

		/**
		@brief server start.
		*/
		void Run();
	};
}