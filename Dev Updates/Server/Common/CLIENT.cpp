#include "CLIENT.h"
#include "ROOM.h"
#include <iostream>

	void CLIENT::set_recv()
	{
		recv_over.reset();
		DWORD flag = 0;
		if (SOCKET_ERROR == WSARecv(socket, recv_over.buffer(), 1, nullptr, &flag, recv_over.overlapped(), nullptr))
		{
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Error at WSARecv()", err_no);
		}
	}
	void CLIENT::error_display(const char* msg, int err_no)
	{
		WCHAR* lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err_no,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf, 0, NULL);
		std::cout << msg;
		std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
		while (true);
		LocalFree(lpMsgBuf);
	}