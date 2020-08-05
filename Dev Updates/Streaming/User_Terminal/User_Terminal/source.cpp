#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <fstream>
#include <chrono>
#include "..\..\Streaming_Server\Streaming_Server\packet_struct.h"

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
const int MAX_BUFFER_SIZE = FRAME_DATA_SIZE * 3;

int STREMING_RENDER_EVENT = 0;

std::wstring config_path{ L".\\TERMINAL_CONFIG.ini" };
char title[256];
chrono::steady_clock::time_point prev_time, cur_time;

short server_port;
std::wstring server_addr;
SOCKET serverSocket;
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
	std::wcout << L"에러 " << lpMsgBuf << std::endl;
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

SDL_Texture* loadTexture(const std::string& file, SDL_Renderer* ren) {
	//Initialize to nullptr to avoid dangling pointer issues
	SDL_Texture* texture = nullptr;
	//Load the image
	SDL_Surface* loadedImage = SDL_LoadBMP(file.c_str());
	//If the loading went ok, convert to texture and return the texture
	if (loadedImage != nullptr) {
		texture = SDL_CreateTextureFromSurface(ren, loadedImage);
		SDL_FreeSurface(loadedImage);
		//Make sure converting went ok too
		if (texture == nullptr) {
			logSDLError(std::cout, "CreateTextureFromSurface");
		}
	}
	else {
		logSDLError(std::cout, "LoadBMP");
	}
	return texture;
}
void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y) {
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	//Query the texture to get its width and height to use
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, NULL, &dst);
}
void RecvFunc()
{
	bool canRender = false;
	UINT8* completeFrame = new UINT8[FRAME_DATA_SIZE];

	UINT savedSize = 0;
	UINT8* savedPacket = new UINT8[MAX_BUFFER_SIZE];
	UINT8* savedPointer = savedPacket;

	UINT8* receivedPacket = new UINT8[MAX_BUFFER_SIZE];
	UINT8* receivedPointer = receivedPacket;
	UINT needBytes = 4;
	UINT decodingBytes = 0;

	while (true)
	{
		int retval = recv(serverSocket, reinterpret_cast<char*>(receivedPacket), MAX_BUFFER_SIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) 
			break;
		
		while (retval > 0) {
			if (savedSize + retval >= needBytes) {
				UINT copyBytes = needBytes - savedSize;
				memcpy(savedPointer, receivedPointer, copyBytes);

				if (needBytes == 4) {
					needBytes = *reinterpret_cast<int*>(savedPacket);
					continue;
				}
				else {
					//Process Packet
					canRender = true;
					memcpy(completeFrame, savedPacket, needBytes);
					decodingBytes = needBytes;
					char* frame_data = new char[decodingBytes];
					memcpy(frame_data, completeFrame, decodingBytes);
					SDL_Event render_event;
					render_event.type = STREMING_RENDER_EVENT;
					render_event.user.code = decodingBytes;
					render_event.user.data1 = frame_data;
					SDL_PushEvent(&render_event);
					needBytes = 4;
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

	delete[] completeFrame;
	delete[] savedPacket;
	delete[] receivedPacket;
}

void event_loop()
{
	prev_time = cur_time = chrono::high_resolution_clock::now();

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

			default:
				if (e.type == STREMING_RENDER_EVENT) {
						decoder.decode(e.user.data1, e.user.code);
						AVFrame* frame = decoder.flush(NULL);
						delete e.user.data1;
						int ret = SDL_UpdateYUVTexture(streaming_data, NULL, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
						decoder.free_frame(&frame);

						SDL_RenderClear(renderer);
						SDL_RenderCopy(renderer, streaming_data, NULL, NULL);
						SDL_RenderPresent(renderer);

						sprintf(title, "BattleArena / FPS : %.1f", 1.0f / chrono::duration<float>(cur_time - prev_time).count());
						prev_time = cur_time;
						cur_time = chrono::high_resolution_clock::now();
						SDL_SetWindowTitle(window, title);
				}
				break;
			}
		}
	}

}
int main(int, char**) {
	InitConfig();

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
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

	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (serverSocket == INVALID_SOCKET)
		error_display("WSASocket()", WSAGetLastError());

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	InetPton(AF_INET, server_addr.c_str(), reinterpret_cast<PVOID>(&serverAddr.sin_addr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(server_port);

	int retval = connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval != SOCKET_ERROR)
		printf("연결 성공");

	STREMING_RENDER_EVENT = SDL_RegisterEvents(1);
	std::thread recvThread{ RecvFunc };

	event_loop();
	closesocket(serverSocket);
	WSACleanup();

	recvThread.join();

	SDL_DestroyTexture(streaming_data);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	
	
	return 0;
}