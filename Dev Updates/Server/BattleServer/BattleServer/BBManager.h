#pragma once
#include <map>
#include <vector>
#include <DirectXCollision.h>

class BBManager
{
public:
    std::vector<DirectX::BoundingBox> world_bb;
    std::map <char, DirectX::BoundingBox> character_bb;

    static BBManager& instance();
    void load_bb();
    ~BBManager();
private:
    BBManager() {};
};

