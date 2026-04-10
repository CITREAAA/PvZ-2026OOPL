#include "Zombie.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

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

// --- ZombieHead ---
ZombieHead::ZombieHead(float startX, float startY, glm::vec2 initialVelocity)
    : m_Velocity(initialVelocity) {
    m_Transform.translation = {startX, startY};
    m_ZIndex = 55;
    auto frames = GetZombieFrames("resources/image/zombie/normal_zombie/die/ZombieHead");
    if (!frames.empty()) {
        SetDrawable(std::make_shared<Util::Animation>(frames, false, 80, true));
    }
}

void ZombieHead::Update(float dt) {
    m_Velocity.y -= 3000.0f * dt;
    m_Transform.translation += m_Velocity * dt;
}

// --- Zombie ---
Zombie::Zombie(float x, float y) : GameEntity("", 270) {
    m_Transform.translation = {x, y};
    m_ZIndex = 50;
    UpdateAnimation();
}

// 🚩 修正：加上 float dt，並移除內部的 dt 宣告
void Zombie::Update(float dt) {
    if (m_CurrentState == State::DEAD) {
        if (m_DeathTimer > 0) m_DeathTimer -= dt;
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
        path = (m_AppearanceStage == 3) ? basePath + "/die/ZombieLostHead" : basePath;
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
        glm::vec2 vel = {40.0f, -200.0f};
        return std::make_shared<ZombieHead>(startX, startY, vel);
    }
    return nullptr;
}