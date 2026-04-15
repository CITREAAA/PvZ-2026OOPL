#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"
#include "Util/SFX.hpp"
#include <memory>
#include <vector>
#include <string>
#include <glm/vec2.hpp>

// ==========================================
// 獨立的頭部物理物件
// ==========================================
class ZombieHead : public Util::GameObject {
public:
    ZombieHead(float startX, float startY, float groundY);
    void Update(float dt);
    bool CanRemove() const { return m_DisappearTimer <= 0.0f; }

private:
    glm::vec2 m_Velocity;
    float m_GroundY;
    bool m_OnGround = false;
    float m_DisappearTimer = 1.0f;
};

// ==========================================
// 殭屍本體物件
// ==========================================
class Zombie : public GameEntity {
public:
    enum class State { WALKING, EATING, DYING, DEAD };
    enum class Type { NORMAL, CONEHEAD, BUCKETHEAD };

    // 🚩 建構子加入種類參數，預設為普通殭屍
    Zombie(float x, float y, Type type = Type::NORMAL);

    float GetAttackPower() const { return m_AttackPower; }

    void TakeDamage(int damage);
    void Update(float dt);

    bool IsDead() const { return m_CurrentState == State::DEAD; }
    bool IsDying() const { return m_CurrentState == State::DYING; }

    void SetState(State state);
    State GetState() const { return m_CurrentState; }
    glm::vec2 GetPosition() const { return m_Transform.translation; }

    bool CanRemove() const { return m_CurrentState == State::DEAD && m_DeathTimer <= 0.0f; }

    std::shared_ptr<ZombieHead> SpawnHead();

private:
    Type m_Type;             // 殭屍種類
    float m_ArmorHP = 0.0f;  // 裝甲生命值 (三角錐 370)
    float m_HP = 270.0f;     // 本體生命值 (普通殭屍 270)
    float m_Speed = 14.4f;
    float m_AttackPower = 100.0f;

    float m_DeathTimer = 0.0f;

    std::shared_ptr<Util::SFX> m_EatSFX;
    std::shared_ptr<Util::SFX> m_GroanSFX;// 🎯 啃食音效
    float m_EatSoundTimer = 0.0f;

    State m_CurrentState = State::WALKING;

    bool m_IsLostArm = false;
    bool m_IsDecapitated = false;
    bool m_HeadHandedOut = false;

    std::shared_ptr<Util::Animation> m_Animation;
    void UpdateAnimation();
};

#endif // ZOMBIE_HPP