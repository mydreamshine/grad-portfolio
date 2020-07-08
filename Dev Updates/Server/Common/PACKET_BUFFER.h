#pragma once
#include <mutex>
#include "PACKET_VECTOR.h"

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
	std::mutex c_lock; ///< lock for c_packet.
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