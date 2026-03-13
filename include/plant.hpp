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

        m_Animation = std::make_shared<Util::Animation>(paths, true, 100, true);
        m_Drawable = m_Animation; 
    }

    virtual void Attack() = 0;
};

class Peashooter : public Plant {
public:
    Peashooter(float x, float y);
    void Update() override;
    void Attack() override;
};

class Sunflower : public Plant {
private:
    float m_ProductionTimer = 0.0f;
    float m_NextProductionTime = 0.0f;
    bool m_ProducedSun = false;

public:
    Sunflower(float x, float y);
    void Update() override;
    void Attack() override;

    bool CanProduceSun() const { return m_ProducedSun; }
    void ResetSunFlag() { m_ProducedSun = false; }
};

#endif