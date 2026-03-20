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

// --- Peashooter 實作 (HP: 270, Cost: 100) ---
Peashooter::Peashooter(float x, float y)
    : Plant(GetFramesFromFolder("resources/image/peashooter"), 270, 100) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
}

void Peashooter::Update() {
    // 使用 GetDeltaTime() (假設回傳秒)
    m_FireTimer += static_cast<float>(Util::Time::GetDeltaTime());

    if (m_FireTimer >= 1.45f) { // 射速 1.36~1.5s
        m_ShouldFire = true;
        m_FireTimer = 0.0f;
    }
}

void Peashooter::Attack() {
    LOG_DEBUG("Peashooter: *Pew!*");
    // 具體發射子彈邏輯通常由 App 呼叫此處或判斷 CanFire
}

// --- Sunflower 實作 (HP: 300, Cost: 50) ---
Sunflower::Sunflower(float x, float y)
    : Plant(GetFramesFromFolder("resources/image/sunflower"), 300, 50) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;

    // 初次生產隨機時間
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(3.0f, 7.5f);
    m_NextProductionTime = dist(gen);
}

void Sunflower::Update() {
    m_ProductionTimer += static_cast<float>(Util::Time::GetDeltaTime());

    if (m_ProductionTimer >= m_NextProductionTime) {
        m_ProducedSun = true;
        m_ProductionTimer = 0.0f;
        m_NextProductionTime = 24.0f;
        LOG_DEBUG("Sunflower ready to produce sun!");
    }
}

void Sunflower::Attack() {
    // 向日葵的 Attack 通常是產陽光
}

// --- Wallnut 實作 ---
Wallnut::Wallnut(float x, float y)
    : Plant(GetFramesFromFolder("resources/image/wallnut"), 4000, 50) {
    // ^^^ 這裡會自動呼叫 Plant 的建構子，把路徑、4000HP、50金錢傳進去
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
}

void Wallnut::Update() {
    int hp = GetHP(); // 現在 Plant 有 GetHP 了，不會報錯

    if (hp <= 1333 && m_CurrentStage != 3) {
        auto paths = GetFramesFromFolder("resources/image/wallnut/3");
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
        m_CurrentStage = 3;
    }
    else if (hp <= 2666 && hp > 1333 && m_CurrentStage != 2) {
        auto paths = GetFramesFromFolder("resources/image/wallnut/2");
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
        m_CurrentStage = 2;
    }
}