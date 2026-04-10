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

// ==========================================
// ZombieHead 實作 (保持不變)
// ==========================================
ZombieHead::ZombieHead(float startX, float startY, float groundY) : m_Velocity({40.0f, 150.0f}) {
    m_Transform.translation = {startX, startY};
    m_ZIndex = 55;
    m_GroundY = groundY;

    auto frames = GetZombieFrames("C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/normal_zombie/die/ZombieHead");
    if (!frames.empty()) {
        SetDrawable(std::make_shared<Util::Animation>(frames, false, 80, true));
    }
}

void ZombieHead::Update(float dt) {
    if (!m_OnGround) {
        m_Velocity.y -= 800.0f * dt;
        m_Transform.translation += m_Velocity * dt;
        if (m_Transform.translation.y <= m_GroundY) {
            m_Transform.translation.y = m_GroundY;
            m_Velocity.y = -m_Velocity.y * 0.3f;
            m_Velocity.x *= 0.6f;
            if (m_Velocity.y < 30.0f) m_OnGround = true;
        }
    } else {
        m_Velocity.x *= std::max(0.0f, 1.0f - 5.0f * dt);
        m_Transform.translation.x += m_Velocity.x * dt;
        m_DisappearTimer -= dt;
    }
}

// ==========================================
// Zombie 實作
// ==========================================
Zombie::Zombie(float x, float y, Type type) : GameEntity("", 270), m_Type(type) {
    m_Transform.translation = {x, y};
    m_ZIndex = 50;

    // 🚩 設定裝甲血量
    if (m_Type == Type::CONEHEAD) {
        m_ArmorHP = 370.0f; // 三角錐有 370 點血量
    } else {
        m_ArmorHP = 0.0f;   // 普通殭屍沒裝甲
    }

    UpdateAnimation();
}

void Zombie::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    if (m_CurrentState == State::DEAD) {
        m_DeathTimer -= dt;
        return;
    }

    if (m_CurrentState == State::DYING) {
        m_HP -= 60.0f * dt;
        if (m_HP <= 0.0f) {
            m_HP = 0.0f;
            SetState(State::DEAD);
            return;
        }
    }

    if (m_CurrentState == State::WALKING || m_CurrentState == State::DYING) {
        m_Transform.translation.x -= m_Speed * dt;
    }
}

void Zombie::TakeDamage(int damage) {
    if (m_CurrentState == State::DEAD) return;

    float dmg = static_cast<float>(damage);

    // 🚩 1. 先扣除裝甲血量 (防具判定)
    if (m_ArmorHP > 0.0f) {
        m_ArmorHP -= dmg;
        if (m_ArmorHP <= 0.0f) {
            dmg = -m_ArmorHP; // 溢出的傷害由本體吸收
            m_ArmorHP = 0.0f;
            m_Type = Type::NORMAL; // 裝甲破碎，變回普通殭屍
            UpdateAnimation();     // 重新載入無頭盔的動畫
        } else {
            dmg = 0.0f; // 傷害被裝甲完全吸收
        }
    }

    // 🚩 2. 扣除本體血量
    if (dmg > 0.0f) {
        m_HP -= dmg;

        if (m_HP < 180.0f && !m_IsLostArm) {
            m_IsLostArm = true;
            // 可以在此加入斷臂圖片切換邏輯
        }

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
    if (m_CurrentState == State::DYING || m_CurrentState == State::DEAD) {
        if (state == State::EATING) return;
    }

    if (m_CurrentState == state) return;
    m_CurrentState = state;
    UpdateAnimation();
}

void Zombie::UpdateAnimation() {
    std::string path;
    int interval = 120;

    // 1. 徹底死亡
    if (m_CurrentState == State::DEAD) {
        path = "C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/normal_zombie/die/ZombieDie";
        interval = 160;
        // 🚩 絕對不改 m_ZIndex，維持 50，避免渲染樹 Bug 導致隱形
    }
    // 2. 垂死 (斷頭走路)
    else if (m_CurrentState == State::DYING) {
        path = "C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/normal_zombie/die/ZombieLostHead";
    }
    // 3. 正常與進食 (判斷種類)
    else {
        if (m_Type == Type::CONEHEAD) {
            if (m_CurrentState == State::EATING) {
                path = "C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/conehead_zombie/eat";
            } else {
                path = "C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/conehead_zombie";
            }
        } else {
            if (m_CurrentState == State::EATING) {
                path = "C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/normal_zombie/eat";
            } else {
                path = "C:/Users/YILI_Ume/Desktop/PvZ-2026OOPL/resources/image/zombie/normal_zombie";
            }
        }
    }

    auto frames = GetZombieFrames(path);

    // 🚩 終極防呆：如果資料夾沒圖片或路徑錯了，馬上印出超大警告！
    if (frames.empty()) {
        LOG_ERROR("===========================================");
        LOG_ERROR("【嚴重錯誤】找不到圖片！路徑：{}", path);
        LOG_ERROR("請檢查資料夾名稱是否拼錯，或裡面是不是空的！");
        LOG_ERROR("===========================================");

        // 給一個保底時間，避免瞬間當機或蒸發
        if (m_CurrentState == State::DEAD) m_DeathTimer = 1.0f;
        return;
    }

    // 🚩 終極破解法：第2和第4個參數全部給 true！(強制播放 + 強制循環)
    // 因為 m_DeathTimer 時間一到 App.cpp 就會把它刪掉，所以就算設循環也只會播一次！
    m_Animation = std::make_shared<Util::Animation>(frames, true, interval, true);
    m_Drawable = m_Animation;

    // 精確計算：圖片數量 * 播放間隔 = 剛好播完一輪的時間
    if (m_CurrentState == State::DEAD) {
        m_DeathTimer = (frames.size() * interval) / 1000.0f;
        LOG_DEBUG("【成功】載入死亡動畫，共 {} 張圖片，預計 {} 秒後刪除殭屍", frames.size(), m_DeathTimer);
    }
}

std::shared_ptr<ZombieHead> Zombie::SpawnHead() {
    if (m_IsDecapitated && !m_HeadHandedOut) {
        m_HeadHandedOut = true;
        float startX = m_Transform.translation.x + 10.0f;
        float startY = m_Transform.translation.y + 75.0f;
        float groundY = m_Transform.translation.y - 65.0f;
        return std::make_shared<ZombieHead>(startX, startY, groundY);
    }
    return nullptr;
}