#pragma once
#include <atomic>
#include <chrono>
#include <WS2tcpip.h>
#include <mutex>
#include <queue>

class CLIENT;
class ROOM
{
public:
	virtual ~ROOM();
	virtual void init() = 0; ///< call with constructor, Init Game World
	virtual bool regist(CLIENT* client) = 0; ///< when client is connected, server call this function
	virtual void disconnect(CLIENT* client) = 0; ///< when client is disconnected, server call this function
	virtual void start() = 0; ///< when all user connected, server call this function
	virtual void end() = 0; ///< when game end, server will delete this object, call end with destructor
	virtual bool update(float elapsedTime) = 0; ///<each update, server call this function with elapsed Time
	virtual void process_packet(CLIENT* client, int ReceivedBytes) = 0; ///when recv data from client buffer, server will call this with client and its length

	std::chrono::high_resolution_clock::time_point last_update_time; ///< last update time stamp.

protected:
	void send_packet(SOCKET socket, void* buff, size_t buff_len);
};