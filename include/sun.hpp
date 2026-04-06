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
    float m_FallSpeed = 100.0f;
    bool m_IsCollected = false;
    bool m_IsExpired = false;
    float m_SurvivalTimer = 0.0f;

public:
    Sun(float startX, float startY, float targetY);

    void Update(float dt);

    bool IsClicked(glm::vec2 mousePos);

    void Collect() { m_IsCollected = true; }

    bool ShouldRemove() const { return m_IsCollected || m_IsExpired; }

    bool IsExpired() const { return m_IsExpired; }
};

#endif