#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <thread>
#include <functional>

#include <unordered_map>
#include <vector>
#include <mutex>

#include "PACKET_BUFFER.h"
#include "OVER_EX.h"

using namespace std;


/**
*@brief class for communicate battle and lobby server.
*@author Gurnwoo Kim
*/
template <class T>
class NWMODULE
{
private:
	T& MainModule;
	HANDLE iocp;
	SOCKET lobby_socket;
	SOCKET battle_socket;
	vector<thread> threads;
	vector<function<void(T&)>> lobby_callbacks;
	vector<function<void(T&)>> battle_callbacks;

	OVER_EX lobby_over;
	PACKET_BUFFER lobby_buffer;
	OVER_EX battle_over;
	PACKET_BUFFER battle_buffer;

private:
	void error_display(const char* msg, int err_no);
	void InitWSA();
	void RecvLobbyThread();
	void RecvBattleThread();

	void send_default_packet(int type);

	bool connect_server(SOCKET& socket, const char* address, const short port);
	void packet_drain(PACKET_BUFFER& packet_buffer, char* buffer, size_t len);
	void packet_parse();

	void process_disconnect(SOCKET& socket, PACKET_BUFFER& buffer);
	void process_lobby_packet(int packet_type, const void* buffer);
	void process_battle_packet(int packet_type, const void* buffer);

public:
	NWMODULE(T& MainModule, HANDLE iocp = INVALID_HANDLE_VALUE);
	~NWMODULE();

	//Basic Funcs
	void enroll_lobby_callback(int packet_type, function<void(T&)> callback);
	void enroll_battle_callback(int packet_type, function<void(T&)> callback);
	void notify_lobby_recv(size_t length);
	void notify_battle_recv(size_t length);
	void update();

	//Connect Server Funcs
	bool connect_lobby(int iocp_key);
	void disconnect_lobby();
	bool connect_battle(int iocp_key);
	void disconnect_battle();

	//Send Packet To Server
	void request_login(const char* id);
	void add_friend(const char* id);
	void accept_friend(const char* id);
	void match_enqueue();
	void match_dequeue();
};

