#pragma once
#include <map>
#include <vector>
#include <string>
#include <DirectXCollision.h>
#include "..\Server\Common\Vector3d.h"

class BBManager
{
public:
    static BBManager& instance();
    void load_bb();
    void load_config();
    ~BBManager();

    //For Server.
    short SERVER_PORT{0};
    int NUM_THREADS{0};
    int MAX_ROOM{0};
    int UPDATE_INTERVAL{0};

    //For Games.
    int max_player{ 0 };
    float play_time{ 0 };
    int win_goal{ 0 };

    float gas_decrease_time{0};
    int gas_decrease_width{ 0 };
    int gas_decrease_height{ 0 };

    std::vector<Vector3d> spawn_points;
    std::vector<DirectX::BoundingBox> world_bb;
    std::vector<DirectX::BoundingBox> grass_bb;
    std::map <char, DirectX::BoundingBox> character_bb;
private:
    std::wstring config_path{ L".\\BATTLE_CONFIG.ini" };
    BBManager() {};
    void gen_default_config();
};

