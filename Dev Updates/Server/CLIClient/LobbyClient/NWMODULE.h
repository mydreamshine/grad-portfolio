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
@brief class for communicate battle and lobby server.
@author Gurnwoo Kim
*/
template <class T>
class NWMODULE
{
public:
	NWMODULE(T& MainModule, HANDLE iocp = INVALID_HANDLE_VALUE);
	~NWMODULE();

	/**
	@brief enroll lobby packets callback.
	@param packet_type target packet.
	@param callback when packet_type arrived, this callback will execute.
	*/
	void enroll_lobby_callback(int packet_type, function<void(T&)> callback);

	/**
	@brief enroll battle packets callback.
	@param packet_type target packet.
	@param callback when packet_type arrived, this callback will execute.
	*/
	void enroll_battle_callback(int packet_type, function<void(T&)> callback);

	/**
	@brief notify recv from lobby. - use with iocp env.
	@param length received data length.
	*/
	void notify_lobby_recv(size_t length);

	/**
	@brief notify recv from battle. - use with iocp env.
	@param length received data length.
	*/
	void notify_battle_recv(size_t length);

	/**
	@brief update packets to client. call before main logic.
	*/
	void update();

	//Connect Server Funcs

	/**
	@brief connect to lobby.
	@param iocp_key key for iocp. if there's no iocp handle, no need.
	@return bool that connection is estalished or not.
	*/
	bool connect_lobby(int iocp_key);

	/**
	@brief disconnect from lobby.
	*/
	void disconnect_lobby();

	/**
	@brief connect to battle.
	@param iocp_key key for iocp. if there's no iocp handle, no need.
	@return bool that connection is estalished or not.
	*/
	bool connect_battle(int iocp_key);

	/**
	@brief disconnect from battle.
	*/
	void disconnect_battle();

	//Send Packet To Server

	/**
	@brief request login to lobby.
	@param id login id. max 10 length.
	*/
	void request_login(const char* id);

	/**
	@brief request add friend to lobby.
	@param id : login id. max 10.
	*/
	void add_friend(const char* id);

	/**
	@brief answer accepting friend request to lobby.
	@param id : login id. max 10.
	*/
	void accept_friend(const char* id);

	/**
	@brief reqeust client enqueue match_make pool to lobby.
	*/
	void match_enqueue();

	/**
	@brief reqeust client dequeue match_make pool to lobby.
	*/
	void match_dequeue();

private:
	T& MainModule;									///< Main class need to attach NW MODULE.
	HANDLE iocp;									///< Handle for IOCP.
	SOCKET lobby_socket;							///< Socket for lobby.
	SOCKET battle_socket;							///< Socket for battle.
	vector<thread> threads;							///< Thread list operated in NW MODULE.
	vector<function<void(T&)>> lobby_callbacks;		///< Callbacks when lobby packet arrived.
	vector<function<void(T&)>> battle_callbacks;	///< Callbacks when battle packet arrived.

	OVER_EX lobby_over;								///< EXPENDED OVERLAPPED Structure for lobby.
	PACKET_BUFFER lobby_buffer;						///< Buffer for complete packet.
	OVER_EX battle_over;							///< EXPENDED OVERLAPPED Structure for battle.
	PACKET_BUFFER battle_buffer;					///< Buffer for complete packet.

private:

	void error_display(const char* msg, int err_no);

    /**
    @brief Initializing WSA environment.
    */
	void InitWSA();

	/**
	@brief if there's no iocp, make recv thread for continous recv from lobby.
	*/
	void RecvLobbyThread();

	/**
	@brief if there's no iocp, make recv thread for continous recv from battle.
	*/
	void RecvBattleThread();

	/**
	@brief send default type packet to lobby.
	@param type packet type.
	*/
	void send_default_packet(int type);

	/**
	@brief connect to server.
	@param socket return with new socket.
	@param address address to connecting server. ex) 192.168.0.1
	@param port server port with big-endian.
	@return true when success or not.
	*/
	bool connect_server(SOCKET& socket, const char* address, const short port);

	/**
	@brief read and make complete packet from buffer after recv and save to packet_buffer.
	@param packet_buffer complete packet will save to this.
	@param buffer buffer that need to drain.
	@param len received data length.
	*/
	void packet_drain(PACKET_BUFFER& packet_buffer, char* buffer, size_t len);

	void packet_parse();

	/**
	@brief process disconnect to socket and clear packet_buffer.
	@param socket socket that want to disconnect.
	@param buffer packet_buffer that want to disconnect.
	*/
	void process_disconnect(SOCKET& socket, PACKET_BUFFER& buffer);

	/**
	@brief process lobby packet and call appropriate callback.
	@param type packets type.
	@param buffer packets data.
	*/
	void process_lobby_packet(int packet_type, const void* buffer);

	/**
	@brief process battle packet and call appropriate callback.
	@param type packets type.
	@param buffer packets data.
	*/
	void process_battle_packet(int packet_type, const void* buffer);


};

