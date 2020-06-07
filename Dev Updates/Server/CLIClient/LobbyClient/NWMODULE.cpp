#include "NWMODULE.h"
#include <CSOCKADDR_IN.h>
#include <OVER_EX.h>
#include <iostream>

#include "../..//LobbyServer/LobbyServer/lobby_protocol.h"


template <class T>
void NWMODULE<T>::error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"���� " << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

/**
*@brief Initializing WSA environment.
*/
template <class T>
void NWMODULE<T>::InitWSA()
{
	WSADATA wsa;
	if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsa))
		error_display("Error at WSAStartup()", WSAGetLastError());
}

/**
*@brief send default type packet to lobby.
*@param type : packet type.
*/
template <class T>
void NWMODULE<T>::send_default_packet(int type)
{
	common_default_packet cdp;
	cdp.size = sizeof(common_default_packet);
	cdp.type = type;
	send(lobby_socket, reinterpret_cast<const char*>(&cdp), cdp.size, 0);
}

/**
*@brief connect to server.
*@param socket : return with new socket.
*@param address : address to connecting server. ex) 192.168.0.1
*@param port : server port with big-endian.
*@return success or not.
*/
template <class T>
bool NWMODULE<T>::connect_server(SOCKET& socket, const char* address, const short port)
{
	socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == socket) {
		error_display("Error at WSASocketW()", WSAGetLastError());
		return false;
	}
	CSOCKADDR_IN serverAddr{ address, port };
	int retval = ::connect(socket, serverAddr.getSockAddr(), *serverAddr.len());
	if (SOCKET_ERROR == retval) {
		error_display("Error at Lobby Connect()", WSAGetLastError());
		return false;
	}
	return true;
}

/**
*@brief read and make complete packet from buffer after recv and save to packet_buffer.
*@param packet_buffer : complete packet will save to this.
*@param buffer : buffer that need to drain.
*@param len : received data length.
*/
template<class T>
void NWMODULE<T>::packet_drain(PACKET_BUFFER& packet_buffer, char* buffer, size_t len)
{
	size_t& min_size = packet_buffer.min_size;
	size_t& need_size = packet_buffer.need_size;
	size_t& saved_size = packet_buffer.d_packet.len;
	PACKET_VECTOR& d_packet = packet_buffer.d_packet;
	PACKET_VECTOR& s_packet = packet_buffer.s_packet;

	size_t copy_size = 0;
	char* buf_ptr = reinterpret_cast<char*>(buffer);

	while (0 < len) {
		if (len + saved_size >= need_size) {
			copy_size = need_size - saved_size;
			d_packet.emplace_back(buf_ptr, copy_size);
			if (need_size == min_size) {
				need_size = *reinterpret_cast<SIZE_TYPE*>(d_packet.data);
				saved_size -= copy_size;
				continue;
			}
			s_packet.emplace_back(d_packet.data, d_packet.len);
			buf_ptr += copy_size;
			len -= copy_size;
			need_size = min_size; d_packet.clear();
		}
		else {
			d_packet.emplace_back(buf_ptr, len);
			len = 0;
		}
	}
	lobby_buffer.SavedtoComplete();
}

/**
*@brief process disconnect to socket and clear packet_buffer.
*@param socket : socket that want to disconnect.
*@param buffer : packet_buffer that want to disconnect.
*/
template<class T>
void NWMODULE<T>::process_disconnect(SOCKET& socket, PACKET_BUFFER& buffer)
{
	closesocket(socket);
	socket = INVALID_SOCKET;
	buffer.d_packet.clear();
	buffer.c_lock.lock();
	buffer.c_packet.clear();
	buffer.c_lock.unlock();
}

/**
*@brief process lobby packet.
*@param type : packets type.
*@param buffer : packets data.
*/
template <class T>
void NWMODULE<T>::process_lobby_packet(int packet_type, const void* buffer)
{
	callbacks[packet_type](MainModule);
	//switch (packet_type)
	//{
	//case SC_PACKET_LOGIN_OK:
	//	/*cout << "[LOGIN SUCCESS]" << endl;*/
	//	callbacks[SC_PACKET_LOGIN_OK](MainModule);
	//	break;

	//case SC_PACKET_LOGIN_FAIL:
	//	cout << "[LOGIN FAIL]" << endl;
	//	break;

	//case CS_PACKET_REQUEST_FRIEND: {
	//	const cs_packet_request_friend* cprf = reinterpret_cast<const cs_packet_request_friend*>(pk);
	//	printf("%s�κ��� ģ����û�� �Խ��ϴ�.\n", cprf->id);
	//	break;
	//}
	//							 
	//case SC_PACKET_FRIEND_STATUS: {
	//	const sc_packet_friend_status* packet = reinterpret_cast<const sc_packet_friend_status*>(pk);
	//	if (packet->status == FRIEND_ONLINE)
	//		printf("%s�� �����߽��ϴ�.\n", packet->id);
	//	else
	//		printf("%s�� ������ �����߽��ϴ�.\n", packet->id);
	//	break;
	//}
	//							
	//case SC_PACKET_MATCH_ENQUEUE:
	//	cout << "[CHANGE STATE - MATCH ENQUEUE]" << endl;
	//	callbacks[SC_PACKET_MATCH_ENQUEUE](MainModule);
	//	break;

	//case SC_PACKET_MATCH_DEQUEUE:
	//	cout << "[CHANGE STATE - MATCH DEQUEUE]" << endl;
	//	callbacks[SC_PACKET_MATCH_DEQUEUE](MainModule);
	//	break;

	//case SC_PACKET_MATCH_ROOM_INFO: {
	//	const sc_packet_match_room_info* room_info = reinterpret_cast<const sc_packet_match_room_info*>(pk);
	//	cout << "[CHANGE STATE - Get GameRoom, Access To Battle Server] - " << room_info->room_id << endl;
	//	//��Ʋ���� ����
	//	break;
	//}
	//							  
	//default:
	//	printf("Unknown PACKET type [%d]\n", pk->type);
	//	break;
	//}
}

/**
*@brief if there's no iocp, make recv thread for continous recv from lobby.
*/
template <class T>
void NWMODULE<T>::RecvLobbyThread()
{
	size_t received_size = 0;
	while (true)
	{
		received_size = recv(lobby_socket, lobby_over.data(), MAX_BUFFER_SIZE, 0);
		if (received_size == 0 || received_size == SOCKET_ERROR) {
			cout << "[Connection Closed]" << endl;
			return;
		}
		packet_drain(lobby_buffer, lobby_over.data(), received_size);
	}
}

/**
*@brief if there's no iocp, make recv thread for continous recv from battle.
*/
template <class T>
void NWMODULE<T>::RecvBattleThread()
{
	size_t received_size = 0;
	while (true)
	{
		received_size = recv(battle_socket, battle_over.data(), MAX_BUFFER_SIZE, 0);
		if (received_size == 0 || received_size == SOCKET_ERROR) {
			cout << "[Connection Closed]" << endl;
			return;
		}
		packet_drain(battle_buffer, battle_over.data(), received_size);
	}
}

template <class T>
NWMODULE<T>::NWMODULE(T& MainModule, HANDLE iocp) : 
	iocp(iocp),
	lobby_socket(INVALID_SOCKET),
	battle_socket(INVALID_SOCKET),
	MainModule(MainModule),
	lobby_buffer(sizeof(SIZE_TYPE))
{
	callbacks.reserve(SC_PACKET_COUNT);
	for (int i = 0; i < SC_PACKET_COUNT; ++i)
		callbacks.emplace_back([a = i](T&) {printf("ENROLL LOBBY PACKET TYPE %d\n", a); });
	InitWSA();
}

template <class T>
NWMODULE<T>::~NWMODULE()
{
	for (int i = 0; i < threads.size(); ++i)
		threads[i].join();
	WSACleanup();
}

/**
*@brief connect to lobby.
*@param iocp_key : key for iocp. if there's no iocp handle, no need.
*@return bool that connection is estalished or not.
*/
template <class T>
bool NWMODULE<T>::connect_lobby(int iocp_key)
{
	bool retval = connect_server(lobby_socket, "127.0.0.1", LOBBYSERVER_PORT);
	if (false == retval) return retval;

	if (INVALID_HANDLE_VALUE == iocp)
		threads.emplace_back(&NWMODULE<T>::RecvLobbyThread, this);
	else {
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(lobby_socket), iocp, iocp_key, 0);
		//WSARecv()
	} // WSARecv ����, ���ϵ��

	return retval;
}

/**
*@brief disconnect from lobby.
*/
template<class T>
void NWMODULE<T>::disconnect_lobby()
{
	SOCKET tmp = lobby_socket;
	lobby_socket = INVALID_SOCKET;
	closesocket(tmp);
}

/**
*@brief connect to battle.
*@param iocp_key : key for iocp. if there's no iocp handle, no need.
*@return bool that connection is estalished or not.
*/
template <class T>
bool NWMODULE<T>::connect_battle(int iocp_key)
{
	bool retval = connect_server(battle_socket, "127.0.0.1", BATTLESERVER_PORT);
	if (false == retval) return retval;

	if (INVALID_HANDLE_VALUE == iocp)
		threads.emplace_back(&NWMODULE<T>::RecvBattleThread, this);
	else {
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(battle_socket), iocp, iocp_key, 0);
		//WSARecv()
	} // WSARecv ����, ���ϵ��

	return retval;
}

/**
*@brief disconnect from battle.
*/
template<class T>
void NWMODULE<T>::disconnect_battle()
{
	SOCKET tmp = battle_socket;
	battle_socket = INVALID_SOCKET;
	closesocket(tmp);
}

/**
*@brief request login to lobby.
*@param id : login id. max 10.
*/
template <class T>
void NWMODULE<T>::request_login(const char* id)
{
	string s_id {id};
	cs_packet_request_login packet;
	packet.cdp.size = sizeof(cs_packet_request_login);
	packet.cdp.type = CS_PACKET_REQUEST_LOGIN;
	strcpy_s(packet.id, ID_LENGTH-1, s_id.c_str());
	send(lobby_socket, reinterpret_cast<const char*>(&packet), packet.cdp.size, 0);
}

/**
*@brief request add friend to lobby.
*@param id : login id. max 10.
*/
template <class T>
void NWMODULE<T>::add_friend(const char* id)
{
	cs_packet_request_friend packet;
	packet.cdp.size = sizeof(cs_packet_request_friend);
	packet.cdp.type = CS_PACKET_REQUEST_FRIEND;
	strcpy_s(packet.id, ID_LENGTH-1, id);
	send(lobby_socket, reinterpret_cast<const char*>(&packet), packet.cdp.size, 0);
}

/**
*@brief answer accepting friend request to lobby.
*@param id : login id. max 10.
*/
template <class T>
void NWMODULE<T>::accept_friend(const char* id)
{
	cs_packet_accept_friend packet;
	packet.cdp.size = sizeof(cs_packet_accept_friend);
	packet.cdp.type = CS_PACKET_ACCEPT_FRIEND;
	strcpy_s(packet.id, ID_LENGTH - 1, id);
	send(lobby_socket, reinterpret_cast<const char*>(&packet), packet.cdp.size, 0);
}

/**
*@brief reqeust client enqueue match_make pool to lobby.
*/
template <class T>
void NWMODULE<T>::match_enqueue()
{
	send_default_packet(CS_PACKET_MATCH_ENQUEUE);
}

/**
*@brief reqeust client dequeue match_make pool to lobby.
*/
template <class T>
void NWMODULE<T>::match_dequeue()
{
	send_default_packet(CS_PACKET_MATCH_DEQUEUE);
}

/**
*@brief enroll lobby packets callback.
*@param packet_type : target packet.
*@param callback : when packet_type arrived, this callback will execute.
*/
template <class T>
void NWMODULE<T>::enroll_lobby_callback(int packet_type, function<void(T&)> callback)
{
	if (callback == nullptr) return;
	lobby_callbacks[packet_type] = callback;
}
/**
*@brief enroll battle packets callback.
*@param packet_type : target packet.
*@param callback : when packet_type arrived, this callback will execute.
*/
template <class T>
void NWMODULE<T>::enroll_battle_callback(int packet_type, function<void(T&)> callback)
{
	if (callback == nullptr) return;
	battle_callbacks[packet_type] = callback;
}
/**
*@brief notify recv from lobby. - iocp env.
*@param length : received data length.
*/
template<class T>
void NWMODULE<T>::notify_lobby_recv(size_t length)
{
	if (length == 0 || length == SOCKET_ERROR)
		process_disconnect(lobby_socket, lobby_buffer);
	else {
		packet_drain(lobby_buffer, lobby_over.data(), length);
		//recv �ѹ� ��
		/*DWORD flag = 0;
		lobby_over.reset();
		WSARecv(lobby_socket, lobby_over.buffer, 1, nullptr, &flag, lobby_over.overlapped(), nullptr);*/
	}
}
/**
*@brief notify recv from battle. - iocp env.
*@param length : received data length.
*/
template<class T>
void NWMODULE<T>::notify_battle_recv(size_t length)
{
	if (length == 0 || length == SOCKET_ERROR)
		process_disconnect(battle_socket, battle_buffer);
	else {
		packet_drain(battle_buffer, batle_over.data(), length);
		//recv �ѹ� ��
		/*DWORD flag = 0;
		lobby_over.reset();
		WSARecv(lobby_socket, lobby_over.buffer, 1, nullptr, &flag, lobby_over.overlapped(), nullptr);*/
	}
}

/**
*@brief update packets to client. call before main logic.
*/
template<class T>
void NWMODULE<T>::update()
{
	//Lobby Update
	lobby_buffer.CompletetoProcess();
	const char* packet_pos = lobby_buffer.p_packet.data;
	size_t packet_length = lobby_buffer.p_packet.len;
	size_t packet_size = 0;
	while (packet_length > 0)
	{
		const common_default_packet* packet = reinterpret_cast<const common_default_packet*>(packet_pos);
		packet_size = packet->size;
		process_lobby_packet(packet->type, packet_pos);
		packet_length -= packet_size;
		packet_pos += packet_size;
	}

	//Battle Update
	battle_buffer.CompletetoProcess();
	packet_pos = battle_buffer.p_packet.data;
	packet_length = battle_buffer.p_packet.len;
	packet_size = 0;
	while (packet_length > 0)
	{
		const common_default_packet* packet = reinterpret_cast<const common_default_packet*>(packet_pos);
		packet_size = packet->size;
		process_battle_packet(packet->type, packet_pos);
		packet_length -= packet_size;
		packet_pos += packet_size;
	}
}