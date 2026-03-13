#ifndef SUN_HPP
#define SUN_HPP

#include "Util/GameObject.hpp"
#include "Util/Animation.hpp"
#include "Util/Input.hpp"
#include <vector>
#include <string>

class Sun : public Util::GameObject {
private:
    float m_TargetY;
    float m_FallSpeed = 2.0f;
    bool m_IsCollected = false;
    bool m_IsFlyingToScore = false;

public:
    Sun(float startX, float targetY);

    void Update();

    bool IsClicked(glm::vec2 mousePos);

    void Collect() { m_IsCollected = true; }
    bool ShouldRemove() const { return m_IsCollected; }
};

#endif