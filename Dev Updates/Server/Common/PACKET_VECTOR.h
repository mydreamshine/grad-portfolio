#pragma once

/**
@brief Vector for BYTE.
@author Gurnwoo Kim
*/

class PACKET_VECTOR {
public:
	char* data;
	size_t len;
	size_t max_len;
	PACKET_VECTOR();
	~PACKET_VECTOR();

	/**
	@brief insert data to vector.
	@param src pointer to insert data.
	@param len length of data.
	*/
	void emplace_back(void* src, size_t len);

	/**
	@brief clear vector.
	@details there's no change in data place. just clear saved length.
	*/
	void clear() { len = 0; };
};