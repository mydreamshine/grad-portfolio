#include "ROOM.h"
#include "CLIENT.h"

ROOM::~ROOM() {};
NGPROOM::NGPROOM()
{
	init();
}
NGPROOM::~NGPROOM()
{
	end();
}
bool NGPROOM::update(float elapsedTime)
{
	last_update_time = std::chrono::high_resolution_clock::now();
	ProcessPacket();
	bool retval = Update(elapsedTime);
	SendGameState();
	return retval;
}
void NGPROOM::init()
{
	clients = new CLIENT* [MATCHUP_NUM];

	player_num = 0;
	PlayerList.clear();
	while (!idQueue.empty()) idQueue.pop();
	idQueue.emplace(player1);
	idQueue.emplace(player2);
	idQueue.emplace(player3);
}
bool NGPROOM::regist(CLIENT* client)
{
	int idx = player_num++;
	clients[idx] = client;

	idlock.lock();
	int id = idQueue.front(); idQueue.pop();
	idlock.unlock();
	client->id = id;

	//플레이어 접속처리
	send_packet_player_id(client->socket, id);
	PlayerList[id] = Player();
	PlayerList[id].m_id = id;
	PlayerList[id].m_socket = static_cast<int>(client->socket);
	PlayerList[id].m_isConnect = true;
	PlayerList[id].SetPos(0.0f, 0.0f);
	PlayerList[id].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	PlayerList[id].SetVol(0.3f, 0.3f);
	PlayerList[id].SetVel(0.0f, 0.0f);
	PlayerList[id].SetMass(1.0f);
	PlayerList[id].SetFriction(0.6f);

	if (++idx == MATCHUP_NUM)
		return true;
	return false;
}
void NGPROOM::disconnect(CLIENT* client)
{
	PlayerList[client->id].m_isConnect = false;
}
void NGPROOM::process_packet(CLIENT* client, int ReceivedBytes)
{
	PacketReceiver(client, ReceivedBytes);
}
void NGPROOM::start()
{
	SendQueue.emplace_back(make_packet_game_start());
	Initialize();
}
void NGPROOM::end()
{
	DeleteObjects();
	//차후 서버단으로 이전
	for (int i=0;i<MATCHUP_NUM;++i)
	{
		SOCKET tmp = clients[i]->socket;
		clients[i]->socket = INVALID_SOCKET;
		clients[i]->room = nullptr;
		closesocket(tmp);
	}
	delete[] clients;
};
void NGPROOM::PacketReceiver(CLIENT* client, int ReceivedBytes)
{
	int retval = ReceivedBytes;

	int packet = 0;
	Vector2d addData;

	char* buf_pos = NULL;

	std::queue<int> tempQueue;
	std::queue<Vector2d> tempAddQueue;

	buf_pos = client->recv_over.data();
	while (retval > 0)
	{
		if (retval + client->saved_size >= client->need_size) {
			int copy_size = (client->need_size - client->saved_size);
			memcpy(client->savedPacket + client->saved_size, buf_pos, copy_size);
			buf_pos += copy_size;
			retval -= copy_size;

			switch (client->need_size) {
			case GENERAL_PACKET_SIZE:
				packet = *(int*)client->savedPacket;
				if (is_extend_packet_server(packet)) {
					client->need_size = EXTEND_PACKET_SIZE;
					client->saved_size = GENERAL_PACKET_SIZE;
					continue;
				}
				client->need_size = GENERAL_PACKET_SIZE;
				tempQueue.emplace(packet);
				break;

			case EXTEND_PACKET_SIZE:
				packet = *(int*)client->savedPacket;
				addData = *(Vector2d*)(client->savedPacket + GENERAL_PACKET_SIZE);
				tempQueue.emplace(packet);
				tempAddQueue.emplace(addData);
				client->need_size = GENERAL_PACKET_SIZE;
				break;
			}
			client->saved_size = 0;
		}
		else {
			memcpy(client->savedPacket + client->saved_size, buf_pos, retval);
			client->saved_size += retval;
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

