#include "DMRoom.h"

bool DMRoom::update(float elapsedTime)
{
    process_packet_vector();
    bool retval = game_logic(elapsedTime);
    send_game_state();
    return retval;
}

void DMRoom::process_packet_vector()
{
	packets.clear();
	packet_lock.lock();
	packets.emplace_back(packet_vector.data, packet_vector.len);
	packet_vector.clear();
	packet_lock.unlock();

	char* packet_pos = packets.data;
	int packet_size = 0;
	int packet_length = packets.len;
	while (packet_length > 0)
	{
		default_packet* dp = reinterpret_cast<default_packet*>(packet_pos);
		packet_size = dp->size;
		process_type_packet(packet_pos, dp->type);
		packet_length -= packet_size;
		packet_pos += packet_size;
	}
}
