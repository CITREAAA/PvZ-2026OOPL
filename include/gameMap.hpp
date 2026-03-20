#ifndef GAME_MAP_HPP
#define GAME_MAP_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Plant.hpp"
#include <vector>
#include <memory>
#include <string>

class GameMap : public Util::GameObject {
private:
    std::shared_ptr<Plant> m_Grids[5][9];

    const float m_OffsetX = -430.0f;
    const float m_OffsetY = 230.0f;
    const float m_GridW = 87.0f;
    const float m_GridH = 100.0f;

public:

    GameMap(const std::string& path);

    bool PlacePlant(int row, int col, std::shared_ptr<Plant> newPlant);

    bool GetGridIndex(glm::vec2 pos, int& row, int& col);

    void Update() ;

    std::shared_ptr<Plant> GetPlant(int r, int c);

    glm::vec2 CalculateGridCenter(int row, int col);
};

#endif