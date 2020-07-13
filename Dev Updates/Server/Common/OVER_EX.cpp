#include "OVER_EX.h"
#include "..//LobbyServer/LobbyServer/lobby_protocol.h"

OVER_EX::~OVER_EX() {
	if (packet != nullptr) {
		delete[] packet;
		packet = nullptr;
	}
}

OVER_EX::OVER_EX() :
	ev_type(0),
	packet(nullptr),
	over(),
	wsabuf()
{}
OVER_EX::OVER_EX(int ev) :
	ev_type(ev),
	packet(nullptr)
{
}
OVER_EX::OVER_EX(int ev, size_t buf_size) :
	OVER_EX(ev)
{
	packet = new char[buf_size];
	init(buf_size);
}

OVER_EX::OVER_EX(int ev, void* buff) :
	OVER_EX(ev)
{
	common_default_packet* cdp = reinterpret_cast<common_default_packet*>(buff);
	packet = new char[cdp->size];
	init(cdp->size);
	memcpy(packet, cdp, cdp->size);
}
void OVER_EX::init(size_t buf_size)
{
	memset(&over, 0, sizeof(WSAOVERLAPPED));
	wsabuf.buf = packet;
	wsabuf.len = buf_size;
}
void OVER_EX::reset()
{
	memset(&over, 0, sizeof(WSAOVERLAPPED));
}
void OVER_EX::set_event(int ev)
{
	ev_type = ev;
}