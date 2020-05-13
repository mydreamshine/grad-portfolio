#pragma once

/*
Copyright 2017 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "ScnMgr.h"
#include "Config.h"
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")

#include <thread>
#include <queue>
#include <mutex>

#include "..\\..\\Common\PacketMgr.h"
#include "..\\..\\LobbyServer\LobbyServer\lobby_protocol.h"

#define SERVERPORT 9000
#define MAX_BUFFER_SIZE 2000
#define BUF_SIZE 200
#define BATTLESERVER_PORT 15600



ScnMgr *g_ScnMgr = NULL;
SOCKET g_socket;
SOCKET lobbysocket;
string LOBBYIP, BATTLEIP;
int g_PrevTime;

constexpr int WIDTH_BIAS = WINDOW_WIDTH / 2;
constexpr int HEIGHT_BIAS = WINDOW_HEIGHT / 2;

//Message Queue
HANDLE connect_success;
mutex RecvLock;
queue<int> RecvQueue;
queue<Vector2d> RecvAddData;

void PacketReceiver(int room_id)
{
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(BATTLESERVER_PORT);
	InetPton(AF_INET, BATTLEIP.c_str(), &serverAddr.sin_addr);

	SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	::connect(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));

	cb_packet_request_login pk;
	pk.cdp.size = sizeof(cb_packet_request_login);
	pk.cdp.type = CB_PACKET_REQUEST_LOGIN;
	pk.room_id = room_id;
	send(serverSocket, reinterpret_cast<char*>(&pk), pk.cdp.size, 0);

	int myid;
	recv(serverSocket, (char*)& myid, sizeof(int), 0);
	g_ScnMgr->MYID = myid;
	g_ScnMgr->soc = serverSocket;

	int retval;
	
	size_t need_size = GENERAL_PACKET_SIZE;
	size_t saved_size = 0;

	int packet = 0;
	Vector2d addData;
	char buffer[MAX_BUFFER_SIZE];
	char savedPacket[EXTEND_PACKET_SIZE];
	char* buf_pos = NULL;

	queue<int> tempQueue;
	queue<Vector2d> tempAddQueue;

	while (true) {
		retval = recv(serverSocket, buffer, MAX_BUFFER_SIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR)
		{
			closesocket(serverSocket);
			return;
		}

		buf_pos = buffer;
		while (retval > 0)
		{
			if (retval + saved_size >= need_size) {
				int copy_size = (need_size - saved_size);
				memcpy(savedPacket + saved_size, buf_pos, copy_size);
				buf_pos += copy_size;
				retval -= copy_size;

				switch (need_size) {
				case GENERAL_PACKET_SIZE:
					packet = *(int*)savedPacket;
					if (is_extend_packet_client(packet)) {
						need_size = EXTEND_PACKET_SIZE;
						saved_size = GENERAL_PACKET_SIZE;
						continue;
					}
					need_size = GENERAL_PACKET_SIZE;
					tempQueue.emplace(packet);
					break;

				case EXTEND_PACKET_SIZE:
					packet = *(int*)savedPacket;
					addData = *(Vector2d*)(savedPacket + GENERAL_PACKET_SIZE);
					tempQueue.emplace(packet);
					tempAddQueue.emplace(addData);
					need_size = GENERAL_PACKET_SIZE;
					break;
				}
				saved_size = 0;
			}
			else {
				memcpy(savedPacket + saved_size, buf_pos, retval);
				saved_size += retval;
				retval = 0;
			}
		}

		RecvLock.lock();
		while (tempQueue.empty() != true) {
			RecvQueue.emplace(tempQueue.front());
			tempQueue.pop();
		}
		while (tempAddQueue.empty() != true) {
			RecvAddData.emplace(tempAddQueue.front());
			tempAddQueue.pop();
		}
		RecvLock.unlock();
	}
}
void ProcessObjectPacket(const int& packet, queue<Vector2d>& addData) {
	int mover = get_packet_player_num(packet);
	int objtype = get_packet_obj_type(packet);
	int objinfo = get_packet_obj_info(packet);
	
	switch (objinfo)
	{
	case obj_position:
	{
			switch (objtype)
			{
				case obj_player: {
					Vector2d Data = addData.front(); addData.pop();
					if (g_ScnMgr->m_players[mover].m_visible == false) {
						g_ScnMgr->m_players[mover].m_dst = Data;
						g_ScnMgr->m_players[mover].m_pos = Data;
					}
					else
						g_ScnMgr->m_players[mover].m_dst = Data;
					g_ScnMgr->m_players[mover].m_visible = true;
				}
					break;

				case obj_bullet: {
					if (mover != player4) {
						int idx = get_packet_bullet_idx(packet);
						int type = get_packet_bullet_type(packet);
						Vector2d Data = addData.front(); addData.pop();
						if (g_ScnMgr->m_players[mover].bullets[idx].m_visible == false)
							g_ScnMgr->m_players[mover].bullets[idx].m_pos = g_ScnMgr->m_players[mover].m_pos;
						g_ScnMgr->m_players[mover].bullets[idx].m_visible = true;
						g_ScnMgr->m_players[mover].bullets[idx].type = type;
						g_ScnMgr->m_players[mover].bullets[idx].m_dst = Data;
					}
					else {
						int idx = get_packet_bullet_idx(packet);
						Vector2d Data = addData.front(); addData.pop();
						if (g_ScnMgr->m_commonBullet[idx]->m_visible == false)
							g_ScnMgr->m_commonBullet[idx]->m_pos = Data;
						g_ScnMgr->m_commonBullet[idx]->m_visible = true;
						g_ScnMgr->m_commonBullet[idx]->m_dst = Data;
					}
				}
					break;

				case obj_item: {
					int idx = get_packet_bullet_idx(packet);
					int type = get_packet_bullet_type(packet);
					Vector2d Data = addData.front(); addData.pop();
					g_ScnMgr->m_item[idx].m_visible = true;
					g_ScnMgr->m_item[idx].type = type;
					g_ScnMgr->m_item[idx].m_pos = Data;
				}
								 break;
			}	
	}
	break;

	case obj_destroy:
		switch (objtype)
		{
			case obj_player:
				g_ScnMgr->m_players[mover].m_visible = false;
				g_ScnMgr->alive_count--;
				break;

			case obj_bullet: {
				int idx = get_packet_bullet_idx(packet);
				if (mover != player4)
					g_ScnMgr->m_players[mover].bullets[idx].m_visible = false;
				else
					g_ScnMgr->m_commonBullet[idx]->m_visible = false;
				break;
			}

			case obj_item: {
				int idx = get_packet_bullet_idx(packet);
				g_ScnMgr->m_item[idx].m_visible = false;
				break;
			}
		}
		break;
	}

}
void ProcessEventPacket(const int& packet) {
	int client = get_packet_player_num(packet);
	int event_type = get_packet_event_type(packet);

	switch (event_type)
	{
	case event_hit: {
		int damage = get_packet_bullet_idx(packet);
		g_ScnMgr->m_players[client].m_hp -= damage;
		break;
	}
	case event_chat: {
		int chat = get_packet_chat(packet);
		g_ScnMgr->m_players[client].m_chat = chat;
		g_ScnMgr->m_players[client].emotion_time = g_ScnMgr->m_players[client].emotion_duration_time;
		break;
	}
	}
}
void ProcessSystemPacket(const int& packet) {
	int client = get_packet_player_num(packet);
	int system_type = get_packet_system_type(packet);

	switch (system_type)
	{
	case system_end: {
		closesocket(g_ScnMgr->soc);
		g_ScnMgr->winner = client;
		g_ScnMgr->m_state = ScnMgr::state_end;
		break;
	}
	case system_start:
		g_ScnMgr->m_state = ScnMgr::state_play;
		g_ScnMgr->Init();
		break;
	}
}
void ProcessPacket() {
	queue<int> cRecvQueue;
	queue<Vector2d> cAddData;

	RecvLock.lock();
	if (RecvQueue.empty() == false) {
		cRecvQueue = RecvQueue;
		while (RecvQueue.empty() == false)
			RecvQueue.pop();

		if (RecvAddData.empty() == false) {
			cAddData = RecvAddData;
			while (RecvAddData.empty() == false)
				RecvAddData.pop();
		}
	}
	RecvLock.unlock();

	while (cRecvQueue.empty() == false)
	{
		int packet = cRecvQueue.front(); cRecvQueue.pop();
		int packet_type = get_packet_type(packet);

		switch (packet_type)
		{
		case p_system:
			ProcessSystemPacket(packet);
			break;

		case p_obj:
			ProcessObjectPacket(packet, cAddData);
			break;

		case p_event:
			ProcessEventPacket(packet);
			break;
		}
	}
}

void ProcessLobbyPacket(void* ptr)
{
	common_default_packet* packet = reinterpret_cast<common_default_packet*>(ptr);

	switch (packet->type)
	{
	case SC_PACKET_MATCH_ENQUEUE:
		cout << "[CHANGE STATE - MATCH ENQUEUE]" << endl;
		g_ScnMgr->m_state = ScnMgr::state_wait;
		//g_ScnMgr->RenderScene();
		//glutSwapBuffers();
		break;

	//case SC_PACKET_MATCH_DEQUEUE:
	//	cout << "[CHANGE STATE - MATCH DEQUEUE]" << endl;
	//	state = false;
	//	break;

	case SC_PACKET_MATCH_ROOM_INFO: {
		sc_packet_match_room_info* room_info = reinterpret_cast<sc_packet_match_room_info*>(ptr);
		cout << "[CHANGE STATE - Get GameRoom, Access To Battle Server] - " << room_info->room_id << endl;

		thread recvThread{ PacketReceiver, room_info->room_id };
		recvThread.detach();
	}
		break;

	default:
		printf("Unknown PACKET type [%d]\n", packet->type);
		break;
	}
}
void process_lobby_data(void* net_buf, size_t io_byte)
{
	char* ptr = reinterpret_cast<char*>(net_buf);
	static common_default_packet* packet;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) {
			packet = reinterpret_cast<common_default_packet*>(ptr);
			in_packet_size = packet->size;
		}
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessLobbyPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}
void RecvLobbyFunc(SOCKET serverSocket)
{
	char ReceivedPacket[BUF_SIZE];
	int ReceivedBytes = 0;
	while (true)
	{
		ReceivedBytes = recv(serverSocket, ReceivedPacket, BUF_SIZE, 0);
		if (ReceivedBytes == 0 || ReceivedBytes == SOCKET_ERROR) {
			cout << "[Connection Closed]" << endl;
			return;
		}
		process_lobby_data(ReceivedPacket, ReceivedBytes);
	}
}

void RenderScene(int temp)
{
	int curTime = glutGet(GLUT_ELAPSED_TIME);
	int eTime = curTime - g_PrevTime;
	g_PrevTime = curTime;

	ProcessPacket();
	g_ScnMgr->Update(eTime / 1000.f);
	g_ScnMgr->RenderScene();
	glutSwapBuffers();

	glutTimerFunc(8, RenderScene, 0);
}

void Display(void)
{

}

void Idle(void)
{
}

void MouseInput(int button, int state, int x, int y)
{
	std::cout << "X : " << x << ", Y : " << y << endl;
	g_ScnMgr->MouseInput(button, state, x-WIDTH_BIAS, -y+HEIGHT_BIAS);
}
void MouseMotion(int x, int y)
{
	g_ScnMgr->MouseMotion(x - WIDTH_BIAS, -y + HEIGHT_BIAS);
}

void KeyDownInput(unsigned char key, int x, int y)
{
	switch (g_ScnMgr->m_state)
	{
	case ScnMgr::state_title:
	case ScnMgr::state_connect_error:
	{
				g_ScnMgr->m_state = ScnMgr::state_connect;
				g_ScnMgr->RenderScene();
				glutSwapBuffers();
				//SOCKADDR_IN serverAddr;
				//serverAddr.sin_family = AF_INET;
				//serverAddr.sin_port = htons(SERVERPORT);
				//serverAddr.sin_addr.s_addr = inet_addr(SERVERIP.data());
				//g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				//int retval = ::connect(g_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
				//if (retval == SOCKET_ERROR) {
				//	closesocket(g_socket);
				//	cout << "연결에러" << endl;
				//	g_ScnMgr->m_state = ScnMgr::state_connect_error;
				//}
				//else
				//{
				//	g_ScnMgr->m_state = ScnMgr::state_wait;
				//	cout << "성공" << endl;
				//	int myid;
				//	recv(g_socket, (char*)&myid, sizeof(int), 0);
				//	g_ScnMgr->MYID = myid;
				//	g_ScnMgr->soc = g_socket;
				//	thread recvThread{ PacketReceiver, g_socket };
				//	recvThread.detach();
				//}
		common_default_packet cdp;
		cdp.size = sizeof(common_default_packet);
		cdp.type = CS_PACKET_MATCH_ENQUEUE;
		send(lobbysocket, (const char*)(&cdp), cdp.size, 0);
		break;
	}
	case ScnMgr::state_play:
		g_ScnMgr->KeyDownInput(key, x, y);
		break;

	case ScnMgr::state_end: {
		g_ScnMgr->m_state = ScnMgr::state_title;
		break;
	}
	}
}
void KeyUpInput(unsigned char key, int x, int y)
{
	g_ScnMgr->KeyUpInput(key, x, y);
}

int main(int argc, char **argv)
{
	// Initialize Socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	ifstream in("lobby_ip.txt");
	in >> LOBBYIP;
	in.close();
	in.open("battle_ip.txt");
	in >> BATTLEIP;
	in.close();

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(LOBBYSERVER_PORT);
	InetPton(AF_INET, LOBBYIP.c_str(), &serverAddr.sin_addr);

	lobbysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	::connect(lobbysocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));

	cout << "서버 접속" << endl;

	thread recvThread{ RecvLobbyFunc, lobbysocket };

	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	const char *windowTitle = "Game Software Engineering KPU";
	glutCreateWindow("Game Software Engineering KPU");
	HWND windowHandle = FindWindow(NULL, windowTitle);
	glewInit();

	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}

	// Initialize Renderer
	g_ScnMgr = new ScnMgr();

	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyDownInput);
	glutKeyboardUpFunc(KeyUpInput);
	glutIgnoreKeyRepeat(1);
	glutMouseFunc(MouseInput);
	glutMotionFunc(MouseMotion);

	g_PrevTime = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(16, RenderScene, 0);

	glutMainLoop();

	if(g_ScnMgr)
		delete g_ScnMgr;
	g_ScnMgr = NULL;

	closesocket(g_socket);
	WSACleanup();
    return 0;
}