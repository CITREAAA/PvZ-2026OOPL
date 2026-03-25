#include "Plant.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>
#include <random>

namespace fs = std::filesystem;

// 輔助函式：自動掃描資料夾內的圖片並排序
static std::vector<std::string> GetFramesFromFolder(const std::string& folderPath) {
    std::vector<std::string> paths;
    if (!fs::exists(folderPath)) {
        LOG_ERROR("資料夾路徑不存在: {}", folderPath);
        return paths; // 回傳空向量
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
    : Plant({}, 270, 100) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    auto paths = GetFramesFromFolder("resources/image/peashooter");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
    }
}

void Peashooter::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    m_FireTimer += dt;
    if (m_FireTimer >= 1.45f) {
        m_ShouldFire = true;
        m_FireTimer = 0.0f;
    }
}

void Peashooter::Attack() { LOG_DEBUG("Peashooter: *Pew!*"); }

// --- Sunflower 實作 ---
Sunflower::Sunflower(float x, float y)
    : Plant({}, 300, 50) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    auto paths = GetFramesFromFolder("resources/image/sunflower");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(3.0f, 7.5f);
    m_NextProductionTime = dist(gen);
}

void Sunflower::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    m_ProductionTimer += dt;
    if (m_ProductionTimer >= m_NextProductionTime) {
        m_ProducedSun = true;
        m_ProductionTimer = 0.0f;
        m_NextProductionTime = 24.0f;
    }
}

void Sunflower::Attack() {}

// --- Wallnut 實作 (修正重點) ---
Wallnut::Wallnut(float x, float y)
    : Plant({}, 4000, 50) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;

    // 嘗試多個可能路徑，防止因為資料夾結構不同而崩潰
    std::vector<std::string> paths = GetFramesFromFolder("resources/image/wallnut/1");
    if (paths.empty()) {
        LOG_WARN("嘗試備用路徑: resources/image/wallnut");
        paths = GetFramesFromFolder("resources/image/wallnut");
    }

    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
    } else {
        LOG_ERROR("Wallnut 找不到任何圖片資源，請檢查 resources/image/wallnut 資料夾！");
        // 程式不再崩潰，只是會看不見堅果
    }
}

void Wallnut::Update() {
    int hp = GetHP();
    std::string newPath = "";

    if (hp <= 1333 && m_CurrentStage != 3) {
        newPath = "resources/image/wallnut/3";
        m_CurrentStage = 3;
    }
    else if (hp <= 2666 && hp > 1333 && m_CurrentStage != 2) {
        newPath = "resources/image/wallnut/2";
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