#include "ROOM.h"
#include "CLIENT.h"
constexpr int EV_SEND = 1;
ROOM::~ROOM() {};

void ROOM::send_packet(SOCKET socket, void* buff, size_t buff_len)
{
    OVER_EX* send_over = new OVER_EX{ EV_SEND, buff, buff_len };
    WSASend(socket, send_over->buffer(), 1, 0, 0, send_over->overlapped(), 0);
}
