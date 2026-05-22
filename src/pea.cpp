#include "Pea.hpp"
#include "Util/Image.hpp"
#include "Util/Animation.hpp"

Pea::Pea(float x, float y, Type type, float maxDist)
    : m_Type(type), m_StartX(x), m_MaxTravelDist(maxDist) {

    m_Transform.translation = {x, y};
    m_ZIndex = 60;
    m_Speed = 300.0f;

    if (m_Type == Type::ICE) {
        SetDrawable(std::make_shared<Util::Image>("resources/image/pea/ice_pea.png"));
    }
    // 🚩 讓 MUSHROOM 和 FUME 都使用這組動畫
    else if (m_Type == Type::MUSHROOM || m_Type == Type::FUME) {
        std::vector<std::string> paths = {
            "resources/image/pea/mushroom/1.png",
            "resources/image/pea/mushroom/2.png",
            "resources/image/pea/mushroom/3.png",
            "resources/image/pea/mushroom/4.png",
            "resources/image/pea/mushroom/5.png"
        };
        SetDrawable(std::make_shared<Util::Animation>(paths, true, 100, true));
    }
    else {
        SetDrawable(std::make_shared<Util::Image>("resources/image/pea/pea.png"));
    }
}

void Pea::Update(float dt) {
    m_Transform.translation.x += m_Speed * dt;
}