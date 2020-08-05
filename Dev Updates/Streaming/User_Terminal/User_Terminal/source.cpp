#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <fstream>
#include <chrono>
#include <map>
#include <vector>
#include <atomic>
#include "..\..\Streaming_Server\Streaming_Server\packet_struct.h"
#include "..\..\..\Server\Common\OVER_EX.h"

#ifndef CLEANUP_H
#define CLEANUP_H

#include <utility>
#include <SDL.h>
#include "DECODER.h"
/*
 * Recurse through the list of arguments to clean up, cleaning up
 * the first one in the list each iteration.
 */
template<typename T, typename... Args>
void cleanup(T* t, Args&& ... args) {
	//Cleanup the first item in the list
	cleanup(t);
	//Recurse to clean up the remaining arguments
	cleanup(std::forward<Args>(args)...);
}
/*
 * These specializations serve to free the passed argument and also provide the
 * base cases for the recursive call above, eg. when args is only a single item
 * one of the specializations below will be called by
 * cleanup(std::forward<Args>(args)...), ending the recursion
 * We also make it safe to pass nullptrs to handle situations where we
 * don't want to bother finding out which values failed to load (and thus are null)
 * but rather just want to clean everything up and let cleanup sort it out
 */
template<>
inline void cleanup<SDL_Window>(SDL_Window* win) {
	if (!win) {
		return;
	}
	SDL_DestroyWindow(win);
}
template<>
inline void cleanup<SDL_Renderer>(SDL_Renderer* ren) {
	if (!ren) {
		return;
	}
	SDL_DestroyRenderer(ren);
}
template<>
inline void cleanup<SDL_Texture>(SDL_Texture* tex) {
	if (!tex) {
		return;
	}
	SDL_DestroyTexture(tex);
}
template<>
inline void cleanup<SDL_Surface>(SDL_Surface* surf) {
	if (!surf) {
		return;
	}
	SDL_FreeSurface(surf);
}

#endif

using namespace std;
char SDLK2VK(int sdlk) {
	//Only Process 'LEFT' Special Key.
	//Notes : VK_NUMPAD_0 and SDLK_BACKQUOTE returns SAME VALUE!!.
	switch (sdlk) {
	//Special
	case SDLK_BACKSPACE: return VK_BACK;
	case SDLK_TAB: return VK_TAB;
	case SDLK_RETURN: return VK_RETURN;
	case SDLK_LSHIFT: return VK_SHIFT;
	case SDLK_LCTRL: return VK_CONTROL;
	case SDLK_LALT: return VK_MENU;
	case SDLK_PAUSE: return VK_PAUSE;
	case SDLK_CAPSLOCK: return VK_CAPITAL;
	case SDLK_ESCAPE: return VK_ESCAPE;
	case SDLK_SPACE: return VK_SPACE;
	case SDLK_PAGEUP: return VK_PRIOR;
	case SDLK_PAGEDOWN: return VK_NEXT;
	case SDLK_END: return VK_END;
	case SDLK_HOME: return VK_HOME;
	case SDLK_LEFT: return VK_LEFT;
	case SDLK_UP: return VK_UP;
	case SDLK_RIGHT: return VK_RIGHT;
	case SDLK_DOWN: return VK_DOWN;
	case SDLK_PRINTSCREEN: return VK_SNAPSHOT;
	case SDLK_INSERT: return VK_INSERT;
	case SDLK_DELETE: return VK_DELETE;
	
	case SDLK_F1: return VK_F1;
	case SDLK_F2: return VK_F2;
	case SDLK_F3: return VK_F3;
	case SDLK_F4: return VK_F4;
	case SDLK_F5: return VK_F5;
	case SDLK_F6: return VK_F6;
	case SDLK_F7: return VK_F7;
	case SDLK_F8: return VK_F8;
	case SDLK_F9: return VK_F9;
	case SDLK_F10: return VK_F10;
	case SDLK_F11: return VK_F11;
	case SDLK_F12: return VK_F12;

	//Nums
	case SDLK_0: return '0';
	case SDLK_1: return '1';
	case SDLK_2: return '2';
	case SDLK_3: return '3';
	case SDLK_4: return '4';
	case SDLK_5: return '5';
	case SDLK_6: return '6';
	case SDLK_7: return '7';
	case SDLK_8: return '8';
	case SDLK_9: return '9';

	case SDLK_KP_0: return VK_NUMPAD0;
	case SDLK_KP_1: return VK_NUMPAD1;
	case SDLK_KP_2: return VK_NUMPAD2;
	case SDLK_KP_3: return VK_NUMPAD3;
	case SDLK_KP_4: return VK_NUMPAD4;
	case SDLK_KP_5: return VK_NUMPAD5;
	case SDLK_KP_6: return VK_NUMPAD6;
	case SDLK_KP_7: return VK_NUMPAD7;
	case SDLK_KP_8: return VK_NUMPAD8;
	case SDLK_KP_9: return VK_NUMPAD9;

	//Char
	case SDLK_a: return 'A';
	case SDLK_b: return 'B';
	case SDLK_c: return 'C';
	case SDLK_d: return 'D';
	case SDLK_e: return 'E';
	case SDLK_f: return 'F';
	case SDLK_g: return 'G';
	case SDLK_h: return 'H';
	case SDLK_i: return 'I';
	case SDLK_j: return 'J';
	case SDLK_k: return 'K';
	case SDLK_l: return 'L';
	case SDLK_m: return 'M';
	case SDLK_n: return 'N';
	case SDLK_o: return 'O';
	case SDLK_p: return 'P';
	case SDLK_q: return 'Q';
	case SDLK_r: return 'R';
	case SDLK_s: return 'S';
	case SDLK_t: return 'T';
	case SDLK_u: return 'U';
	case SDLK_v: return 'V';
	case SDLK_w: return 'W';
	case SDLK_x: return 'X';
	case SDLK_y: return 'Y';
	case SDLK_z: return 'Z';

    case SDLK_EXCLAIM: return '!';
    case SDLK_QUOTEDBL: return '"';
    case SDLK_HASH: return '#';
    case SDLK_PERCENT: return '%';
    case SDLK_DOLLAR: return '$';
    case SDLK_AMPERSAND: return '&';
    case SDLK_QUOTE: return '\'';
    case SDLK_LEFTPAREN: return '(';
    case SDLK_RIGHTPAREN: return ')';
    case SDLK_ASTERISK: return '*';
    case SDLK_PLUS: return '+';
    case SDLK_COMMA: return ';';
    case SDLK_MINUS: return '-';
    case SDLK_PERIOD: return '.';
    case SDLK_SLASH: return '/';
    case SDLK_COLON: return ':';
    case SDLK_SEMICOLON: return ';';
    case SDLK_LESS: return '<';
    case SDLK_EQUALS: return '=';
    case SDLK_GREATER: return '>';
    case SDLK_QUESTION: return '?';
    case SDLK_AT: return '@';
    case SDLK_LEFTBRACKET: return '[';
    case SDLK_BACKSLASH: return '\\';
    case SDLK_RIGHTBRACKET: return ']';
    case SDLK_CARET: return '^';
    case SDLK_UNDERSCORE: return '_';
    case SDLK_BACKQUOTE: return '`';

	case SDL_BUTTON_LEFT: return VK_LBUTTON;
	case SDL_BUTTON_RIGHT: return VK_RBUTTON;
	}
	return -1;
}

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FRAME_DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
const int FRAME_BUFFER_SIZE = FRAME_DATA_SIZE * 3;


std::wstring config_path{ L".\\TERMINAL_CONFIG.ini" };
atomic<int> fps;
atomic<int> rtt;

std::wstring server_addr;
unsigned short server_port;
unsigned short local_port;
SOCKET serverSocket;
SOCKET udpSocket;

HANDLE renderIOCP;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* streaming_data;
DECODER decoder;

void error_display(const char* msg, int err_no)
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
void logSDLError(std::ostream& os, const std::string& msg) {
	os << msg << " error: " << SDL_GetError() << std::endl;
}

void gen_default_config()
{
	WritePrivateProfileString(L"TERMINAL", L"SERVER_PORT", L"15700", config_path.c_str());
	WritePrivateProfileString(L"TERMINAL", L"SERVER_ADDR", L"127.0.0.1", config_path.c_str());
}
void InitConfig()
{
	wprintf(L"Initializing Configs...");
	std::ifstream ini{ config_path.c_str() };
	if (false == ini.is_open())
		gen_default_config();
	ini.close();

	wchar_t buffer[512];
	GetPrivateProfileString(L"TERMINAL", L"SERVER_PORT", L"15700", buffer, 512, config_path.c_str());
	server_port = std::stoi(buffer);

	GetPrivateProfileString(L"TERMINAL", L"SERVER_ADDR", L"127.0.0.1", buffer, 512, config_path.c_str());
	server_addr = std::wstring(buffer);
	wprintf(L" Done.\n");
}
void PostRender(char* frame, int frame_size)
{
	OVER_EX* over_ex = new OVER_EX{ 0 };
	over_ex->packet = frame;
	PostQueuedCompletionStatus(renderIOCP, 1, frame_size, over_ex->overlapped());
}
Uint32 RTTChecker(Uint32 interval, void* param)
{
	tss_packet_req_rtt packet{};
	send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
	return(interval);
}

void process_tcp_packet(PACKET_TYPE type, void* buffer) {
	switch (type) {
	case SST_TCP_FRAME: {
		Lpacket_inheritance* packet = reinterpret_cast<Lpacket_inheritance*>(buffer);
		int decodingBytes = packet->size - sizeof(Lpacket_inheritance);
		char* frame_data = new char[decodingBytes];
		memcpy(frame_data, (char*)buffer + sizeof(Lpacket_inheritance), decodingBytes);
		PostRender(frame_data, decodingBytes);
		break;
	}

	case TSS_REQ_RTT: {
		sst_packet_ack_rtt* ack_packet = reinterpret_cast<sst_packet_ack_rtt*>(buffer);
		rtt = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - ack_packet->current_time).count() / 2;
		SDL_Event e;
		e.type = SDL_USEREVENT;
		SDL_PushEvent(&e);
		break;
	}
	}
}
void tcp_recv_thread()
{
	UINT savedSize = 0;
	UINT8* savedPacket = new UINT8[FRAME_BUFFER_SIZE];
	UINT8* savedPointer = savedPacket;

	UINT8* receivedPacket = new UINT8[FRAME_BUFFER_SIZE];
	UINT8* receivedPointer = receivedPacket;

	Lpacket_inheritance* packet = reinterpret_cast<Lpacket_inheritance*>(savedPacket);
	LPACKET_SIZE needBytes = sizeof(LPACKET_SIZE);
	UINT decodingBytes = 0;

	SDL_AddTimer(1000, RTTChecker, NULL);

	while (true)
	{
		int retval = recv(serverSocket, reinterpret_cast<char*>(receivedPacket), FRAME_BUFFER_SIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) 
			break;
		
		while (retval > 0) {
			if (savedSize + retval >= needBytes) {
				UINT copyBytes = needBytes - savedSize;
				memcpy(savedPointer, receivedPointer, copyBytes);

				if (needBytes == sizeof(LPACKET_SIZE)) {
					needBytes = *reinterpret_cast<LPACKET_SIZE*>(savedPacket);
					continue;
				}
				else {
					//Process Packet
					process_tcp_packet(packet->type, savedPacket);
					needBytes = sizeof(LPACKET_SIZE);
				}

				retval -= copyBytes;
				if (retval != 0)
					receivedPointer += copyBytes;
				else
					receivedPointer = receivedPacket;
				savedPointer = savedPacket;
				savedSize = 0;
			}
			else {
				memcpy(savedPointer, receivedPointer, retval);
				retval -= retval;
				savedPointer += retval;
				savedSize += retval;
				receivedPointer = receivedPacket;
			}
		}
	}
	delete[] savedPacket;
	delete[] receivedPacket;
}

class FRAME_CONTAINER {
public:
	FRAME_CONTAINER(int total_count, int total_size) : current_count(0), total_count(total_count), total_size(total_size), checker(total_count, false) {
		frame = new char[total_count * PACKET_SPLIT_SIZE];
	};
	~FRAME_CONTAINER() { delete[] frame; };

	bool emplace(int current_count, void* buffer) {
		if (true == checker[current_count]) return false;
		checker[current_count] = true;
		memcpy(frame + PACKET_SPLIT_SIZE * current_count, buffer, PACKET_SPLIT_SIZE);
		return ++this->current_count == total_count;
	}
	const char* data() { return frame; };
private:
	int current_count;
	int total_count;
	int total_size;
	vector<bool> checker;
	char* frame;
};
void udp_recv_thread()
{


	char* receivedPacket = new char[sizeof(video_packet)];
	video_packet* packet = reinterpret_cast<video_packet*>(receivedPacket);

	int lastest_render_frame = -1;
	map<int, FRAME_CONTAINER*> frames;
	bool isComplete = false;

	SOCKADDR_IN from;
	int from_len = sizeof(from);
	memset(&from, 0, from_len);
	
	tss_packet_udp_port port_packet{ local_port };
	send(serverSocket, (const char*)&port_packet, port_packet.size, 0);

	while (true)
	{
		int retval = recvfrom(udpSocket, reinterpret_cast<char*>(receivedPacket), MAX_BUFFER_SIZE, 0, (struct sockaddr*)&from, &from_len);
		if (retval == 0) {
			break;
		}
		else if (retval == SOCKET_ERROR) {
			int error_no = WSAGetLastError();
			if (error_no == WSAEFAULT)
				continue;
			else
				break;
		}
		
		if (lastest_render_frame >= packet->frame_number) continue;
		if (packet->total_count == 0) continue;

		if (frames.count(packet->frame_number) == 0) {
			frames[packet->frame_number] = new FRAME_CONTAINER{ packet->total_count, packet->total_size };
		}

		isComplete = frames[packet->frame_number]->emplace(packet->current_count, packet->data);
		if (true == isComplete) {
			//Send Render Events.
			char* frame_data = new char[packet->total_size];
			memcpy(frame_data, frames[packet->frame_number]->data(), packet->total_size);
			PostRender(frame_data, packet->total_size);

			//Clear Old Frames.
			lastest_render_frame = packet->frame_number;
			for (auto i = frames.begin(); i != frames.end();) {
				if (lastest_render_frame >= i->first) {
					delete i->second;
					frames.erase(i++);
				}
				else ++i;
			}
		}
	}

	for (auto i = frames.begin(); i != frames.end();) {
			delete i->second;
			frames.erase(i++);
	}
	delete[] receivedPacket;
}

void renderer_thread() {
	DWORD num_byte;
	ULONGLONG key;
	PULONG_PTR p_key = &key;
	WSAOVERLAPPED* p_over;

	chrono::steady_clock::time_point prev_time, cur_time;
	prev_time = cur_time = chrono::high_resolution_clock::now();
	int update_counter {0};
	int update_interval{ 30 };
	float elapsedTime  {0};
	while (true)
	{
		GetQueuedCompletionStatus(renderIOCP, &num_byte, p_key, &p_over, INFINITE);
		OVER_EX* over_ex = reinterpret_cast<OVER_EX*> (p_over);

        decoder.decode(over_ex->data(), key);
        AVFrame* frame = decoder.flush(NULL);
        int ret = SDL_UpdateYUVTexture(streaming_data, NULL, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
        decoder.free_frame(&frame);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, streaming_data, NULL, NULL);
        SDL_RenderPresent(renderer);

		elapsedTime += chrono::duration<float>(cur_time - prev_time).count();
		prev_time = cur_time;
		cur_time = chrono::high_resolution_clock::now();
		if (++update_counter > update_interval) {
			update_counter = 0;
			fps = 1.0f / (elapsedTime / update_interval);
			elapsedTime = 0.0f;
		}
		delete over_ex;
	}
}
void event_loop()
{
	char title[256];
	int i_fps, i_rtt;
	SDL_Event e;
	while (true)
	{
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_KEYDOWN:
				if (e.key.repeat == 0) {
					tss_packet_keydown packet{SDLK2VK(e.key.keysym.sym)};
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
				}
				break;

			case SDL_KEYUP:
				if (e.key.repeat == 0) {
					tss_packet_keyup packet{ SDLK2VK(e.key.keysym.sym) };
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT)
				{
					tss_packet_mouse_button_down packet{SDLK2VK(e.button.button), e.motion.x, e.motion.y};
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT)
				{
					tss_packet_keyup packet{SDLK2VK(e.button.button)};
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
				}
				break;

			case SDL_QUIT:
				return;
				
			case SDL_USEREVENT:
				i_fps = fps; i_rtt = rtt;
				sprintf_s(title, "BattleArena / FPS : %d / Latency : %dms", i_fps, i_rtt);
				SDL_SetWindowTitle(window, title);
				break;

			default:
				break;
			}
		}
	}

}
int main(int, char**) {
	InitConfig();
	vector<thread> threads;
	renderIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	threads.emplace_back(renderer_thread);

	if (SDL_Init(SDL_INIT_TIMER || SDL_INIT_VIDEO) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	window = SDL_CreateWindow("BattleArena", 100, 100, SCREEN_WIDTH,
		SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}
	streaming_data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (streaming_data == nullptr) {
		cleanup(streaming_data);
		SDL_Quit();
		return 1;
	}

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (serverSocket == INVALID_SOCKET)
		error_display("WSASocket()", WSAGetLastError());

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	InetPton(AF_INET, server_addr.c_str(), reinterpret_cast<PVOID>(&serverAddr.sin_addr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(server_port);
	int serverLen = sizeof(serverAddr);

	int retval = connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "NW ERROR", "SERVER CONNECT FAIL", NULL);
	threads.emplace_back(tcp_recv_thread);

	getsockname(serverSocket, (sockaddr*)&serverAddr, &serverLen);
	local_port = ntohs(serverAddr.sin_port) + 1;

	//Setup UDP Channel.
	SOCKADDR_IN udpAddr;
	memset(&udpAddr, 0, sizeof(SOCKADDR_IN));
	udpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	udpAddr.sin_family = AF_INET;
	udpAddr.sin_port = htons(local_port);
	udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	retval = ::bind(udpSocket, reinterpret_cast<const SOCKADDR*>(&udpAddr), sizeof(udpAddr));
	if (retval == SOCKET_ERROR)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "NW ERROR", "UDP Binding ERROR", NULL);
	
	//threads.emplace_back(udp_recv_thread);

	event_loop();
	closesocket(serverSocket);
	closesocket(udpSocket);
	threads.back().~thread();
	WSACleanup();

	SDL_DestroyTexture(streaming_data);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}