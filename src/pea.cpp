#include "Pea.hpp"
#include "Util/Time.hpp"

Pea::Pea(float x, float y, Type type) : m_Type(type) {
    m_Transform.translation = {x, y};
    if (m_Type == Type::ICE) {
        SetDrawable(std::make_shared<Util::Image>("resources/image/pea/peaIce.png"));
    } else {
        SetDrawable(std::make_shared<Util::Image>("resources/image/pea/pea.png"));
    }
}

void Pea::Update(float dt) {
    float speed = 450.0f;
    m_Transform.translation.x += speed * dt;
}