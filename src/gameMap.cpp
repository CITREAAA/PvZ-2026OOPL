#include "GameMap.hpp"
#include "plant.hpp"
#include <iostream>

#include "Util/Logger.hpp"
#include "Util/Time.hpp"

GameMap::GameMap(const std::string& path) {
    m_Drawable = std::make_shared<Util::Image>(path);
    m_ZIndex = -10;

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 9; ++j) {
            m_Grids[i][j] = nullptr;
        }
    }
}

bool GameMap::PlacePlant(int row, int col, std::shared_ptr<Plant> newPlant) {

    if (row < 0 || row >= 5 || col < 0 || col >= 9) return false;
    if (m_Grids[row][col] != nullptr) return false;

    m_Grids[row][col] = std::move(newPlant);
    this->AddChild(m_Grids[row][col]);

    m_Grids[row][col]->SetZIndex(5);

    float px = m_OffsetX + (col + 0.5f) * m_GridW;
    float py = m_OffsetY - (row + 0.5f) * m_GridH;

    m_Grids[row][col]->m_Transform.translation = {px, py};

    return true;
}

bool GameMap::GetGridIndex(glm::vec2 pos, int& row, int& col) {

    if (pos.x < m_OffsetX || pos.x > (m_OffsetX + 9 * m_GridW) ||
        pos.y > m_OffsetY || pos.y < (m_OffsetY - 5 * m_GridH)) {
        return false;
        }

    col = static_cast<int>((pos.x - m_OffsetX) / m_GridW);
    row = static_cast<int>((m_OffsetY - pos.y) / m_GridH);

    return true;
}

void GameMap::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (m_Grids[i][j] != nullptr) {
                if (m_Grids[i][j]->IsDead()) {
                    LOG_DEBUG("Map: Removing dead plant at {}, {}", i, j);
                    this->RemoveChild(m_Grids[i][j]);
                    m_Grids[i][j] = nullptr;
                    continue;
                }

                m_Grids[i][j]->Update(dt);
            }
        }
    }
}

glm::vec2 GameMap::CalculateGridCenter(int row, int col) {

    float centerX = m_OffsetX + (float)col * m_GridW + (m_GridW / 2.0f);
    float centerY = m_OffsetY - (float)row * m_GridH - (m_GridH / 2.0f);

    return {centerX, centerY};
}

std::shared_ptr<Plant> GameMap::GetPlant(int r, int c) {
    if (r < 0 || r >= 5 || c < 0 || c >= 9) {
        return nullptr;
    }
    return m_Grids[r][c];
}

void GameMap::RemovePlant(int r, int c) {
    if (r >= 0 && r < 5 && c >= 0 && c < 9) {
        if (m_Grids[r][c] != nullptr) {
            // 🚩 修正 2：手動移除時也要確保從渲染樹拔掉
            this->RemoveChild(m_Grids[r][c]);
            m_Grids[r][c] = nullptr;
        }
    }
}