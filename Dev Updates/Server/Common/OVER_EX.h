#pragma once
#include <WS2tcpip.h>

//Extend OVERLAPPED
constexpr auto MAX_BUFFER_SIZE = 256;

class OVER_EX
{
private:
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	char packet[MAX_BUFFER_SIZE];
	int ev_type;

public:
	OVER_EX();
	OVER_EX(int ev);
	OVER_EX(int ev, void* buff);
	WSABUF* buffer() { return &wsabuf; }
	char* data() { return packet; }
	WSAOVERLAPPED* overlapped() { return &over; }
	int event_type() { return ev_type; }
	void reset();
	void init();
	void set_event(int ev);
};