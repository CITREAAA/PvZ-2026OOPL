#include "Zombie.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// 讓殭屍與頭部共用這個讀取圖片的函式
static std::vector<std::string> GetZombieFrames(const std::string& folder) {
    std::vector<std::string> paths;
    if (!fs::exists(folder)) return paths;
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

// ==========================================
// ZombieHead 實作區域 (頭部物理)
// ==========================================
ZombieHead::ZombieHead(float startX, float startY, glm::vec2 initialVelocity)
    : m_Velocity(initialVelocity) {

    m_Transform.translation = {startX, startY};
    m_ZIndex = 55; // 讓頭顯示在身體前面一點

    auto frames = GetZombieFrames("resources/image/zombie/normal_zombie/die/ZombieHead");
    if (!frames.empty()) {
        SetDrawable(std::make_shared<Util::Animation>(frames, false, 80, true));
    }
}

void ZombieHead::Update(float dt) {
    // 🎯 極限重力：加碼到 3000.0f，讓向下拉扯的力量變得極大
    m_Velocity.y -= 3000.0f * dt;

    // 物理：根據目前速度更新座標
    m_Transform.translation += m_Velocity * dt;
}

// ==========================================
// Zombie 實作區域 (本體邏輯)
// ==========================================
Zombie::Zombie(float x, float y) : GameEntity("", 270) {
    m_Transform.translation = {x, y};
    m_ZIndex = 50;
    UpdateAnimation();
}

void Zombie::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    if (m_CurrentState == State::DEAD) {
        m_DeathTimer -= dt;
        return;
    }

    if (m_CurrentState == State::WALKING) {
        m_Transform.translation.x -= m_Speed * dt;
    }

    if (m_HP <= 0) {
        SetState(State::DEAD);
    }
    else if (m_HP < 90 && m_AppearanceStage != 3) {
        m_AppearanceStage = 3;
        UpdateAnimation();
    }
}

void Zombie::TakeDamage(int damage) {
    if (m_CurrentState != State::DEAD) m_HP -= damage;
}

void Zombie::SetState(State state) {
    if (m_CurrentState == state) return;
    m_CurrentState = state;
    if (m_CurrentState == State::DEAD) m_DeathTimer = 1.6f;
    UpdateAnimation();
}

void Zombie::UpdateAnimation() {
    std::string basePath = "resources/image/zombie/normal_zombie";
    std::string path;
    bool loop = true;

    if (m_CurrentState == State::DEAD) {
        path = basePath + "/die/ZombieDie";
        loop = false;
        m_ZIndex = 5;
    } else if (m_CurrentState == State::EATING) {
        path = basePath + "/eat";
    } else {
        if (m_AppearanceStage == 3) {
            path = basePath + "/die/ZombieLostHead";
        } else {
            path = basePath;
        }
    }

    auto frames = GetZombieFrames(path);
    if (!frames.empty()) {
        m_Animation = std::make_shared<Util::Animation>(frames, loop, 120, true);
        m_Drawable = m_Animation;
    }
}

std::shared_ptr<ZombieHead> Zombie::SpawnHead() {
    if (m_AppearanceStage == 3 && !m_HeadDropped) {
        m_HeadDropped = true;

        float startX = m_Transform.translation.x + 80.0f;
        float startY = m_Transform.translation.y + 75.0f;

        // 🎯 極限初速度：
        // X = +40.0f (輕微往右飄出)
        // Y = -200.0f (給予負數！讓它一出生就是往下砸的狀態)
        glm::vec2 vel = {40.0f, -200.0f};

        return std::make_shared<ZombieHead>(startX, startY, vel);
    }
    return nullptr;
}