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
    enum class Type {
        PEASHOOTER = 1, SUNFLOWER = 2, WALLNUT = 3, MINE = 5,
        SNOWPEA = 6, CHERRYBOMB = 7, SUNSHROOM = 8, PUFFSHROOM = 9,
        FUMESHROOM = 10, SCAREDYSHROOM = 11, REPEATERPEA = 12
    };

    Plant(const std::vector<std::string>& paths, int h, int c)
        : GameEntity("", h), m_HP(h), cost(c) {}

    glm::vec2 GetPosition() const { return m_Transform.translation; }
    int GetHP() const { return m_HP; }
    void TakeDamage(int damage) { m_HP -= damage; }
    bool IsDead() const { return m_HP <= 0; }

    virtual void Update(float dt) = 0;

    virtual Type GetType() const = 0;
};

class Peashooter : public Plant {
public:
    Peashooter(float x, float y);
    void Update(float dt) override;
    void Attack();
    bool CanFire() const { return m_ShouldFire; }
    void ResetFireFlag() { m_ShouldFire = false; }
    Type GetType() const override;

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
    Type GetType() const override;

private:
    float m_ProductionTimer = 0.0f;
    float m_NextProductionTime = 0.0f;
    bool m_ProducedSun = false;
};

class Wallnut : public Plant {
public:
    Wallnut(float x, float y);
    void Update(float dt) override;
    Type GetType() const override;

private:
    int m_CurrentStage = 1;
};

// --- PotatoMine ---
class PotatoMine : public Plant {
public:
    enum class MineState { UNDERGROUND, READY, EXPLODING };

    PotatoMine(float x, float y);
    void Update(float dt) override;
    void Trigger();
    MineState GetMineState() const { return m_State; }
    Type GetType() const override;

private:
    MineState m_State = MineState::UNDERGROUND;
    float m_LocalTimer = 0.0f;
    std::shared_ptr<Util::Image> m_ImgUnderground;
    std::shared_ptr<Util::Image> m_ImgExplode;
    std::vector<std::shared_ptr<Util::Image>> m_FramesReady;
};

// --- SnowPea ---
class SnowPea : public Plant {
public:
    SnowPea(float x, float y);
    void Update(float dt) override;
    bool CanFire() const { return m_ShouldFire; }
    void ResetFireFlag() { m_ShouldFire = false; }
    Type GetType() const override;

private:
    float m_FireTimer = 0.0f;
    bool m_ShouldFire = false;
};

#endif