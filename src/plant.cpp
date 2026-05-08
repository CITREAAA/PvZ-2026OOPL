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
            std::string p = entry.path().string();
            std::replace(p.begin(), p.end(), '\\', '/');
            paths.push_back(p);
        }
    }

    std::sort(paths.begin(), paths.end(), [](const std::string& a, const std::string& b) {
        auto get_num = [](const std::string& s) {
            std::string stem = fs::path(s).stem().string();
            size_t last_num = stem.find_last_of("0123456789");
            if (last_num == std::string::npos) return 0;
            size_t first_num = stem.find_last_not_of("0123456789", last_num);
            if (first_num == std::string::npos) return std::stoi(stem.substr(0, last_num + 1));
            return std::stoi(stem.substr(first_num + 1, last_num - first_num));
        };
        return get_num(a) < get_num(b);
    });
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

// 🚩 補齊 GetType
Plant::Type Peashooter::GetType() const { return Plant::Type::PEASHOOTER; }


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

// 🚩 補齊 GetType
Plant::Type Sunflower::GetType() const { return Plant::Type::SUNFLOWER; }


// --- Wallnut ---
Wallnut::Wallnut(float x, float y) : Plant({}, 4000, 50) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    m_CurrentStage = 1; // 確保初始化階段
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
        m_CurrentStage = 3;
    }
    else if (hp <= 2666 && hp > 1333 && m_CurrentStage != 2) {
        m_CurrentStage = 2;
    }

    if (!newPath.empty()) {
        auto paths = GetFramesFromFolder(newPath);
        if (!paths.empty()) {
            m_Animation = std::make_shared<Util::Animation>(paths, true, 150, true);
            m_Drawable = m_Animation;
        }
    }
}

Plant::Type Wallnut::GetType() const { return Plant::Type::WALLNUT; }

// --- PotatoMine ---
PotatoMine::PotatoMine(float x, float y) : Plant({}, 300, 25) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    m_State = MineState::UNDERGROUND;
    m_LocalTimer = 0.0f;

    // 初始狀態：地底
    auto paths = GetFramesFromFolder("resources/image/potatomine/underground");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 150, true);
        SetDrawable(m_Animation);
    }
}

void PotatoMine::Update(float dt) {
    m_LocalTimer += dt;

    if (m_State == MineState::UNDERGROUND) {
        if (m_LocalTimer >= 16.06f) {
            m_State = MineState::READY;
            m_LocalTimer = 0.0f;

            // 切換成準備好的動畫 (1-8號圖)
            auto paths = GetFramesFromFolder("resources/image/potatomine"); // 假設1-8就在這層
            if (!paths.empty()) {
                m_Animation = std::make_shared<Util::Animation>(paths, true, 200, true);
                SetDrawable(m_Animation);
            }
        }
    }
    else if (m_State == MineState::EXPLODING) {
        if (m_LocalTimer >= 0.5f) {
            TakeDamage(9999);
        }
    }
}

void PotatoMine::Trigger() {
    if (m_State == MineState::READY) {
        m_State = MineState::EXPLODING;
        m_LocalTimer = 0.0f;

        auto paths = GetFramesFromFolder("resources/image/potatomine/explode");
        if (!paths.empty()) {
            m_Animation = std::make_shared<Util::Animation>(paths, false, 500, false); // 爆炸通常不循環
            SetDrawable(m_Animation);
        }
    }
}

Plant::Type PotatoMine::GetType() const { return Plant::Type::MINE; }

// --- SnowPea ---
SnowPea::SnowPea(float x, float y) : Plant({}, 270, 175) {
    m_Transform.translation = {x, y};
    m_ZIndex = 10;
    // 假設動畫資源放在 resources/image/snowpea
    auto paths = GetFramesFromFolder("resources/image/snowpea");
    if (!paths.empty()) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 150, true);
        m_Drawable = m_Animation;
    }
}

void SnowPea::Update(float dt) {
    m_FireTimer += dt;
    // 雪花豌豆射速通常與普通豌豆一致 (約 1.45 秒)
    if (m_FireTimer >= 1.45f) {
        m_ShouldFire = true;
        m_FireTimer = 0.0f;
    }
}

Plant::Type SnowPea::GetType() const { return Plant::Type::SNOWPEA; }


// --- CherryBomb ---
CherryBomb::CherryBomb(float x, float y) : Plant({}, 300, 150) {
    m_Transform.translation = {x, y};
    m_ZIndex = 65; // 層級提高，確保蓋在殭屍上方

    auto paths = GetFramesFromFolder("resources/image/cherrybomb");
    if (!paths.empty()) {
        // 1秒後要爆炸，共有8幀，所以每幀間隔 1000ms / 8 = 125ms
        // 設定不循環播放 loop = false
        m_Animation = std::make_shared<Util::Animation>(paths, false, 125, true);
        SetDrawable(m_Animation);
    }
}

// 在 Plant.cpp 中
void CherryBomb::Update(float dt) {
    m_Timer += dt;

    // 🚩 原本這裡有 TakeDamage(9999)，把它刪除
    // if (m_Timer >= 1.2f) { TakeDamage(9999); }

    // 🚩 改成在 1.0 秒爆炸瞬間，把圖片換成靜態的爆炸圖 (第8幀)
    // 這樣它就不會因為動畫結束而變成透明或消失
    if (m_Timer >= 1.0f && !m_Exploded) {
        m_Exploded = true;
        if (m_Animation) {
            // 🚩 讓它定格在最後一幀 (第 8 幀，索引值是 7)
            m_Animation->SetCurrentFrame(m_Animation->GetFrameCount() - 1);

            // 🚩 如果沒有 Stop()，我們把幀間隔設成 100 萬毫秒，它就會看起來像停住了
            m_Animation->SetInterval(1000000);
        }
    }

    // 🚩 2.0 秒後徹底移除
    if (m_Timer >= 2.0f) {
        m_HP = 0; // 徹底死亡，讓 App.cpp 移除它
    }
}

Plant::Type CherryBomb::GetType() const { return Plant::Type::CHERRYBOMB; }