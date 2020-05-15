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



template <class T>
class NWMODULE
{
private:
	T& MainModule;
	HANDLE iocp;
	SOCKET lobby_socket;
	SOCKET battle_socket;
	vector<thread> threads;
	vector<function<void(T&)>> callbacks;

	OVER_EX lobby_over;
	PACKET_BUFFER lobby_buffer;

private:
	void error_display(const char* msg, int err_no);
	void InitWSA();
	void RecvLobbyThread();

	void send_default_packet(int type);

	bool connect_server(SOCKET& socket, const char* address, const short port);
	void packet_drain(PACKET_BUFFER& packet_buffer, char* buffer, size_t len);
	void packet_parse();

	void process_disconnect(SOCKET& socket, PACKET_BUFFER& buffer);
	void process_lobby_packet(int packet_type, const void* buffer);

public:
	NWMODULE(T& MainModule, HANDLE iocp = INVALID_HANDLE_VALUE);
	~NWMODULE();

	//Basic Funcs
	void enroll_callback(int packet_type, function<void(T&)> callback);
	void notify_lobby_recv(size_t length);
	void notify_battle_recv();
	void update();

	//Connect Server Funcs
	bool connect_lobby(int key);
	void disconnect_lobby();
	bool connect_battle();
	void disconnect_battle();

	//Send Packet;
	void request_login();
	void add_friend(const char* id);
	void accept_friend(const char* id);
	void match_enqueue();
	void match_dequeue();
};

