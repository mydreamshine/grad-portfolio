#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WS2tcpip.h>
#include <SDL.h>
#include <string>

#ifndef CLEANUP_H
#define CLEANUP_H

#include <utility>
#include <SDL.h>

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
int main(int, char**) {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9000);

	int retval = connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval != SOCKET_ERROR)
		printf("연결 성공");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Lesson 2", 100, 100, SCREEN_WIDTH,
		SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}

	SDL_Texture* streaming_data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	//SDL_Texture* streaming_data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	if (streaming_data == nullptr) {
		cleanup(streaming_data);
		SDL_Quit();
		return 1;
	}

	UINT DataSize = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
	UINT savedSize = 0;
	
	UINT8* receivedPacket = new UINT8[DataSize];
	UINT8* receivedPointer = receivedPacket;
	UINT8* savedPacket = new UINT8[DataSize];
	UINT8* savedPointer = savedPacket;

	while (true)
	{
		int retval = recv(serverSocket, reinterpret_cast<char *>(receivedPacket), SCREEN_WIDTH * SCREEN_HEIGHT * 4, 0);
		if (retval == 0 || retval == SOCKET_ERROR) break;

		while (retval > 0) {
			if (savedSize + retval >= DataSize) {
				UINT needBytes = DataSize - savedSize;
				memcpy(savedPointer, receivedPointer, needBytes);
				
				//Process Packet
				int pitch;
				int* pixels;
				SDL_LockTexture(streaming_data, NULL, reinterpret_cast<void**>(&pixels), &pitch);
				memcpy(pixels, savedPacket, DataSize);
				SDL_UnlockTexture(streaming_data);

				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, streaming_data, NULL, NULL);
				SDL_RenderPresent(renderer);

				retval -= needBytes;
				if (retval != 0)
					receivedPointer += needBytes;
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

	closesocket(serverSocket);
	WSACleanup();

	SDL_DestroyTexture(streaming_data);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}