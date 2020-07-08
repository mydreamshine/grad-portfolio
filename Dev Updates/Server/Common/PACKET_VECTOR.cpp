#include "PACKET_VECTOR.h"
#include <iostream>

PACKET_VECTOR::PACKET_VECTOR() : data(new char[100]), len(0), max_len(100) { };
PACKET_VECTOR::~PACKET_VECTOR() { delete[] data; data = nullptr; len = 0; max_len = 0; };

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