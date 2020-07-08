#include "PACKET_BUFFER.h"

PACKET_BUFFER::PACKET_BUFFER(size_t min_size) :
	min_size(min_size),
	need_size(min_size)
{
}

PACKET_BUFFER::~PACKET_BUFFER()
{
}

void PACKET_BUFFER::SavedtoComplete()
{
	c_lock.lock();
	c_packet.emplace_back(s_packet.data, s_packet.len);
	s_packet.clear();
	c_lock.unlock();
}

void PACKET_BUFFER::CompletetoProcess()
{
	c_lock.lock();
	p_packet.clear();
	p_packet.emplace_back(c_packet.data, c_packet.len);
	c_packet.clear();
	c_lock.unlock();
}

