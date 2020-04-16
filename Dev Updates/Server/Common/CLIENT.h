#pragma once
#include "OVER_EX.h"

class ROOM;
class CLIENT
{
public:
	OVER_EX recv_over{};
	SOCKET socket{ INVALID_SOCKET };
	ROOM* room{ nullptr };

	int id{ 0 };
	char savedPacket[200]{};
	size_t saved_size{ 0 };
	size_t need_size{ 4 };

	CLIENT() {};
	~CLIENT() {};
	void set_recv();
	void error_display(const char* msg, int err_no);
};