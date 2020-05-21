#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include "packet_struct.h"

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

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FRAME_DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
const int MAX_BUFFER_SIZE = FRAME_DATA_SIZE * 3;

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
		if (retval == 0 || retval == SOCKET_ERROR) return;
		
		while (retval > 0) {
			if (savedSize + retval >= needBytes) {
				UINT copyBytes = needBytes - savedSize;
				memcpy(savedPointer, receivedPointer, copyBytes);

				if(needBytes == 4)
					needBytes = *reinterpret_cast<int*>(savedPacket);
				else {
					//Process Packet
					canRender = true;
					memcpy(completeFrame, savedPacket, needBytes);
					decodingBytes = needBytes;
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

		if (canRender == true) {
			decoder.decode(completeFrame, decodingBytes);
			AVFrame* frame = decoder.flush(NULL);
			int ret = SDL_UpdateYUVTexture(streaming_data, NULL, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, streaming_data, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}
}

void event_loop()
{
	SDL_Event e;
	while (true)
	{
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_KEYDOWN:
				if(e.key.repeat == 0)
				switch (e.key.keysym.sym)
				{
				case SDLK_w:
				{
					tss_packet_keydown packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYDOWN_W;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				case SDLK_s:
				{
					tss_packet_keydown packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYDOWN_S;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				case SDLK_a:
				{
					tss_packet_keydown packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYDOWN_A;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				case SDLK_d:
				{
					tss_packet_keydown packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYDOWN_D;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					POINT oldMouseCursor = { e.motion.x, e.motion.y };
					tss_packet_mouse_button_down packet;
					packet.size = sizeof(packet);
					packet.type = TSS_MOUSE_LBUTTON_DOWN;
					packet.x = oldMouseCursor.x;
					packet.y = oldMouseCursor.y;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					tss_packet_mouse_button_up packet;
					packet.size = sizeof(packet);
					packet.type = TSS_MOUSE_LBUTTON_UP;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
				}
				break;
			case SDL_KEYUP:
				if (e.key.repeat == 0)
				switch (e.key.keysym.sym)
				{
				case SDLK_w:
				{
					tss_packet_keyup packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYUP_W;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				case SDLK_s:
				{
					tss_packet_keyup packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYUP_S;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				case SDLK_a:
				{
					tss_packet_keyup packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYUP_A;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				case SDLK_d:
				{
					tss_packet_keyup packet;
					packet.size = sizeof(packet);
					packet.type = TSS_KEYUP_D;
					send(serverSocket, reinterpret_cast<const char*>(&packet), packet.size, 0);
					break;
				}
				}
				break;

			default:
				break;
			}
		}
	}

}
int main(int, char**) {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	window = SDL_CreateWindow("Lesson 2", 100, 100, SCREEN_WIDTH,
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
	inet_pton(AF_INET, "127.0.0.1", reinterpret_cast<PVOID>(&serverAddr.sin_addr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9000);

	int retval = connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval != SOCKET_ERROR)
		printf("연결 성공");

	std::thread recvThread{ RecvFunc };

	event_loop();

	recvThread.join();

	closesocket(serverSocket);
	WSACleanup();

	SDL_DestroyTexture(streaming_data);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}