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
    enum class Type { NORMAL, CONEHEAD, BUCKETHEAD, POLEVAULTER };

    Zombie(float x, float y, Type type = Type::NORMAL);

    float GetAttackPower() const { return m_AttackPower; }

    void TakeDamage(int damage);
    void Update(float dt);

    bool IsDead() const { return m_CurrentState == State::DEAD; }
    bool IsDying() const { return m_CurrentState == State::DYING; }

    void SetState(State state);
    State GetState() const { return m_CurrentState; }
    Type GetType() const { return m_Type; }

    glm::vec2 GetPosition() const { return m_Transform.translation; }

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