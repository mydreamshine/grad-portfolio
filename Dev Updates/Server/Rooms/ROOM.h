#pragma once
#include <atomic>
#include <chrono>
#include <WS2tcpip.h>
#include <mutex>
#include <queue>
#include "Vector2d.h"

//Scene
#include "Object.h"
#include "Config.h"
#include <map>

//Packet
#include "PacketMgr.h"

#define CLIENT_EXIT 0
#define COMMON_BULLET_NUM 128
#define COMMON_BULLET_DAMAGE 1

class CLIENT;
class ROOM
{
public:
	virtual ~ROOM();
	virtual void init() = 0; ///< call with constructor, Init Game World
	virtual bool regist(CLIENT* client) = 0; ///< when client is connected, server call this function
	virtual void disconnect(CLIENT* client) = 0; ///< when client is disconnected, server call this function
	virtual void start() = 0; ///< when all user connected, server call this function
	virtual void end() = 0; ///< when game end, server will delete this object, call end with destructor
	virtual bool update(float elapsedTime) = 0; ///<each update, server call this function with elapsed Time
	virtual void process_packet(CLIENT* client, int ReceivedBytes) = 0; ///when recv data from client buffer, server will call this with client and its length

	std::chrono::high_resolution_clock::time_point last_update_time; ///< last update time stamp.
protected:
	int MATCHUP_NUM = 0; ///< max connect user num.
	std::atomic<int> player_num; ///< current user num
	CLIENT** clients; ///< array of CLIENT*, you need to get packet data from this
};


class NGPROOM : public ROOM
{
public:
	NGPROOM();
	virtual ~NGPROOM();
	virtual void init();
	virtual bool regist(CLIENT* client);
	virtual void disconnect(CLIENT* client);
	virtual void start();
	virtual void end();
	virtual bool update(float elapsedTime);
	virtual void process_packet(CLIENT* client, int ReceivedBytes);

private:
	//ID
	int MATCHUP_NUM = 3;
	std::queue<int> idQueue;
	std::mutex idlock;

	//Message Queue
	std::mutex RecvLock;
	std::queue<int> RecvQueue;
	std::queue<Vector2d> RecvAddData;
	std::vector<int> SendQueue;

	int m_Width = WINDOW_WIDTH;
	int m_Height = WINDOW_HEIGHT;
	Object* m_wall[4];
	Object* m_block[8];
	Item* m_item[MAX_ITEM];
	std::map<int, Player> PlayerList;
	Vector2d Initialpos[3]{ {-3, -1.5}, {+3, -1.5},{0, 1.5} };

	//총알분수
	Bullet m_commonBullet[COMMON_BULLET_NUM];
	int AddBullet(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef)
	{
		int idx = -1;
		for (int i = 0; i < COMMON_BULLET_NUM; ++i)
		{
			if (m_commonBullet[i].m_visible == false)
			{
				idx = i;
				break;
			}
		}

		if (idx == -1)
		{
			std::cout << "No more remaining object" << std::endl;
			return idx;
		}

		m_commonBullet[idx].m_visible = true;
		m_commonBullet[idx].SetPos(pos);
		m_commonBullet[idx].SetColor(r, g, b, a);
		m_commonBullet[idx].SetVol(vol);
		m_commonBullet[idx].SetVel(vel);
		m_commonBullet[idx].SetMass(mass);
		m_commonBullet[idx].SetFriction(fricCoef);

		return idx;
	}

	float shootTime = 0.05f;
	float remainTime = 0.0f;
	float rot = 3.0f;
	Vector2d m_pos{ 0, 0 };
	Vector2d m_dir[4]{ {1, 0},{-1, 0}, {0, 1}, {0, -1} };
	//총알분수
	void ShootBullet(float fTimeElapsed)
	{
		remainTime -= fTimeElapsed;
		if (remainTime > 0) return;


		float fAmountBullet = 8.0f, mass = 1.0f, fricCoef = 0.9f;

		for (auto& dir : m_dir)
		{
			dir.rotate(rot);
			Vector2d hVel = dir * fAmountBullet;
			Vector2d vol(0.05f, 0.05f);
			int idx = AddBullet(m_pos, vol, hVel, 1, 0, 0, 1, mass, fricCoef);
			m_commonBullet[idx].type = 1;
		}

		remainTime = shootTime;
	}

	//SendPacket ID, POS, 
	void send_packet_player_id(SOCKET client, const int& id)
	{
		int packet = 0;
		pTurnOn(packet, id);
		send(client, (const char*)&packet, sizeof(int), 0);
	}
	void reserve_packet_player_pos(const Player& mover, std::vector<int>& dat)
	{
		int packet = 0;
		pTurnOn(packet, p_obj);
		pTurnOn(packet, mover.m_id);
		pTurnOn(packet, obj_player);
		pTurnOn(packet, obj_position);

		int* p_pos = (int*)&mover.m_pos.x;
		dat.emplace_back(packet);
		dat.emplace_back(*p_pos++);
		dat.emplace_back(*p_pos++);
	}
	void reserve_packet_bullet_pos(const Player& shooter, const int& idx, std::vector<int>& dat)
	{
		int packet = 0;
		int bullet_type = shooter.bullets[idx].type << 24;

		pTurnOn(packet, p_obj);
		pTurnOn(packet, shooter.m_id);
		pTurnOn(packet, idx);
		pTurnOn(packet, bullet_type);
		pTurnOn(packet, obj_bullet);
		pTurnOn(packet, obj_position);

		int* p_pos = (int*)&shooter.bullets[idx].m_pos.x;
		dat.emplace_back(packet);
		dat.emplace_back(*p_pos++);
		dat.emplace_back(*p_pos++);
	}
	void reserve_packet_common_bullet_pos(const int& shooter_id, const int& idx, std::vector<int>& dat)
	{
		int packet = 0;

		pTurnOn(packet, p_obj);
		pTurnOn(packet, shooter_id);
		pTurnOn(packet, idx);
		pTurnOn(packet, obj_bullet);
		pTurnOn(packet, obj_position);

		int* p_pos = (int*)&m_commonBullet[idx].m_pos.x;
		dat.emplace_back(packet);
		dat.emplace_back(*p_pos++);
		dat.emplace_back(*p_pos++);
	}
	void reserve_packet_item_pos(const int& idx, std::vector<int>& dat)
	{
		int packet = 0;
		int item_type = m_item[idx]->type << 24;

		pTurnOn(packet, p_obj);
		pTurnOn(packet, idx);
		pTurnOn(packet, item_type);
		pTurnOn(packet, obj_item);
		pTurnOn(packet, obj_position);

		int* p_pos = (int*)&m_item[idx]->m_pos.x;
		dat.emplace_back(packet);
		dat.emplace_back(*p_pos++);
		dat.emplace_back(*p_pos++);
	}

	//Message Process
	void ProcessInputPacket(const int& packet, std::queue<Vector2d>& addData) {
		int sender = get_packet_player_num(packet);
		int input = get_packet_input(packet);
		switch (input)
		{
		case input_Wdown:
			PlayerList[sender].m_keyW = true;
			break;
		case input_Wup:
			PlayerList[sender].m_keyW = false;
			break;

		case input_Sdown:
			PlayerList[sender].m_keyS = true;
			break;
		case input_Sup:
			PlayerList[sender].m_keyS = false;
			break;

		case input_Adown:
			PlayerList[sender].m_keyA = true;
			break;
		case input_Aup:
			PlayerList[sender].m_keyA = false;
			break;

		case input_Ddown:
			PlayerList[sender].m_keyD = true;
			break;
		case input_Dup:
			PlayerList[sender].m_keyD = false;
			break;

		case input_Mleft:
			Vector2d Data = addData.front(); addData.pop();
			PlayerList[sender].ShootBullet(Data);
			break;
		}
	}

	void PacketReceiver(CLIENT* client, int ReceivedBytes);
	void ProcessPacket() {
		std::queue<int> cRecvQueue;
		std::queue<Vector2d> cAddData;

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
				break;

			case p_input:
				ProcessInputPacket(packet, cAddData);
				break;

			case p_obj:
				break;

			case p_event:
				SendQueue.emplace_back(packet);
				break;
			}
		}
	}

	//Game Logic
	void Initialize()
	{
		int count = 0;
		for (auto& p_pair : PlayerList)
			p_pair.second.m_pos = Initialpos[count++];

		//Add Unvisible Wall
		m_wall[0] = new Object();
		m_wall[0]->SetPos((m_Width + m_Height) / 200, 0);
		m_wall[0]->SetVol(m_Height / 100, m_Height / 100);
		m_wall[1] = new Object();
		m_wall[1]->SetPos(-(m_Width + m_Height) / 200, 0);
		m_wall[1]->SetVol(m_Height / 100, m_Height / 100);
		m_wall[2] = new Object();
		m_wall[2]->SetPos(0, (m_Width + m_Height) / 200);
		m_wall[2]->SetVol(m_Width / 100, m_Width / 100);
		m_wall[3] = new Object();
		m_wall[3]->SetPos(0, -(m_Width + m_Height) / 200);
		m_wall[3]->SetVol(m_Width / 100, m_Width / 100);

		//Add Block
		float volumn = 0.5f;
		m_block[0] = new Object();
		m_block[0]->SetPos(m_Width / 400, 0);
		m_block[0]->SetVol(volumn, volumn);
		m_block[1] = new Object();
		m_block[1]->SetPos(-m_Width / 400, 0);
		m_block[1]->SetVol(volumn, volumn);
		m_block[2] = new Object();
		m_block[2]->SetPos(0, m_Height / 400);
		m_block[2]->SetVol(volumn, volumn);
		m_block[3] = new Object();
		m_block[3]->SetPos(0, -m_Height / 400);
		m_block[3]->SetVol(volumn, volumn);

		m_block[4] = new Object();
		m_block[4]->SetPos(m_Width / 400, m_Height / 400 * 2);
		m_block[4]->SetVol(volumn, volumn);
		m_block[5] = new Object();
		m_block[5]->SetPos(-m_Width / 400, m_Height / 400 * 2);
		m_block[5]->SetVol(volumn, volumn);
		m_block[6] = new Object();
		m_block[6]->SetPos(m_Width / 400, -m_Height / 400 * 2);
		m_block[6]->SetVol(volumn, volumn);
		m_block[7] = new Object();
		m_block[7]->SetPos(-m_Width / 400, -m_Height / 400 * 2);
		m_block[7]->SetVol(volumn, volumn);

		//Add Item
		m_item[0] = new Item();
		m_item[0]->SetPos(m_Width / 400, m_Height / 400);
		m_item[0]->SetVol(0.2f, 0.2f);
		m_item[0]->m_visible = true;
		m_item[1] = new Item();
		m_item[1]->SetPos(m_Width / 400, -m_Height / 400);
		m_item[1]->SetVol(0.2f, 0.2f);
		m_item[1]->m_visible = true;
		m_item[2] = new Item();
		m_item[2]->SetPos(-m_Width / 400, -m_Height / 400);
		m_item[2]->SetVol(0.2f, 0.2f);
		m_item[2]->m_visible = true;
		m_item[3] = new Item();
		m_item[3]->SetPos(-m_Width / 400, m_Height / 400);
		m_item[3]->SetVol(0.2f, 0.2f);
		m_item[3]->m_visible = true;

		//아이템 위치 전송
		for (auto& p_pair : PlayerList)
			for (int i = 0; i < MAX_ITEM; i++)
				reserve_packet_item_pos(i, SendQueue);

		for (auto& bullet : m_commonBullet)
			bullet.m_visible = false;
	}
	void DoGarbageCollection()
	{
		//delete bullets
		for (auto& p_pair : PlayerList)
		{
			auto& player = p_pair.second;
			if (player.m_visible == false) continue;

			for (int i = 0; i < MAX_BULLET; ++i)
			{
				if (player.bullets[i].m_visible == false) continue;
				if (player.bullets[i].m_vel.length() < 0.00001f) {
					player.bullets[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player.m_id, i);
					SendQueue.emplace_back(packet);
				}
			}
		}

		for (int i = 0; i < 100; ++i)
		{
			if (m_commonBullet[i].m_visible == false) continue;
			if (m_commonBullet[i].m_vel.length() < 0.00001f) {
				m_commonBullet[i].m_visible = false;
				int packet = make_packet_destroy_bullet(player4, i);
				SendQueue.emplace_back(packet);
			}
		}
	}
	void DeleteObjects() {
		for (int i = 0; i < 4; i++) {
			delete m_wall[i];
			m_wall[i] = NULL;
		}
		for (int i = 0; i < MAX_ITEM; i++) {
			delete m_item[i];
			m_item[i] = NULL;
		}

		for (int i = 0; i < 8; i++) {
			delete m_block[i];
			m_block[i] = NULL;
		}
	}
	bool Update(float fTimeElapsed)
	{
		//플레이어 업데이트
		for (auto& p_player : PlayerList)
		{
			auto& player = p_player.second;
			//Move Player
			player.keyMove(fTimeElapsed);
			//총알 발사
			if (player.m_mouseLeft == true && player.CanShootBullet()) {
				player.ShootBullet(player.m_mousepos);
			}
			player.Update(fTimeElapsed);
		}

		//총알분수
		ShootBullet(fTimeElapsed);
		for (auto& b : m_commonBullet)
			b.Update(fTimeElapsed);

		//Collision Detect
		for (auto& p_pair : PlayerList)
		{
			if (p_pair.second.m_visible == false) continue;
			auto& player = p_pair.second;

			//플레이어, 벽 체크
			for (auto& wall : m_wall)
			{
				if (player.isOverlap(*wall)) {
					player.correctpos(*wall);
				}
			}

			//플레이어, 장애물 체크
			for (auto& block : m_block)
			{
				if (player.isOverlap(*block)) {
					player.correctpos(*block);
				}
			}

			//플레이어, 아이템 체크
			for (int i = 0; i < MAX_ITEM; ++i)
			{
				if (m_item[i]->m_visible == false) continue;
				if (player.isOverlap(*m_item[i])) {
					player.weapon = m_item[i]->type;
					m_item[i]->m_visible = false;
					int packet = make_packet_destroy_item(i);
					SendQueue.emplace_back(packet);
				}
			}

			//총알 충돌체크
			for (int i = 0; i < MAX_BULLET; ++i)
			{
				if (player.bullets[i].m_visible == false) continue;

				//총알, 벽체크
				for (auto& block : m_block)
				{
					if (block->isOverlap(player.bullets[i])) {
						player.bullets[i].m_visible = false;
						int packet = make_packet_destroy_bullet(player.m_id, i);
						SendQueue.emplace_back(packet);
						break;
					}
				}

				//총알, 플레이어 체크
				for (auto& p_other : PlayerList)
				{
					if (p_other.second.m_visible == false) continue;
					if (p_other.first == p_pair.first) continue;
					if (p_other.second.isOverlap(player.bullets[i])) {
						player.bullets[i].m_visible = false;
						int packet = make_packet_destroy_bullet(player.m_id, i);
						SendQueue.emplace_back(packet);
						if (p_other.second.getDamage(player.bullets[i])) {
							//사망처리
							p_other.second.die();
							int packet = make_packet_destroy_player(p_other.second.m_id);
							SendQueue.emplace_back(packet);
						}
						else {
							//데미지처리
							int packet = make_packet_hit_player(p_other.second.m_id, player.m_damage[player.bullets[i].type]);
							SendQueue.emplace_back(packet);
						}
					}
				}
			}

			//중립총알 체크
			for (int i = 0; i < COMMON_BULLET_NUM; i++)
			{
				if (m_commonBullet[i].m_visible == false) continue;
				//총알, 벽체크
				for (auto& block : m_block)
				{
					if (block->isOverlap(m_commonBullet[i])) {
						m_commonBullet[i].m_visible = false;
						int packet = make_packet_destroy_bullet(player4, i);
						SendQueue.emplace_back(packet);
						break;
					}
				}

				//총알, 플레이어 체크
				for (auto& p_other : PlayerList)
				{
					if (p_other.second.m_visible == false) continue;
					if (p_other.second.isOverlap(m_commonBullet[i])) {
						m_commonBullet[i].m_visible = false;
						int packet = make_packet_destroy_bullet(player4, i);
						SendQueue.emplace_back(packet);
						if (p_other.second.getDamage(COMMON_BULLET_DAMAGE)) {
							//사망처리
							p_other.second.die();
							int packet = make_packet_destroy_player(p_other.second.m_id);
							SendQueue.emplace_back(packet);
						}
						else {
							//데미지처리
							int packet = make_packet_hit_player(p_other.second.m_id, COMMON_BULLET_DAMAGE);
							SendQueue.emplace_back(packet);
						}
					}
				}

			}

		}

		//종료 검사
		int live_count = 0;
		int last_liver = 0;
		for (auto& p_player : PlayerList) {
			if (p_player.second.m_visible == true) {
				++live_count;
				last_liver = p_player.second.m_id;
			}
		}

		if (live_count <= 1) {
			int packet = make_packet_game_end(last_liver);
			SendQueue.emplace_back(packet);
			return true;
		}

		DoGarbageCollection();
		return false;
	}
	void SendGameState() {
		for (auto& p_other_player : PlayerList)
		{
			auto& other_player = p_other_player.second;
			if (other_player.m_visible == false) continue;
			reserve_packet_player_pos(other_player, SendQueue);
			for (int i = 0; i < MAX_BULLET; ++i)
				if (other_player.bullets[i].m_visible)
					reserve_packet_bullet_pos(other_player, i, SendQueue);
		}

		for (int i = 0; i < COMMON_BULLET_NUM; ++i) {
			if (m_commonBullet[i].m_visible)
				reserve_packet_common_bullet_pos(player4, i, SendQueue);
		}

		for (auto& p_player : PlayerList)
		{
			if (p_player.second.m_isConnect == true)
				send(p_player.second.m_socket, (const char*)SendQueue.data(), SendQueue.size() * sizeof(int), 0);
		}

		SendQueue.clear();
	}
};