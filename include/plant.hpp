#ifndef PVZ_PLANT_HPP
#define PVZ_PLANT_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"
#include <vector>
#include <string>

class Plant : public GameEntity {
protected:
    int cost;
    int m_HP;
    std::shared_ptr<Util::Animation> m_Animation;

public:
    Plant(const std::vector<std::string>& paths, int h, int c)
        : GameEntity("", h), m_HP(h), cost(c) {
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
    }

    int GetHP() const { return m_HP; }

    void TakeDamage(int damage) { m_HP -= damage; }

    glm::vec2 GetPosition() const { return m_Transform.translation; }
    virtual void Attack() = 0;
};

// --- Peashooter ---
class Peashooter : public Plant {
public:
    Peashooter(float x, float y);
    void Update() override;
    void Attack() override;
    bool CanFire() const { return m_ShouldFire; }
    void ResetFireFlag() { m_ShouldFire = false; }
private:
    float m_FireTimer = 0.0f;
    bool m_ShouldFire = false;
};

// --- Sunflower ---
class Sunflower : public Plant {
public:
    Sunflower(float x, float y);
    void Update() override;
    void Attack() override;
    bool CanProduceSun() const { return m_ProducedSun; }
    void ResetSunFlag() { m_ProducedSun = false; }
private:
    float m_ProductionTimer = 0.0f;
    float m_NextProductionTime = 0.0f;
    bool m_ProducedSun = false;
};

// --- Wallnut ---
class Wallnut : public Plant {
public:
    Wallnut(float x, float y);
    void Update() override;
    void Attack() override {}
private:
    int m_CurrentStage = 1;
};

#endif