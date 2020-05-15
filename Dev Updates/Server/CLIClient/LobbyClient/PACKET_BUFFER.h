#pragma once
#include <mutex>

using namespace std;

class PACKET_VECTOR {
public:
	char* data;
	size_t len;
	size_t max_len;
	PACKET_VECTOR() : data(new char[100]), len(0), max_len(100) { };
	~PACKET_VECTOR() { delete[] data; data = NULL; len = 0; max_len = 0; };
	void emplace_back(void* src, size_t len) {
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
	};
	void clear() { len = 0; };
};

class PACKET_BUFFER {
public:
	PACKET_VECTOR d_packet;
	PACKET_VECTOR s_packet;
	PACKET_VECTOR c_packet;
	PACKET_VECTOR p_packet;
	mutex c_lock;
	size_t min_size;
	size_t need_size;

	PACKET_BUFFER(size_t min_size);
	~PACKET_BUFFER();
	void SavedtoComplete();
	void CompletetoProcess();
};