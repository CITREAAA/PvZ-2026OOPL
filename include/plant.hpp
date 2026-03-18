#ifndef PVZ_PLANT_HPP
#define PVZ_PLANT_HPP

#include "GameEntity.hpp"
#include "Util/Animation.hpp"

class Plant : public GameEntity {
protected:
    int cost;
    std::shared_ptr<Util::Animation> m_Animation;

public:
    Plant(const std::vector<std::string>& paths, int h, int c)
        : GameEntity("", h), cost(c) {
        // 設定動畫
        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation;
    }

    // --- 新增這個函式，解決 App.cpp 的報錯 ---
    glm::vec2 GetPosition() const {
        return m_Transform.translation;
    }

    virtual void Attack() = 0;
};

// --- 豌豆射手 ---
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

// --- 向日葵 ---
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

#endif