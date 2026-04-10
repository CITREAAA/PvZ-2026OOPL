#include "Plant.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>
#include <random>

namespace fs = std::filesystem;

static std::vector<std::string> GetFramesFromFolder(const std::string& folderPath) {
    std::vector<std::string> paths;
    if (!fs::exists(folderPath)) return paths;

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

// --- Peashooter ---
Peashooter::Peashooter(float x, float y) : Plant({}, 270, 100) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    auto paths = GetFramesFromFolder("resources/image/peashooter");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 150, true);
        m_Drawable = m_Animation;
    }
}

void Peashooter::Update(float dt) {
    m_FireTimer += dt;
    if (m_FireTimer >= 1.45f) {
        m_ShouldFire = true;
        m_FireTimer = 0.0f;
    }
}

void Peashooter::Attack() { LOG_DEBUG("Peashooter: Pew!"); }

// --- Sunflower ---
Sunflower::Sunflower(float x, float y) : Plant({}, 300, 50) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    auto paths = GetFramesFromFolder("resources/image/sunflower");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 150, true);
        m_Drawable = m_Animation;
    }
    m_NextProductionTime = 5.0f; // 初始產出時間
}

void Sunflower::Update(float dt) {
    m_ProductionTimer += dt;
    if (m_ProductionTimer >= m_NextProductionTime) {
        m_ProducedSun = true;
        m_ProductionTimer = 0.0f;
        m_NextProductionTime = 24.0f; // 之後固定每 24 秒
    }
}

// --- Wallnut ---
Wallnut::Wallnut(float x, float y) : Plant({}, 4000, 50) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    auto paths = GetFramesFromFolder("resources/image/wallnut");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 150, true);
        m_Drawable = m_Animation;
    }
}

void Wallnut::Update(float dt) {
    int hp = GetHP();
    std::string newPath = "";

    if (hp <= 1333 && m_CurrentStage != 3) {
        newPath = "resources/image/wallnut";
        m_CurrentStage = 3;
    }
    else if (hp <= 2666 && hp > 1333 && m_CurrentStage != 2) {
        newPath = "resources/image/wallnut";
        m_CurrentStage = 2;
    }

    if (!newPath.empty()) {
        auto paths = GetFramesFromFolder(newPath);
        if (!paths.empty()) {
            m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
            m_Drawable = m_Animation;
        }
    }
}