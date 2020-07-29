#include <fstream>
#include <string>
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

BBManager::~BBManager()
{
}
