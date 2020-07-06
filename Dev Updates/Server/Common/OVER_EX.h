#pragma once
#include <WS2tcpip.h>

constexpr auto MAX_BUFFER_SIZE = 256;

/**
@brief EXPENDED class for WSAOVERLAPPED structure for IOCP.
@author Gurnwoo Kim
*/
class OVER_EX
{
private:
	WSAOVERLAPPED over;				///< WSAOVERLAPPED structure.
	WSABUF wsabuf;					///< WSABUF structure.
	char packet[MAX_BUFFER_SIZE];	///< Inner recv buffer.
	int ev_type;					///< Event type for iocp. can be defined for its own goal.

public:
	/**
	@brief Basic Constructor.
	*/
	OVER_EX();

	/**
	@brief Set event type.
	@param ev set event.
	*/
	OVER_EX(int ev);

	/**
	@brief Set event type and copy buff to inner buffer. usually for send data.
	@param ev set event.
	@param buff want to send data pointer.
	*/
	OVER_EX(int ev, void* buff);

	/**
	@brief return wsabuf pointer.
	*/
	WSABUF* buffer() { return &wsabuf; }

	/**
	@brief return buffer pointer.
	*/
	char* data() { return packet; }

	/**
	@brief return overlapped structure pointer.
	*/
	WSAOVERLAPPED* overlapped() { return &over; }

	/**
	@brief return event type.
	*/
	int event_type() { return ev_type; }

	/**
	@brief reset for recv.
	*/
	void reset();

	/**
	@brief init for socket communication.
	*/
	void init();

	/**
	@brief set event.
	*/
	void set_event(int ev);
};