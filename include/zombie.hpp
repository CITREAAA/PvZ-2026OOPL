#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"
#include <vector>
#include <string>
#include <memory>

class Zombie : public GameEntity {
public:
    enum class State { WALKING, EATING, DEAD };

    Zombie(float x, float y);
    void Update() override;
    
    // --- 生命與狀態控制 ---
    void TakeDamage(int damage);
    bool IsDead() const { return m_HP <= 0; }
    void SetState(State state);
    State GetState() const { return m_CurrentState; }

    // --- 座標與碰撞 ---
    glm::vec2 GetPosition() const { return m_Transform.translation; }

    // 👈 新增：取得脖子的位置（用來產生掉落的頭）
    glm::vec2 GetHeadPosition() const {
        return m_Transform.translation + glm::vec2{10.0f, 40.0f};
    }

    // --- 動態外觀判斷 ---
    // 👈 新增：讓 App 知道是否該噴出頭了
    bool ShouldSpawnHead() {
        if (m_AppearanceStage == 3 && !m_HeadDropped) {
            m_HeadDropped = true; // 標記已噴頭，下次呼叫回傳 false
            return true;
        }
        return false;
    }

    // 👈 確保死亡動畫播完才移除 (對應 ZombieDie)
    bool CanRemove() const { return m_CurrentState == State::DEAD && m_DeathTimer <= 0.0f; }

private:
    int m_HP = 270;
    float m_Speed = 14.4f;
    float m_DeathTimer = 2.0f; // 👈 預留 2 秒給 ZombieDie 動畫播放

    State m_CurrentState = State::WALKING;
    int m_AppearanceStage = 1; // 1:正常, 2:斷臂(<180), 3:斷頭(<90)
    bool m_HeadDropped = false; // 👈 紀錄頭是否已經掉落

    std::shared_ptr<Util::Animation> m_Animation;
    void UpdateAnimation(); 
};

#endif