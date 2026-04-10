#include "Zombie.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>
#include <string>

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

// --- ZombieHead 實作 ---
ZombieHead::ZombieHead(float startX, float startY, float groundY) : m_Velocity({40.0f, 150.0f}) {
    m_Transform.translation = {startX, startY};
    m_ZIndex = 55;
    m_GroundY = groundY;
    auto frames = GetFramesFromFolder("resources/image/zombie/normal_zombie/die/ZombieHead");
    if (!frames.empty()) SetDrawable(std::make_shared<Util::Animation>(frames, false, 80, true));
}

void ZombieHead::Update(float dt) {
    if (!m_OnGround) {
        m_Velocity.y -= 800.0f * dt;
        m_Transform.translation += m_Velocity * dt;
        if (m_Transform.translation.y <= m_GroundY) {
            m_Transform.translation.y = m_GroundY;
            m_Velocity.y = -m_Velocity.y * 0.3f;
            if (std::abs(m_Velocity.y) < 30.0f) m_OnGround = true;
        }
    } else {
        m_DisappearTimer -= dt;
    }
}

// --- Zombie 實作 ---
Zombie::Zombie(float x, float y, Type type) : GameEntity("", 270.0f), m_Type(type) {
    m_Transform.translation = {x, y};
    m_ZIndex = 50;
    m_ArmorHP = (m_Type == Type::CONEHEAD) ? 370.0f : 0.0f;
    m_HP = 270.0f; // 強制初始化血量
    UpdateAnimation();
}

void Zombie::Update(float dt) {

    // 1. 如果已經進入 DEAD 狀態，只跑計時器，不跑任何邏輯
    if (m_CurrentState == State::DEAD) {
        if (m_DeathTimer > 0.0f) m_DeathTimer -= dt;
        return;
    }

    // 2. 斷頭流血與致死檢查 (核心：只要血量 <= 0 絕對要倒下)
    if (m_IsDecapitated || m_CurrentState == State::DYING) {
        m_HP -= 60.0f * dt;
    }

    if (m_HP <= 0.0f) {
        m_HP = 0.0f;
        LOG_DEBUG("Zombie HP is 0. Forcing DEAD state.");
        SetState(State::DEAD);
        return; // 切換到 DEAD 後立刻跳出，交給下一幀的 DEAD 邏輯處理
    }

    // 3. 移動邏輯
    if (m_CurrentState == State::WALKING || m_CurrentState == State::DYING) {
        m_Transform.translation.x -= m_Speed * dt;
    }
}

void Zombie::TakeDamage(int damage) {
    if (m_CurrentState == State::DEAD) return;

    float dmg = static_cast<float>(damage);

    // 裝甲處理
    if (m_ArmorHP > 0.0f) {
        m_ArmorHP -= dmg;
        if (m_ArmorHP <= 0.0f) {
            dmg = -m_ArmorHP;
            m_ArmorHP = 0.0f;
            m_Type = Type::NORMAL;
            UpdateAnimation();
        } else {
            dmg = 0.0f;
        }
    }

    // 本體處理
    if (dmg > 0.0f) {
        m_HP -= dmg;
        LOG_DEBUG("Zombie HP remaining: {}", m_HP);

        if (m_HP <= 0.0f) {
            m_HP = 0.0f;
            SetState(State::DEAD);
        } else if (m_HP < 90.0f && !m_IsDecapitated) {
            m_IsDecapitated = true;
            SetState(State::DYING);
        }
    }
}

void Zombie::SetState(State state) {
    LOG_DEBUG("SetState called: {} -> {}", (int)m_CurrentState, (int)state);
    if (m_CurrentState == state) return;
    if (m_CurrentState == State::DEAD) return;
    m_CurrentState = state;
    UpdateAnimation();
}

void Zombie::UpdateAnimation() {
    std::string path;
    int interval = 120;
    bool loop = true;

    if (m_CurrentState == State::DEAD) {
        path = "resources/image/zombie/normal_zombie/die/ZombieDie";
        interval = 150;
        auto frames = GetFramesFromFolder(path);
    }

    else if (m_CurrentState == State::DYING) {
        path = "resources/image/zombie/normal_zombie/die/ZombieLostHead";
    }
    else {
        if (m_Type == Type::CONEHEAD) {
            path = (m_CurrentState == State::EATING) ?
                   "resources/image/zombie/conehead_zombie/eat" :
                   "resources/image/zombie/conehead_zombie";
        } else {
            path = (m_CurrentState == State::EATING) ?
                   "resources/image/zombie/normal_zombie/eat" :
                   "resources/image/zombie/normal_zombie";
        }
    }

    auto frames = GetFramesFromFolder(path);
    LOG_DEBUG("UpdateAnimation called, state: {}, path: {}, frames: {}",
              (int)m_CurrentState, path, frames.size());

    if (frames.empty()) {
        LOG_ERROR("ANIMATION ERROR: Folder is empty or wrong path: {}", path);
        if (m_CurrentState == State::DEAD) m_DeathTimer = 1.0f;
        return;
    }

    // 🚩 reset = true 確保切換瞬間從第一幀開始
    m_Animation = std::make_shared<Util::Animation>(frames, loop, interval, true);
    m_Drawable = m_Animation;

    if (m_CurrentState == State::DEAD) {
        m_DeathTimer = (frames.size() * interval) / 1000.0f;
        LOG_DEBUG("Zombie DEAD: Playing animation, will be removed in {}s", m_DeathTimer);
    }
}

std::shared_ptr<ZombieHead> Zombie::SpawnHead() {
    if (m_IsDecapitated && !m_HeadHandedOut) {
        m_HeadHandedOut = true;
        return std::make_shared<ZombieHead>(m_Transform.translation.x + 10.0f, m_Transform.translation.y + 10.0f, m_Transform.translation.y - 100.0f);
    }
    return nullptr;
}