#ifndef PVZ_PLANT_HPP
#define PVZ_PLANT_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"
#include <vector>
#include <string>
#include <memory>

class Plant : public GameEntity {
protected:
    int m_HP;
    int cost;
    std::shared_ptr<Util::Animation> m_Animation;

public:
    Plant(const std::vector<std::string>& paths, int h, int c)
        : GameEntity("", h), m_HP(h), cost(c) {}

    glm::vec2 GetPosition() const {
        return m_Transform.translation;
    }

    int GetHP() const { return m_HP; }
    void TakeDamage(int damage) { m_HP -= damage; }
    bool IsDead() const { return m_HP <= 0; }

    virtual void Update(float dt) = 0;
};

class Peashooter : public Plant {
public:
    Peashooter(float x, float y);
    void Update(float dt) override;
    void Attack() ;
    bool CanFire() const { return m_ShouldFire; }
    void ResetFireFlag() { m_ShouldFire = false; }
private:
    float m_FireTimer = 0.0f;
    bool m_ShouldFire = false;
};

class Sunflower : public Plant {
public:
    Sunflower(float x, float y);
    void Update(float dt) override;
    bool CanProduceSun() const { return m_ProducedSun; }
    void ResetSunFlag() { m_ProducedSun = false; }
private:
    float m_ProductionTimer = 0.0f;
    float m_NextProductionTime = 0.0f;
    bool m_ProducedSun = false;
};

class Wallnut : public Plant {
public:
    Wallnut(float x, float y);
    void Update(float dt) override;
private:
    int m_CurrentStage = 1;
};

#endif