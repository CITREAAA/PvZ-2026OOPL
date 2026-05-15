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
        FUMESHROOM = 10, SCAREDYSHROOM = 11, REPEATER = 12
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

// --- CherryBomb ---
class CherryBomb : public Plant {
public:
    CherryBomb(float x, float y);
    void Update(float dt) override;

    // 🚩 讓 App.cpp 呼叫，判定「現在這瞬間」要不要算傷害
    bool LogicReady() {
        // 如果時間到了 (1秒) 且還沒觸發過傷害
        if (m_Timer >= 1.0f && !m_LogicTriggered) {
            m_LogicTriggered = true; // 鎖定，下次就不會再回傳 true
            return true;
        }
        return false;
    }

    Type GetType() const override;

private:
    float m_Timer = 0.0f;
    bool m_LogicTriggered = false; // 專門控制傷害結算
    bool m_Exploded = false;
};

// --- Repeater (連發豌豆) ---
class Repeater : public Plant {
public:
    Repeater(float x, float y);
    void Update(float dt) override;

    // 讓 App.cpp 判斷是否可以發射
    bool CanFire() { return m_CanFire; }
    void ResetFireFlag() { m_CanFire = false; }

    Type GetType() const override;

private:
    float m_FireTimer = 0.0f;
    const float m_FireInterval = 1.4f; // 射速約 1.36~1.5 秒
    bool m_CanFire = false;
};

// --- SunShroom (陽光蘑菇) ---
class SunShroom : public Plant {
public:
    SunShroom(float x, float y);
    void Update(float dt) override;
    bool CanProduceSun() { return m_ReadyToProduce; }
    void ResetSunFlag() { m_ReadyToProduce = false; }
    int GetSunAmount() const { return m_IsGrown ? 25 : 15; }
    Type GetType() const override;

private:
    float m_SunTimer = 0.0f;
    float m_GrowthTimer = 0.0f;
    bool m_ReadyToProduce = false;
    bool m_IsGrown = false;
};

// --- PuffShroom (小噴菇) ---
class PuffShroom : public Plant {
public:
    PuffShroom(float x, float y);
    void Update(float dt) override;
    bool CanFire() { return m_CanFire; }
    void ResetFireFlag() { m_CanFire = false; }
    Type GetType() const override;

private:
    float m_FireTimer = 0.0f;
    bool m_CanFire = false;
};

// --- FumeShroom (大噴菇) ---
class FumeShroom : public Plant {
public:
    FumeShroom(float x, float y);
    void Update(float dt) override;
    bool CanFire() { return m_CanFire; }
    void ResetFireFlag() { m_CanFire = false; }
    Type GetType() const override;

private:
    float m_FireTimer = 0.0f;
    bool m_CanFire = false;
};

// --- ScaredyShroom (膽小菇) ---
class ScaredyShroom : public Plant {
public:
    ScaredyShroom(float x, float y);
    void Update(float dt) override;
    bool CanFire() { return m_CanFire && !m_IsScared; }
    void ResetFireFlag() { m_CanFire = false; }
    void SetScared(bool scared);
    bool IsScared() const { return m_IsScared; }
    Type GetType() const override;

private:
    float m_FireTimer = 0.0f;
    bool m_CanFire = false;
    bool m_IsScared = false;
};

#endif