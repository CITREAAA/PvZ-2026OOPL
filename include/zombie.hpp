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

class Zombie : public GameEntity {
public:
    enum class State { WALKING, EATING, DYING, DEAD };
    enum class Type { NORMAL, CONEHEAD, BUCKETHEAD, POLEVAULTER, FOOTBALL,SCREENDOOR };

    Zombie(float x, float y, Type type = Type::NORMAL);

    float GetAttackPower() const { return m_AttackPower; }

    void TakeDamage(int damage);
    void Update(float dt);
    void SlowDown(float duration);

    // 🚩 修正：如果已經在倒下了，讓 App.cpp 的戰鬥/吃植物邏輯放過牠
    bool IsDead() const { return m_CurrentState == State::DEAD; }
    bool IsDying() const { return m_CurrentState == State::DYING; }

    void SetState(State state);
    State GetState() const { return m_CurrentState; }
    Type GetType() const { return m_Type; }

    glm::vec2 GetPosition() const { return m_Transform.translation; }

    // 🚩 修正：只有當進入 DEAD 狀態，且倒下計時器真的扣完時，才允許 App.cpp 將本體移除
    bool CanRemove() const {
        return m_CurrentState == State::DEAD && m_DeathTimer <= 0.0f;
    }

    void StartJump();

    bool HasJumped() const { return m_HasJumped; }
    bool IsJumping() const { return m_IsJumping; }
    bool IsPreparingJump() const { return m_IsPreparingJump; }

    std::shared_ptr<ZombieHead> SpawnHead();

private:
    Type m_Type;
    float m_ArmorHP = 0.0f;
    float m_HP = 270.0f;
    float m_Speed = 14.4f;
    float m_AttackPower = 100.0f;

    float m_DeathTimer = 0.0f;

    std::shared_ptr<Util::SFX> m_EatSFX;
    float m_EatSoundTimer = 0.0f;

    State m_CurrentState = State::WALKING;

    float m_SlowTimer = 0.0f;
    bool m_IsSlowed = false;
    int m_BaseAnimInterval = 120;

    bool m_IsLostArm = false;
    bool m_IsDecapitated = false;
    bool m_HeadHandedOut = false;

    bool m_IsPreparingJump = false;
    bool m_HasJumped = false;
    bool m_IsJumping = false;
    float m_JumpTimer = 0.0f;

    std::shared_ptr<Util::Animation> m_Animation;
    void UpdateAnimation();
};

#endif