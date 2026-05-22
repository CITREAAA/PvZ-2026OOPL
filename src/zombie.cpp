#include "Zombie.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include "Util/SFX.hpp"
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

    if (m_Type == Type::POLEVAULTER) {
        m_Transform.translation.y += 20.0f;
    }

    if (m_Type == Type::BUCKETHEAD) {
        m_ArmorHP = 1100.0f;
    } else if (m_Type == Type::CONEHEAD) {
        m_ArmorHP = 370.0f;
    } else if (m_Type == Type::POLEVAULTER) {
        m_ArmorHP = 0.0f;
        m_Speed = 32.0f;
    } else if (m_Type == Type::FOOTBALL) {
        m_ArmorHP = 0.0f;
        m_HP = 500.0f;
        m_Speed = 45.0f;
    }else if (m_Type == Type::SCREENDOOR) {
        m_ArmorHP = 1100.0f;
        m_Speed = 14.4f;
    } else {
        m_ArmorHP = 0.0f;
    }

    m_EatSFX = std::make_shared<Util::SFX>("resources/music/zombieChomp.wav");
    m_BaseAnimInterval = 120;
    UpdateAnimation();
}

void Zombie::Update(float dt) {
    // 1. 死亡狀態檢查
    if (m_CurrentState == State::DEAD) {
        if (m_DeathTimer > 0.0f) m_DeathTimer -= dt;
        return;
    }

    // 2. 處理減速狀態與計時
    if (m_IsSlowed) {
        m_SlowTimer -= dt;
        if (m_SlowTimer <= 0.0f) {
            m_IsSlowed = false;
            m_SlowTimer = 0.0f;
            if (m_Animation) m_Animation->SetInterval(m_BaseAnimInterval);
        } else {
            if (m_Animation) m_Animation->SetInterval(m_BaseAnimInterval * 2);
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
        SetState(State::DEAD);
        return;
    }

    // 5. 移動邏輯 (受減速影響)
    if (m_CurrentState == State::WALKING || m_CurrentState == State::DYING) {
        if (m_IsJumping) {
            m_JumpTimer -= dt;
            if (m_JumpTimer <= 0.0f) {
                m_IsJumping = false;
                m_IsPreparingJump = false;
                m_HasJumped = true;
                m_Transform.translation.x -= 130.0f;
                m_Speed = 14.4f;
                UpdateAnimation();
            }
        } else {
            float actualSpeed = m_IsSlowed ? (m_Speed * 0.5f) : m_Speed;
            m_Transform.translation.x -= actualSpeed * dt;
        }
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

            // 🚩 只有普通/三角錐/鐵桶會進入 DYING (掉頭) 狀態
            // 橄欖球跟撐竿跳會直接流血扣到 HP=0，跳過 DYING 進入 DEAD
            if (m_Type == Type::NORMAL || m_Type == Type::CONEHEAD || m_Type == Type::BUCKETHEAD) {
                SetState(State::DYING);
            }
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

    // 🚩 核心修正：全部強制 loop = true 規避記憶體破圖 bug，靠 m_DeathTimer 回收
    bool loop = true;

    // 確保撐竿跳殭屍大小正確
    if (m_Type == Type::POLEVAULTER) {
        m_Transform.scale = {0.35f, 0.35f};
    } else {
        m_Transform.scale = {1.0f, 1.0f};
    }

    // 1. DEAD 狀態 (死透倒下)
    if (m_CurrentState == State::DEAD) {
        if (m_Type == Type::FOOTBALL) {
            path = "resources/image/zombie/football_zombie/die";
            interval = 150;
        } else if (m_Type == Type::POLEVAULTER) {
            path = "resources/image/zombie/polevaulter_zombie/die";
            interval = 120;
        } else {
            path = "resources/image/zombie/normal_zombie/die/ZombieDie";
            interval = 150;
        }
    }
    // 2. DYING 狀態 (無頭走路)
    else if (m_CurrentState == State::DYING) {
        path = "resources/image/zombie/normal_zombie/die/ZombieLostHead";
    }
    // 3. 存活狀態 (EATING / WALKING)
    else {
        if (m_Type == Type::POLEVAULTER) {
            if (m_CurrentState == State::EATING) path = "resources/image/zombie/polevaulter_zombie/eat";
            else if (m_IsPreparingJump) path = "resources/image/zombie/polevaulter_zombie/beforejump";
            else if (m_HasJumped) path = "resources/image/zombie/polevaulter_zombie/afterjump";
            else path = "resources/image/zombie/polevaulter_zombie/run";
        }
        else if (m_Type == Type::FOOTBALL) {
            path = (m_CurrentState == State::EATING) ? "resources/image/zombie/football_zombie/eat" : "resources/image/zombie/football_zombie";
        }
        else if (m_Type == Type::BUCKETHEAD) {
            path = (m_CurrentState == State::EATING) ? "resources/image/zombie/buckethead_zombie/eat" : "resources/image/zombie/buckethead_zombie";
        }
        else if (m_Type == Type::CONEHEAD) {
            path = (m_CurrentState == State::EATING) ? "resources/image/zombie/conehead_zombie/eat" : "resources/image/zombie/conehead_zombie";
            // 🚩 新增：紗門殭屍的外觀判定
        }else if (m_Type == Type::SCREENDOOR) {
            if (m_ArmorHP > 0.0f) {
                // 紗門還在
                path = (m_CurrentState == State::EATING) ?
                       "resources/image/zombie/screendoor_zombie/eat" :
                       "resources/image/zombie/screendoor_zombie";
            } else {
                // 紗門被打爆了，外觀直接扒光變成普通殭屍！
                path = (m_CurrentState == State::EATING) ?
                       "resources/image/zombie/normal_zombie/eat" :
                       "resources/image/zombie/normal_zombie";
            }

        } else {
            path = (m_CurrentState == State::EATING) ? "resources/image/zombie/normal_zombie/eat" : "resources/image/zombie/normal_zombie";

        }
    }

    auto frames = GetFramesFromFolder(path);
    if (frames.empty()) {
        LOG_ERROR("ANIMATION ERROR: {}", path);
        return;
    }

    m_BaseAnimInterval = interval;
    int actualInterval = m_IsSlowed ? (interval * 2) : interval;

    m_Animation = std::make_shared<Util::Animation>(frames, loop, actualInterval, true);
    m_Drawable = m_Animation;

    // 精準計算倒下動畫的播放時間，播完剛好被 App.cpp 回收移除
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

void Zombie::StartJump() {
    if (m_Type != Type::POLEVAULTER) return;
    if (m_HasJumped) return;
    if (m_CurrentState == State::DEAD) return;

    m_IsPreparingJump = true;
    m_IsJumping = true;
    m_CurrentState = State::WALKING;
    m_JumpTimer = 5.2f;
    UpdateAnimation();
}