#pragma once

#include "stdafx.h"
#include "ScnMgr.h"
#include "Dependencies\freeglut.h"
#include "..\\..\\server\server\PacketMgr.h"
#include <WinSock2.h>

ScnMgr::ScnMgr()
{
	//NW
	soc = -1;
	MYID = -1;
	m_state = state_title;

	m_Renderer = new Renderer(m_Width, m_Height);
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}

	textures[0] = m_Renderer->GenPngTexture("./Textures/bg.png");
	textures[1] = m_Renderer->GenPngTexture("./Textures/player.png");
	textures[2] = m_Renderer->GenPngTexture("./Textures/block.png");
	textures[3] = m_Renderer->GenPngTexture("./Textures/mushroom.png");
	bullettextures[0] = m_Renderer->GenPngTexture("./Textures/bullet1.png");
	bullettextures[1] = m_Renderer->GenPngTexture("./Textures/bullet2.png");
	bullettextures[2] = m_Renderer->GenPngTexture("./Textures/bullet3.png");
	winlose[0] = m_Renderer->GenPngTexture("./Textures/win_J.png");
	winlose[1] = m_Renderer->GenPngTexture("./Textures/lose_J.png");
	hpbar = m_Renderer->GenPngTexture("./Textures/hp.png");
	state_texture[0] = m_Renderer->GenPngTexture("./Textures/title_J.png");
	state_texture[1] = m_Renderer->GenPngTexture("./Textures/connect_J.png");
	state_texture[2] = m_Renderer->GenPngTexture("./Textures/connect_error_J.png");
	state_texture[3] = m_Renderer->GenPngTexture("./Textures/wait_J.png");
	item_texture[0] = m_Renderer->GenPngTexture("./Textures/item1.png");
	item_texture[1] = m_Renderer->GenPngTexture("./Textures/item2.png");
	item_texture[2] = m_Renderer->GenPngTexture("./Textures/item3.png");

	message_texture[0] = m_Renderer->GenPngTexture("./Textures/M_HI.png");
	message_texture[1] = m_Renderer->GenPngTexture("./Textures/M_BYE.png");
	message_texture[2] = m_Renderer->GenPngTexture("./Textures/M_NO.png");
	message_texture[3] = m_Renderer->GenPngTexture("./Textures/M_SRY.png");
	message_texture[4] = m_Renderer->GenPngTexture("./Textures/M_STAY.png");
	message_texture[5] = m_Renderer->GenPngTexture("./Textures/M_HELP.png");

	//button
	button[0][0] = m_Renderer->GenPngTexture("./Textures/Start_button.png");
	button[0][1] = m_Renderer->GenPngTexture("./Textures/On_Start_button.png");

	button[1][0] = m_Renderer->GenPngTexture("./Textures/Exit_button.png");
	button[1][1] = m_Renderer->GenPngTexture("./Textures/On_Exit_button.png");

	//Alive
	alive[0] = m_Renderer->GenPngTexture("./Textures/alive1.png");
	alive[1] = m_Renderer->GenPngTexture("./Textures/alive2.png");
	alive[2] = m_Renderer->GenPngTexture("./Textures/alive3.png");
	alive[3] = m_Renderer->GenPngTexture("./Textures/alive_many.png");

	//Add Unvisible Wall
	m_wall[0] = new Object();
	m_wall[0]->SetPos((m_Width+m_Height) / 200, 0);
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

	m_button[0] = new Object();
	m_button[0]->SetPos(180, -50);
	m_button[0]->SetVol(342 / 1.5, 87 / 1.5);

	m_button[1] = new Object();
	m_button[1]->SetPos(180, m_button[0]->m_pos.y - 87 / 1.5 - 5);
	m_button[1]->SetVol(342 / 1.5, 87 / 1.5);

	m_alive = new Object();
	m_alive->SetPos(300, 200);
	m_alive->SetVol(234 / 2, 106 / 2);

	for (auto& b : m_commonBullet) {
		b = new Object();
		b->m_visible = false;
	}
}
ScnMgr::~ScnMgr()
{
	if(m_Renderer)
		delete m_Renderer;
	m_Renderer = NULL;

	for (auto& b : m_commonBullet) {
		delete b;
	}
}

void ScnMgr::Update(float fTimeElapsed)
{
	for (auto& b : m_commonBullet)
		b->Update(fTimeElapsed);

	//플레이어 위치 보간
	for (auto& p_pair : m_players)
		p_pair.second.Update(fTimeElapsed);

	if (m_mouseLeft && m_players[MYID].CanShootBullet()) {
		int packet = make_packet_input(MYID, input_Mleft);
		send(soc, (const char*)& packet, sizeof(int), 0);
		send(soc, (const char*)& m_mousepos, sizeof(Vector2d), 0);
	}
}
void ScnMgr::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	switch (m_state)
	{
	case state_title:
	case state_connect:
	case state_connect_error:
	case state_wait:
		m_Renderer->DrawTextureRect(0, 0, 0, m_Width, -m_Height, 0, 1, 1, 1, 1, state_texture[m_state]);
		break;


	case state_play:
	case state_end:
		//UI
		m_Renderer->DrawTextureRect(m_alive->m_pos.x, m_alive->m_pos.y, 2, m_alive->m_vol.x, -m_alive->m_vol.y, 0, 1, 1, 1, 1, alive[alive_count]); //On

		//Draw Background
		m_Renderer->DrawTextureRect(0, 0, 0, 800, 600, 0, 1, 1, 1, 1, textures[0]);


		//Draw blocks
		for (auto& block : m_block)
			m_Renderer->DrawTextureRect(block->m_pos.x * 100, block->m_pos.y * 100, 0, block->m_vol.x * 100, block->m_vol.y * 100, 0, 1, 1, 1,1, textures[2]);

		//Draw Items
		for (auto& p_item : m_item)
		{
			auto& item = p_item.second;
			if (item.m_visible == false) continue;
			m_Renderer->DrawTextureRect(item.m_pos.x * 100, item.m_pos.y * 100, 0, 20, 20, 0, 1, 1, 1, 1, item_texture[item.type]);
			
		}

		//중립 총알 그리기
		for (auto& b : m_commonBullet) {
			if (b->m_visible == false) continue;
			m_Renderer->DrawTextureRect(b->m_pos.x * 100, b->m_pos.y * 100, 0, 5, 5, 0, 1, 1, 1, 1, bullettextures[0]);
		}

		for (auto& p : m_players) {
			auto& o = p.second;
			if (o.m_visible == false) continue;
			//Draw Bullets
			for (auto& b : o.bullets)
			{
				if (b.m_visible == false) continue;
				m_Renderer->DrawTextureRect(b.m_pos.x * 100, b.m_pos.y * 100, 0, 5, 5, 0, 1, 1, 1, 1, bullettextures[b.type]);
			}
			if (o.m_chat > -1 && o.emotion_time > 0.0f) {
				m_Renderer->DrawTextureRect(o.m_pos.x * 100, (o.m_pos.y + 0.6) * 100, 0,
					60, 60, 0, 0, 0, 0, 1, message_texture[o.m_chat - 1]);
			}
			//Draw HP
			float hp = o.m_hp / 100.0f;
			m_Renderer->DrawTextureRect(o.m_pos.x * 100, (o.m_pos.y + 0.19) * 100, 0, (o.m_vol.x * 100) * hp, 5, 0, o.m_color[0], o.m_color[1], o.m_color[2], o.m_color[3], hpbar);

			//Draw Character
			m_Renderer->DrawTextureRect(o.m_pos.x * 100, o.m_pos.y * 100, 0, o.m_vol.x * 100, -o.m_vol.y * 100, 0, o.m_color[0], o.m_color[1], o.m_color[2], o.m_color[3], textures[1]);
		}

		//Draw End Title
		if (m_state == state_end) {
			if (MYID != winner)
				m_Renderer->DrawTextureRect(0, 0, 0, m_Width, -m_Height, 0, 1, 1, 1, 1, winlose[1]);
			else if (MYID == winner)
				m_Renderer->DrawTextureRect(0, 0, 0, m_Width, -m_Height, 0, 1, 1, 1, 1, winlose[0]);
		}
		break;
	}


}

void ScnMgr::Init() {
	m_players.clear();
	for (auto& b : m_commonBullet) {
		b->m_visible = false;
	}
	alive_count = 2;
}

void ScnMgr::KeyDownInput(unsigned char key, int x, int y)
{
		int packet = -1;
		if (key == 'w' || key == 'W')
		{
			packet = make_packet_input(MYID, input_Wdown);
		}
		else if (key == 'a' || key == 'A')
		{
			packet = make_packet_input(MYID, input_Adown);
		}
		else if (key == 's' || key == 'S')
		{
			packet = make_packet_input(MYID, input_Sdown);
		}
		else if (key == 'd' || key == 'D')
		{
			packet = make_packet_input(MYID, input_Ddown);
		}

		else if (key == '1')
		{
			packet = make_packet_chat(MYID, 0x01);
		}
		else if (key == '2')
		{
			packet = make_packet_chat(MYID, 0x02);
		}
		else if (key == '3')
		{
			packet = make_packet_chat(MYID, 0x03);
		}
		else if (key == '4')
		{
			packet = make_packet_chat(MYID, 0x04);
		}
		else if (key == '5')
		{
			packet = make_packet_chat(MYID, 0x05);
		}
		else if (key == '6')
		{
			packet = make_packet_chat(MYID, 0x06);
		}

		if (packet != -1)
			send(soc, (const char*)&packet, sizeof(int), 0);
}
void ScnMgr::KeyUpInput(unsigned char key, int x, int y)
{
	if (m_state == state_play) {
		int packet = 0;
		if (key == 'w' || key == 'W')
		{
			packet = make_packet_input(MYID, input_Wup);
		}
		else if (key == 'a' || key == 'A')
		{
			packet = make_packet_input(MYID, input_Aup);
		}
		else if (key == 's' || key == 'S')
		{
			packet = make_packet_input(MYID, input_Sup);
		}
		else if (key == 'd' || key == 'D')
		{
			packet = make_packet_input(MYID, input_Dup);
		}
		if (packet != -1)
			send(soc, (const char*)&packet, sizeof(int), 0);
	}
}
void ScnMgr::MouseInput(int button, int state, int x, int y)
{
	if (m_state == state_play || m_state == state_title) {
		if (button == GLUT_LEFT_BUTTON) {
			if (state == GLUT_DOWN) {
				m_mouseLeft = true;
				m_mousepos.x = x; m_mousepos.y = y;
			}
			else if (state == GLUT_UP) m_mouseLeft = false;
		}
	}
}
void ScnMgr::MouseMotion(int x, int y)
{
	if (m_mouseLeft == true) {
		m_mousepos.x = x;
		m_mousepos.y = y;
	}
}

void ScnMgr::DoGarbageCollection()
{
	//delete bullets
	for (auto& p_pair : m_players)
	{
		auto& player = p_pair.second;
		if (player.m_visible == false) continue;

		for (auto& b : player.bullets)
		{
			if (b.m_visible == false) continue;
			if (b.m_vel.length() < FLT_EPSILON)
				b.m_visible = false;
		}
	}
}