#include "OVER_EX.h"
#include "..\..\Streaming\Streaming_Server\Streaming_Server\packet_struct.h"

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
	packet(nullptr),
	over(),
	wsabuf()
{
}
OVER_EX::OVER_EX(int ev, size_t buf_size) :
	OVER_EX(ev)
{
	init(buf_size);
}

OVER_EX::OVER_EX(int ev, void* buff) :
	OVER_EX(ev)
{
	packet_inheritance* cdp = reinterpret_cast<packet_inheritance*>(buff);
	init(cdp->size);
	memcpy(packet, buff, cdp->size);
}

OVER_EX::OVER_EX(int ev, void* buff, size_t buf_size) :
	OVER_EX(ev)
{
	init(buf_size);
	memcpy(packet, buff, buf_size);
}
void OVER_EX::init(size_t buf_size)
{
	if (packet != nullptr) {
		delete[] packet;
		packet = nullptr;
	}
	packet = new char[buf_size];
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