#ifndef PEA_HPP
#define PEA_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Animation.hpp"

class Pea : public Util::GameObject {
public:
    // 🚩 新增 FUME 類型，專給大噴菇用
    enum class Type { NORMAL, ICE, MUSHROOM, FUME };

    Pea(float x, float y, Type type = Type::NORMAL, float maxDist = 9999.0f);

    Type GetPeaType() const { return m_Type; }
    void Update(float dt);
    bool IsOffScreen() const { return m_Transform.translation.x > 640.0f; }
    glm::vec2 GetPosition() const { return m_Transform.translation; }

    float GetStartX() const { return m_StartX; }
    float GetMaxTravelDist() const { return m_MaxTravelDist; }

private:
    Type m_Type;
    float m_StartX = 0.0f;
    float m_MaxTravelDist = 9999.0f;
    float m_Speed = 300.0f;
};

#endif