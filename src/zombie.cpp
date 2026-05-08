#include "Zombie.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include "Util/SFX.hpp"
#include "Util/Color.hpp"
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
    if (m_Type == Type::BUCKETHEAD) {
        m_ArmorHP = 1100.0f;
    } else if (m_Type == Type::CONEHEAD) {
        m_ArmorHP = 370.0f;
    } else {
        m_ArmorHP = 0.0f;
    }
    m_EatSFX = std::make_shared<Util::SFX>("resources/music/zombieChomp.wav");
    m_HP = 270.0f;
    m_BaseAnimInterval = 120; // 🚩 設定基準動畫速度
    UpdateAnimation();
}

void Zombie::Update(float dt) {
    // 1. 死亡狀態檢查
    if (m_CurrentState == State::DEAD) {
        if (m_DeathTimer > 0.0f) m_DeathTimer -= dt;
        return;
    }

    // 🚩 2. 處理減速狀態與計時
    if (m_IsSlowed) {
        m_SlowTimer -= dt;
        if (m_SlowTimer <= 0.0f) {
            m_IsSlowed = false;
            m_SlowTimer = 0.0f;
            // 恢復原本顏色與動畫速度
            if (m_Animation) {
                m_Animation->SetInterval(m_BaseAnimInterval);
            }
        } else {
            if (m_Animation) {
                m_Animation->SetInterval(m_BaseAnimInterval * 2);
            }
        }
    }

    // 3. 處理啃食音效
    if (m_CurrentState == State::EATING) {
        m_EatSoundTimer -= dt;
        if (m_EatSoundTimer <= 0.0f) {
            if (m_EatSFX) m_EatSFX->Play();
            m_EatSoundTimer = 0.6f;
        }
    } else {
        m_EatSoundTimer = 0.0f;
    }

    // 4. 致死與流血邏輯
    if (m_IsDecapitated || m_CurrentState == State::DYING) {
        m_HP -= 60.0f * dt;
    }

    if (m_HP <= 0.0f) {
        m_HP = 0.0f;
        LOG_DEBUG("Zombie DEAD.");
        SetState(State::DEAD);
        return;
    }

    // 5. 移動邏輯 (受減速影響)
    if (m_CurrentState == State::WALKING || m_CurrentState == State::DYING) {
        float actualSpeed = m_IsSlowed ? (m_Speed * 0.5f) : m_Speed;
        m_Transform.translation.x -= actualSpeed * dt;
    }
}

void Zombie::TakeDamage(int damage) {
    if (m_CurrentState == State::DEAD) return;
    float dmg = static_cast<float>(damage);

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

    if (dmg > 0.0f) {
        m_HP -= dmg;
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
    if (m_CurrentState == state || m_CurrentState == State::DEAD) return;
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
        loop = false; // 死亡動畫不循環
    }
    else if (m_CurrentState == State::DYING) {
        path = "resources/image/zombie/normal_zombie/die/ZombieLostHead";
    }
    else {
        if (m_Type == Type::BUCKETHEAD) {
            path = (m_CurrentState == State::EATING) ?
                   "resources/image/zombie/buckethead_zombie/eat" :
                   "resources/image/zombie/buckethead_zombie";
        }
        else if (m_Type == Type::CONEHEAD) {
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
    if (frames.empty()) {
        LOG_ERROR("ANIMATION ERROR: {}", path);
        return;
    }

    // 更新基準間隔
    m_BaseAnimInterval = interval;

    // 🚩 若在減速狀態下切換動畫，立即套用兩倍間隔
    int actualInterval = m_IsSlowed ? (interval * 2) : interval;

    m_Animation = std::make_shared<Util::Animation>(frames, loop, actualInterval, true);
    m_Drawable = m_Animation;

    if (m_CurrentState == State::DEAD) {
        m_DeathTimer = (frames.size() * interval) / 1000.0f;
    }
}

std::shared_ptr<ZombieHead> Zombie::SpawnHead() {
    if (m_IsDecapitated && !m_HeadHandedOut) {
        m_HeadHandedOut = true;
        return std::make_shared<ZombieHead>(m_Transform.translation.x + 10.0f, m_Transform.translation.y + 10.0f, m_Transform.translation.y - 100.0f);
    }
    return nullptr;
}

void Zombie::SlowDown(float duration) {
    m_IsSlowed = true;
    m_SlowTimer = duration;
    if (m_Animation) {
        m_Animation->SetInterval(m_BaseAnimInterval * 2);
    }
}