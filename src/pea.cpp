#include "Pea.hpp"
#include "Util/Time.hpp"

Pea::Pea(float x, float y) {
    m_Drawable = std::make_shared<Util::Image>("resources/image/pea/pea.png");
    m_Transform.translation = {x, y};
    m_ZIndex = 70;
    m_Transform.scale = {1.0f, 1.0f};
}

void Pea::Update(float dt) {
    float speed = 400.0f;
    m_Transform.translation.x += speed * static_cast<float>(Util::Time::GetDeltaTime());
}