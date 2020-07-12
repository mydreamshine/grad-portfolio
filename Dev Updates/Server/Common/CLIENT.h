#pragma once
#include "OVER_EX.h"
#include "PACKET_BUFFER.h"

class ROOM;
/**
@brief Client class for battle server.
@author Gurnwoo Kim
*/
class CLIENT
{
public:
	OVER_EX recv_over{}; ///< OVERRAPPED Wrapping class for IOCP.
	SOCKET socket{ INVALID_SOCKET }; ///< socket for client.
	ROOM* room{ nullptr }; ///< room where the clients connect.

	int id{ 0 }; ///< id
	char savedPacket[MAX_BUFFER_SIZE]{}; ///< packet buffer.
	size_t saved_size{ 0 }; ///< packet buffer data.
	size_t need_size{ 1 }; ///< packet buffer data.

	CLIENT() {};
	~CLIENT() {};

	/**
	@brief set recv on iocp env.
	*/
	void set_recv();
	void error_display(const char* msg, int err_no);
};