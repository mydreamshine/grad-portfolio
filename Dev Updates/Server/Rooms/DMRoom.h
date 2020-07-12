#pragma once
#include "ROOM.h"
#include "PACKET_VECTOR.h"
#include <mutex>
#include <map>

#include <DirectXCollision.h>

class DMRoom : public ROOM
{
public:
	DMRoom();
	virtual ~DMRoom();
	virtual void init();
	virtual bool regist(CLIENT* client);
	virtual void disconnect(CLIENT* client);
	virtual void start();
	virtual void end();
	virtual bool update(float elapsedTime);
	virtual void process_packet(CLIENT* client, int ReceivedBytes);

private:
	std::mutex packet_lock;
	PACKET_VECTOR packet_vector;
	PACKET_VECTOR packets;

	PACKET_VECTOR event_data; //���۵� �̺�Ʈ ��Ŷ(�÷��̾� ��, ������ ��)
	PACKET_VECTOR info_data;  //���۵� ��ġ���� ��Ŷ

	std::mutex socket_lock;
	std::map<int, SOCKET> sockets;
	std::map<int, HERO> m_heros;
	std::map<int, SKILL> m_bullets;
	std::vector<DirectX::BoundingBox> m_walls;

	void process_packet_vector();
	bool game_logic(float elapsedTime);
	void send_game_state();
};