/*
BA Logic Server
클라이언트 로직 개발을 위한 1인 접속 서버
플레이어가 접속하면 서버는 30fps로 업데이트 및 패킷 전송

Recv 쓰레드에서는 지속적으로 클라이언트의 패킷을 받고 packet_vector에 저장
메인 쓰레드는 매 업데이트마다 packet_vector에 저장된 패킷을 복사하여 처리한 후 로직 실행
*/

#pragma comment(lib, "ws2_32")

#define SERVER_PORT 15600
#define MAX_BUFFER_SIZE 1500
#define MAX_USER 5

#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <fstream>
#include "Timer.h"
#include "protocol.h"

#include "HERO.h"
#include <map>

using namespace std;


SOCKET listen_socket;
SOCKET client_sock;
atomic_int cur_user;
atomic_int uid_pool;
HANDLE update_flag;
HANDLE accept_flag;

mutex packet_lock;
PACKET_VECTOR packet_vector; //수신 스레드와 게임 로직 간 공유되는 패킷 리스트
PACKET_VECTOR packets; //처리해야할 패킷 리스트 - 로직 시작 전 packet_vector에서 복사

//최종적으로 전송은 event_data + info_data 두개를 합쳐 유저에게 한번에 보냄
PACKET_VECTOR event_data; //전송될 이벤트 패킷(플레이어 힛, 리스폰 등)
PACKET_VECTOR info_data; //전송될 위치정보 패킷

mutex socket_lock;
map<int, SOCKET> sockets;
map<int, HERO> m_heros;
map<int, BULLET> m_bullets;
vector<BoundingBox> m_walls;
int bullet_uid;


//BB 로딩 함수
void loadMap(const char* path)
{
	printf("Loading Maps... ");
	ifstream in;
	in.open(path);
	int wall_count;
	float x, y, z;
	XMFLOAT3 center, extent;
	in >> wall_count;
	for (int i = 0; i < wall_count; ++i)
	{
		in >> x; in >> y; in >> z;
		center = XMFLOAT3(x, y, z);
		in >> x; in >> y; in >> z;
		extent = XMFLOAT3(x, y, z);
		m_walls.emplace_back(center, extent);
	}
	in.close();
	printf("Done.\n");
}

void send_game_start()
{
	default_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_GAME_START;
	event_data.emplace_back(&packet, packet.size);
}
void send_game_end()
{
	default_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_GAME_END;
	event_data.emplace_back(&packet, packet.size);
}
void send_player_uid(int uid)
{
	sc_player_uid packet;
	packet.size = sizeof(sc_player_uid);
	packet.type = SC_PLAYER_UID;
	packet.uid = uid;
	send(client_sock, (char*)&packet, packet.size, 0);
}
void send_create_player(int uid, float* pos, float* rot)
{
	sc_create_player packet;
	packet.size = sizeof(sc_create_player);
	packet.type = SC_CREATE_PLAYER;
	packet.uid = (char)uid;
	packet.hero = 0;
	packet.hp = 100;
	packet.pos[0] = pos[0]; packet.pos[1] = pos[1]; packet.pos[2] = pos[2];
	packet.rot[0] = rot[0]; packet.rot[1] = rot[1]; packet.rot[2] = rot[2];
	send(client_sock, (char*)&packet, packet.size, 0);
}
void send_create_bullet(int uid, BULLET& bullet)
{
	sc_create_bullet bullet_packet;
	bullet_packet.size = sizeof(sc_create_bullet);
	bullet_packet.type = SC_CREATE_BULLET;
	bullet_packet.uid = (char)uid;
	bullet_packet.pos[0] = bullet.pos.x; bullet_packet.pos[1] = bullet.pos.y; bullet_packet.pos[2] = bullet.pos.z;
	bullet_packet.rot[0] = bullet.rot.x; bullet_packet.rot[1] = bullet.rot.y; bullet_packet.rot[2] = bullet.rot.z;
	event_data.emplace_back(&bullet_packet, bullet_packet.size);
}
void send_destroy_bullet(int uid, BULLET& bullet)
{
	sc_destroy_bullet packet;
	packet.size = sizeof(sc_destroy_bullet);
	packet.type = SC_DESTROY_BULLET;
	packet.uid = (char)uid;
	event_data.emplace_back(&packet, packet.size);
	bullet.isDestroy = true;
}
void send_sc_hit(int uid, int damage)
{
	sc_hit packet;
	packet.size = sizeof(sc_hit);
	packet.type = SC_HIT;
	packet.uid = uid;
	packet.damage = damage;
	event_data.emplace_back(&packet, packet.size);
}
void send_player_info(int uid, HERO& hero)
{
	sc_player_info info_packet;
	info_packet.size = sizeof(sc_player_info);
	info_packet.type = SC_PLAYER_INFO;
	info_packet.uid = uid;
	info_packet.pos[0] = hero.pos.x; info_packet.pos[1] = hero.pos.y; info_packet.pos[2] = hero.pos.z;
	info_packet.rot[0] = hero.rot.x; info_packet.rot[1] = hero.rot.y; info_packet.rot[2] = hero.rot.z;
	info_data.emplace_back(&info_packet, info_packet.size);
}
void send_bullet_info(int uid, BULLET& bullet)
{
	sc_bullet_info info_packet;
	info_packet.size = sizeof(sc_player_info);
	info_packet.type = SC_BULLET_INFO;
	info_packet.uid = uid;
	info_packet.pos[0] = bullet.pos.x; info_packet.pos[1] = bullet.pos.y; info_packet.pos[2] = bullet.pos.z;
	info_packet.rot[0] = bullet.rot.x; info_packet.rot[1] = bullet.rot.y; info_packet.rot[2] = bullet.rot.z;
	info_data.emplace_back(&info_packet, info_packet.size);
}

void err_display(const char* msg) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	while (1);
}
void recvFunc()
{
	//클라 데이터
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(SOCKADDR_IN);

	while (true) {
		//클라이언트 접속
		if (cur_user == MAX_USER) {
			WaitForSingleObject(accept_flag, INFINITE);
			ResetEvent(accept_flag);
		}

		client_sock = accept(listen_socket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		int uid = uid_pool++;

		printf("Player Accept\n");

		m_heros.emplace(make_pair(uid, HERO{}));
		printf("Send UID - %d\n", uid);
		send_player_uid(uid);

		float pos[3] = { 0, 0, 0 };
		float rot[3] = { 0, 0, 0 };
		send_create_player(uid, pos, rot);
		printf("Send Create Player - %d\n", uid);
		SetEvent(update_flag);
		//Done.

		socket_lock.lock();
		sockets.emplace(make_pair(uid, client_sock));
		socket_lock.unlock();

		thread t{ recvThread, client_sock, uid };
		t.detach();
	}
}
void recvThread(SOCKET client_sock, int uid) {
	//패킷 데이터
	PACKET_VECTOR dummy_packet_vector;
	int retval;
	size_t need_size = 0;
	size_t saved_size = 0;

	char buffer[MAX_BUFFER_SIZE];
	char savedPacket[MAX_BUFFER_SIZE];
	char* buf_pos = NULL;

	//패킷 처리
	while (true) {
		retval = recv(client_sock, buffer, MAX_BUFFER_SIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR)
		{
			need_size = 0;
			saved_size = 0;
			dummy_packet_vector.clear();

			socket_lock.lock();
			sockets.erase(uid);
			socket_lock.unlock();

			//유저 종료
			cur_user = 0;
			closesocket(client_sock);
			printf("Player Disconnected\n");
			return;
		}

		buf_pos = buffer;
		while (retval > 0)
		{
			if (need_size == 0) need_size = *buf_pos; // 모든 사이즈 패킷이 char라는 가정 하에
			if (retval + saved_size >= need_size) {
				int copy_size = (need_size - saved_size);
				memcpy(savedPacket + saved_size, buf_pos, copy_size);

				//패킷 완성
				dummy_packet_vector.emplace_back(savedPacket, need_size);

				buf_pos += copy_size;
				retval -= copy_size;
				saved_size = 0;
				need_size = 0;
			}
			else {
				memcpy(savedPacket + saved_size, buf_pos, retval);
				saved_size += retval;
				retval = 0;
			}
		}

		//Insert DummyVector To Real Vector
		packet_lock.lock();
		packet_vector.emplace_back(dummy_packet_vector.data, dummy_packet_vector.len);
		packet_lock.unlock();
		dummy_packet_vector.clear();
	}
}

void process_type_packet(void* buffer, int type) {
	switch (type)
	{
	case CS_KEYUP: {
		cs_key_info* packet = reinterpret_cast<cs_key_info*>(buffer);
		m_heros[packet->uid].set_key(1, packet->key);
		printf("KEYUP from %d, %c\n", packet->uid, packet->key);
		break;
	}

	case CS_KEYDOWN: {
		cs_key_info* packet = reinterpret_cast<cs_key_info*>(buffer);
		m_heros[packet->uid].set_key(0, packet->key);
		printf("KEYDOWN from %d, %c\n", packet->uid, packet->key);
		break;
	}

	case CS_ATTACK: {
		cs_attack* packet = reinterpret_cast<cs_attack*>(buffer);
		Vector3d dir{ packet->dir[0] ,packet->dir[1], packet->dir[2] };
		BULLET bullet = m_heros[packet->uid].get_bullet(packet->uid, dir);
		int new_uid = bullet_uid++;
		m_bullets.emplace(make_pair(new_uid, bullet));
		event_data.emplace_back(packet, packet->size); //다른 클라이언트에 중계

		send_create_bullet(new_uid, bullet);
		break;
	}
		
	default:
		break;
	}
};
void process_packet_vector() 
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
};
void game_logic(float fTime) 
{
	//Object Update
	for (auto hero = m_heros.begin(); hero != m_heros.end(); ++hero)
		hero->second.update(fTime);
	for (auto bullet = m_bullets.begin(); bullet != m_bullets.end(); ++bullet)
		bullet->second.update(fTime);

	//Collision Check
	for (auto h = m_heros.begin(); h != m_heros.end(); ++h)
	{
		const int& hero_uid = h->first;
		HERO& hero = h->second;
		//Hero Vs Wall
		for (int i = 0; i < m_walls.size(); ++i)
		{
			int count = 0;
			while ((hero.AABB.Intersects(m_walls[i])) && (count++ < 3))
				hero.half_back();
			if (count == 3) hero.roll_back();
		}

		//Hero vs Bullet
		for (auto b = m_bullets.begin(); b != m_bullets.end(); ++b)
		{
			const int& bullet_uid = b->first;
			BULLET& bullet = b->second;
			if (bullet.shooter == hero_uid) continue;
			if (hero.AABB.Intersects(bullet.AABB)) {
				hero.hp -= bullet.damage;
				send_sc_hit(hero_uid, bullet.damage);
				send_destroy_bullet(bullet_uid, bullet);
			}
		}
	}

	//Bullet vs Wall
	for (auto b = m_bullets.begin(); b != m_bullets.end(); ++b)
	{
		const int& bullet_uid = b->first;
		BULLET& bullet = b->second;
		for (int i = 0; i < m_walls.size(); ++i)
		{
			if (bullet.AABB.Intersects(m_walls[i])) {
				send_destroy_bullet(bullet_uid, bullet);
			}
		}
	}
};
void send_game_state() 
{
	//플레이어 정보 정리
	for (auto i = m_heros.begin(); i != m_heros.end(); ++i)
	{
		char uid = i->first;
		HERO& hero = i->second;
		send_player_info(uid, hero);
	}
	//총알 정보 정리
	for (auto b = m_bullets.begin(); b != m_bullets.end();)
	{
		char uid = b->first;
		BULLET& bullet = b->second;
		if (bullet.isDestroy == false) {
			send_bullet_info(uid, bullet);
			++b;
		}
		else {
			m_bullets.erase(b++);
		}
	}
	event_data.emplace_back(info_data.data, info_data.len);

	socket_lock.lock();
	for (auto& p : sockets) {
		send(p.second, (const char*)event_data.data, event_data.len, 0);
	}
	socket_lock.unlock();

	info_data.clear();
	event_data.clear();
};

void update(float fTime)
{
	process_packet_vector();
	game_logic(fTime);
	send_game_state();
};

int main()
{
	printf("Initializing Socket... ");
	//TCP 소켓 세팅 --- Listen까지 처리
	{
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listen_socket == INVALID_SOCKET) err_display("socket()");

		SOCKADDR_IN serverAddr;
		memset(&serverAddr, 0, sizeof(SOCKADDR_IN));

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(SERVER_PORT);

		int retval = ::bind(listen_socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
		if (retval == SOCKET_ERROR) err_display("bind()");

		retval = listen(listen_socket, SOMAXCONN);
		if (retval == SOCKET_ERROR) err_display("listen()");
	}
	printf("Done.\n");

	loadMap("map.txt");

	update_flag = CreateEvent(NULL, true, false, NULL);
	accept_flag = CreateEvent(NULL, true, false, NULL);

	vector<thread> threads;
	threads.emplace_back(recvFunc);

	Timer timer;
	timer.setFPS(30.0f);
	//Update 처리
	while (true)
	{
		if (cur_user == 0) {
			WaitForSingleObject(update_flag, INFINITE);
			ResetEvent(update_flag);
			timer.reset();

			m_heros.clear();
			m_bullets.clear();
			bullet_uid = 0;
		}

		//실제 로직
		float fTime = timer.tick();
		update(fTime);
	}

	for (size_t i = 0; i < threads.size(); ++i)
		threads[i].join();

	closesocket(listen_socket);
	WSACleanup();
}
