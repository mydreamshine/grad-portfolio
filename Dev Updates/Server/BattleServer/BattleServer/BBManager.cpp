#include <fstream>
#include <string>
#include <Windows.h>
#include "BBManager.h"


BBManager& BBManager::instance()
{
    static BBManager* instance = new BBManager();
    return *instance;
}

void BBManager::load_bb()
{
    std::vector<std::string> world_target{
        "Cactus",
        "Barrel",
        "Fence",
        "Box",
        "Pilar",
        "cliff",
        "water",
        "Bone2"
    };

    std::string tempName;
    DirectX::BoundingBox tempBox;

    //Load Map.
    std::ifstream world{ "BB/Environment_BoundingBoxes.txt" };
    while (false == world.eof()) {
        world >> tempName
            >> tempBox.Center.x
            >> tempBox.Center.y
            >> tempBox.Center.z
            >> tempBox.Extents.x
            >> tempBox.Extents.y
            >> tempBox.Extents.z;

        if (tempName.find("Grass") != std::string::npos) {
            grass_bb.emplace_back(tempBox);
            continue;
        }
        else if (tempName.find("SpawnPoint") != std::string::npos) {
            spawn_points.emplace_back(tempBox.Center.x, tempBox.Center.y, tempBox.Center.z);
            continue;
        }

        for (const std::string& target : world_target) {
            if (tempName.find(target) == std::string::npos) continue;
            world_bb.emplace_back(tempBox);
        }
    }
    world.close();

    std::vector<std::string> character_target{
        "WARRIOR",
        "BERSERKER",
        "ASSASSIN",
        "PRIEST",
    };

    //Load Chacracter.
    std::ifstream hero{ "BB/Character_BoundingBoxes.txt" };
    while (false == hero.eof()) {
        hero >> tempName
            >> tempBox.Center.x
            >> tempBox.Center.y
            >> tempBox.Center.z
            >> tempBox.Extents.x
            >> tempBox.Extents.y
            >> tempBox.Extents.z;

        for (int i = 0; i < character_target.size(); ++i) {
            if (tempName.find(character_target[i]) == std::string::npos) continue;
            character_bb[i] = tempBox;
        }
    }
    hero.close();
}

void BBManager::load_config()
{
    std::ifstream ini{ config_path.c_str() };
    if (false == ini.is_open())
        gen_default_config();
    ini.close();

    wchar_t buffer[512];
    GetPrivateProfileString(L"SERVER", L"PORT", L"15600", buffer, 512, config_path.c_str());
    SERVER_PORT = std::stoi(buffer);

    GetPrivateProfileString(L"SERVER", L"NUM_THREADS", L"4", buffer, 512, config_path.c_str());
    NUM_THREADS = std::stoi(buffer);

    GetPrivateProfileString(L"SERVER", L"UPDATE_INTERVAL", L"30", buffer, 512, config_path.c_str());
    UPDATE_INTERVAL = std::stoi(buffer);

    GetPrivateProfileString(L"SERVER", L"MAX_ROOM", L"100", buffer, 512, config_path.c_str());
    MAX_ROOM = std::stoi(buffer);

    GetPrivateProfileString(L"DMMODE", L"MAX_PLAYER", L"4", buffer, 512, config_path.c_str());
    max_player = std::stoi(buffer);

    GetPrivateProfileString(L"DMMODE", L"PLAYTIME", L"180.0", buffer, 512, config_path.c_str());
    play_time = std::stof(buffer);

    GetPrivateProfileString(L"DMMODE", L"WIN_GOAL", L"5", buffer, 512, config_path.c_str());
    win_goal = std::stoi(buffer);

    GetPrivateProfileString(L"DMMODE", L"GAS_DECREASE_TIME", L"9.0", buffer, 512, config_path.c_str());
    gas_decrease_time = std::stof(buffer);

    GetPrivateProfileString(L"DMMODE", L"GAS_DECREASE_WIDTH", L"400", buffer, 512, config_path.c_str());
    gas_decrease_width = std::stoi(buffer);

    GetPrivateProfileString(L"DMMODE", L"GAS_DECREASE_HEIGHT", L"400", buffer, 512, config_path.c_str());
    gas_decrease_height = std::stoi(buffer);
}

BBManager::~BBManager()
{
}

void BBManager::gen_default_config()
{
    WritePrivateProfileString(L"SERVER", L"PORT", L"15600", config_path.c_str());
    WritePrivateProfileString(L"SERVER", L"NUM_THREADS", L"4", config_path.c_str());
    WritePrivateProfileString(L"SERVER", L"UPDATE_INTERVAL", L"30", config_path.c_str());
    WritePrivateProfileString(L"SERVER", L"MAX_ROOM", L"100", config_path.c_str());
    
    WritePrivateProfileString(L"DMMODE", L"MAX_PLAYER", L"4", config_path.c_str());
    WritePrivateProfileString(L"DMMODE", L"PLAYTIME", L"180.0", config_path.c_str());
    WritePrivateProfileString(L"DMMODE", L"WIN_GOAL", L"5", config_path.c_str());
    WritePrivateProfileString(L"DMMODE", L"GAS_DECREASE_TIME", L"9.0", config_path.c_str());
    WritePrivateProfileString(L"DMMODE", L"GAS_DECREASE_WIDTH", L"400", config_path.c_str());
    WritePrivateProfileString(L"DMMODE", L"GAS_DECREASE_HEIGHT", L"400", config_path.c_str());
}