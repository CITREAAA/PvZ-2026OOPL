#include "Plant.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>
#include <random>

namespace fs = std::filesystem;

static std::vector<std::string> GetFramesFromFolder(const std::string& folderPath) {
    std::vector<std::string> paths;
    if (!fs::exists(folderPath)) {
        LOG_ERROR("Folder not found: {}", folderPath);
        return paths;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                std::string p = entry.path().string();
                std::replace(p.begin(), p.end(), '\\', '/');
                paths.push_back(p);
            }
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

// --- Peashooter 實作 ---
Peashooter::Peashooter(float x, float y)
    : Plant(GetFramesFromFolder("resources/image/peashooter"), 300, 100) { // HP: 300, Cost: 100
    m_Transform.translation = {x, y};
    m_Transform.scale = {1.0f, 1.0f};
    m_ZIndex = 10;
}

void Peashooter::Update() {
    m_FireTimer += static_cast<float>(Util::Time::GetDeltaTime());

    // 每 1.5 秒發射一顆
    if (m_FireTimer >= 1.5f) {
        m_ShouldFire = true;
        m_FireTimer = 0.0f;
    }
}

void Peashooter::Attack() {
    m_ShouldFire = true;
    LOG_DEBUG("Peashooter: *Pew!*");
}

// --- Sunflower 實作 ---
Sunflower::Sunflower(float x, float y)
    : Plant(GetFramesFromFolder("resources/image/sunflower"), 300, 50) { // HP: 300, Cost: 50
    m_Transform.translation = {x, y};
    m_Transform.scale = {1.0f, 1.0f};
    m_ZIndex = 10;

    // 設定第一次生產時間：3~12秒
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(3.0f, 12.0f);
    m_NextProductionTime = dist(gen);
}

void Sunflower::Update() {
    m_ProductionTimer += static_cast<float>(Util::Time::GetDeltaTime());

    if (m_ProductionTimer >= m_NextProductionTime) {
        m_ProducedSun = true; // 標記為可以生產，交由 App 產生 Sun 物件

        // 重設下次生產時間：23~25秒
        m_ProductionTimer = 0.0f;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(23.0f, 25.0f);
        m_NextProductionTime = dist(gen);

        LOG_DEBUG("Sunflower at ({}, {}) is ready to produce sun!",
                  m_Transform.translation.x, m_Transform.translation.y);
    }
}

void Sunflower::Attack() {
}