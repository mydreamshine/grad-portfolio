#pragma once
#include <mutex>

using namespace std;

/**
@brief Vector for BYTE.
@author Gurnwoo Kim
*/
class PACKET_VECTOR {
public:
	char* data;
	size_t len;
	size_t max_len;
	PACKET_VECTOR() : data(new char[100]), len(0), max_len(100) { };
	~PACKET_VECTOR() { delete[] data; data = NULL; len = 0; max_len = 0; };
	
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

/**
@brief Buffer for Packet.
@author Gurnwoo Kim
@details Dummy -> Saved -> Complete(Shared variable for multithread) -> Process
*/
class PACKET_BUFFER {
public:
	PACKET_VECTOR d_packet; ///< Dummy buffer for raw packet.
	PACKET_VECTOR s_packet; ///< Saved buffer for complete packet before recv done.
	PACKET_VECTOR c_packet; ///< Complete buffer for complete packet after recv done.
	PACKET_VECTOR p_packet; ///< Process buffer for update.
	mutex c_lock; ///< lock for c_packet.
	size_t min_size; ///< minimum size for make complete packet.
	size_t need_size; ///< need size for make complete packet.

	PACKET_BUFFER(size_t min_size);
	~PACKET_BUFFER();

	/**
	@brief Move s_packet data to c_packet when recv done.
	*/
	void SavedtoComplete();

	/**
	@brief Move c_packet data to p_packet for update.
	*/
	void CompletetoProcess();
};