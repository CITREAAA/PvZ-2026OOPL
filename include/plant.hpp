#ifndef PVZ_PLANT_HPP
#define PVZ_PLANT_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"
#include <vector>
#include <string>
#include <memory>

class Plant : public GameEntity {
protected:
    // 調整宣告順序以符合初始化列表：m_HP 先於 cost
    int m_HP;
    int cost;
    std::shared_ptr<Util::Animation> m_Animation;

public:
    Plant(const std::vector<std::string>& paths, int h, int c)
        : GameEntity("", h), m_HP(h), cost(c) { // 這裡順序現在正確了
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
    }

    int GetHP() const { return m_HP; }
    void TakeDamage(int damage) { m_HP -= damage; }
    bool IsDead() const { return m_HP <= 0; }
    glm::vec2 GetPosition() const { return m_Transform.translation; }

    // 修正：移除 override，因為基類沒有這個函式
    virtual void Update() override = 0;
    virtual void Attack() = 0;
};

// --- 豌豆射手 (Peashooter) ---
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

// --- 向日葵 (Sunflower) ---
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

// --- 堅果牆 (Wallnut) ---
class Wallnut : public Plant {
public:
    // 堅果牆初始血量應較高 (例如 4000)
    Wallnut(float x, float y);
    void Update() override;
    void Attack() override {} // 堅果不攻擊
private:
    int m_CurrentStage = 1; // 1:正常, 2:小受傷, 3:嚴重受傷
};

#endif