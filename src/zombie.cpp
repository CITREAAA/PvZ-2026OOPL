#include "Zombie.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

static std::vector<std::string> GetZombieFrames(const std::string& folder) {
    std::vector<std::string> paths;
    if (!fs::exists(folder)) {
        LOG_ERROR("找不到資料夾: {}", folder);
        return paths;
    }
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::string p = entry.path().string();
            std::replace(p.begin(), p.end(), '\\', '/');
            paths.push_back(p);
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

Zombie::Zombie(float x, float y) : GameEntity("", 270) {
    m_Transform.translation = {x, y};
    m_ZIndex = 50;
    m_AppearanceStage = 1; // 1:正常, 2:斷臂, 3:斷頭
    UpdateAnimation();
}

void Zombie::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    // 如果已經死透了，處理倒地計時並停止邏輯
    if (m_CurrentState == State::DEAD) {
        m_DeathTimer -= dt;
        return;
    }

    // 行走邏輯
    if (m_CurrentState == State::WALKING) {
        m_Transform.translation.x -= m_Speed * dt;
    }

    // --- 關鍵修復：血量階段判定 (只在變化的那一瞬間切換動畫) ---
    if (m_HP <= 0) {
        SetState(State::DEAD);
    }
    else if (m_HP < 90) {
        if (m_AppearanceStage != 3) { // 只有從 1或2 變成 3 的瞬間才執行
            m_AppearanceStage = 3;
            UpdateAnimation();
        }
    }
    else if (m_HP < 180) {
        if (m_AppearanceStage != 2) { // 只有從 1 變成 2 的瞬間才執行
            m_AppearanceStage = 2;
            UpdateAnimation();
        }
    }
}

void Zombie::TakeDamage(int damage) {
    if (m_CurrentState == State::DEAD) return;
    m_HP -= damage;
}

void Zombie::SetState(State state) {
    if (m_CurrentState == state) return; // 避免重複設定相同狀態導致動畫重置
    m_CurrentState = state;
    UpdateAnimation();
}

void Zombie::UpdateAnimation() {
    std::string path = "resources/image/zombie/";
    bool loop = true;

    if (m_CurrentState == State::DEAD) {
        path += "zombie_dead/ZombieDie";
        loop = false; // 死亡動畫播完就停在最後一幀
        m_ZIndex = 5;
    } else if (m_CurrentState == State::EATING) {
        path += "zombie_eat";
    } else {
        // WALKING 狀態下的外觀分支
        if (m_AppearanceStage == 3) {
            path += "zombie_dead/ZombieLostHead"; // 斷頭走
        } else if (m_AppearanceStage == 2) {
            path += "zombie_walk_lost_arm";      // 斷臂走
        } else {
            path += "zombie_walk";               // 正常走
        }
    }

    auto frames = GetZombieFrames(path);
    if (!frames.empty()) {
        // 建立新的動畫並更換
        m_Animation = std::make_shared<Util::Animation>(frames, loop, 120, true);
        m_Drawable = m_Animation;
    } else {
        LOG_ERROR("動畫路徑為空或找不到圖片: {}", path);
    }
}