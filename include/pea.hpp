#ifndef PEA_HPP
#define PEA_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

class Pea : public Util::GameObject {
public:
    Pea(float x, float y);
    void Update(float dt);
    bool IsOffScreen() const { return m_Transform.translation.x > 640.0f; } // 假設螢幕邊界
    glm::vec2 GetPosition() const { return m_Transform.translation; }
};

#endif