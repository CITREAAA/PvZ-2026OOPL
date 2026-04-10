#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <vector>
#include <string>
#include <glm/vec2.hpp>

class ZombieHead : public Util::GameObject {
public:
    ZombieHead(float startX, float startY, glm::vec2 initialVelocity);
    void Update(float dt);
    bool CanRemove() const { return m_Transform.translation.y < -300.0f; }
private:
    glm::vec2 m_Velocity;
};

class Zombie : public GameEntity {
public:
    enum class State { WALKING, EATING, DEAD };

    Zombie(float x, float y);

    // 🚩 修正：這裡必須接收 float dt 參數
    void Update(float dt);

    // 🚩 修正：為了符合基類 GameEntity 的要求，可以保留一個空的 Update 覆寫
    void Update() override {}

    void TakeDamage(int damage);
    bool IsDead() const { return m_HP <= 0; }
    void SetState(State state);
    State GetState() const { return m_CurrentState; }
    glm::vec2 GetPosition() const { return m_Transform.translation; }
    bool CanRemove() const { return m_CurrentState == State::DEAD && m_DeathTimer <= 0.0f; }
    std::shared_ptr<ZombieHead> SpawnHead();

private:
    int m_HP = 270;
    float m_Speed = 14.4f;
    float m_DeathTimer = 0.0f;
    State m_CurrentState = State::WALKING;
    int m_AppearanceStage = 1;
    bool m_HeadDropped = false;
    std::shared_ptr<Util::Animation> m_Animation;
    void UpdateAnimation();
};

#endif