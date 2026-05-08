#ifndef PEA_HPP
#define PEA_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

class Pea : public Util::GameObject {
public:
    enum class Type { NORMAL, ICE }; // 新增型別
    Pea(float x, float y, Type type = Type::NORMAL);
    Type GetPeaType() const { return m_Type; }
    Pea(float x, float y);
    void Update(float dt);
    bool IsOffScreen() const { return m_Transform.translation.x > 640.0f; } // 假設螢幕邊界
    glm::vec2 GetPosition() const { return m_Transform.translation; }
private:
    Type m_Type;
};

#endif
