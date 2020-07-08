#include "PACKET_BUFFER.h"

void PACKET_VECTOR::emplace_back(void* src, size_t len)
{
	size_t require_len = this->len + len;
	if (require_len > max_len) {
		char* new_data = new char[require_len];
		memcpy(new_data, data, this->len);
		delete[] data;
		data = new_data;
		max_len = require_len;
	}
	memcpy(data + this->len, src, len);
	this->len += len;
}

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

