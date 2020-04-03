#include "OVER_EX.h"
#include "..//LobbyServer/LobbyServer/lobby_protocol.h"

OVER_EX::OVER_EX() {}
OVER_EX::OVER_EX(int ev) :
	ev_type(ev)
{
	init();
}
OVER_EX::OVER_EX(int ev, void* buff) :
	OVER_EX(ev)
{
	common_default_packet* cdp = reinterpret_cast<common_default_packet*>(buff);
	memcpy(packet, cdp, cdp->size);
	wsabuf.len = cdp->size;
}
void OVER_EX::init()
{
	memset(&over, 0, sizeof(WSAOVERLAPPED));
	wsabuf.buf = packet;
	wsabuf.len = sizeof(packet);
}
void OVER_EX::reset()
{
	memset(&over, 0, sizeof(WSAOVERLAPPED));
}
void OVER_EX::set_event(int ev)
{
	ev_type = ev;
}