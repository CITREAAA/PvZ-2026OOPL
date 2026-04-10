#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

class GameEntity : public Util::GameObject {
protected:
    int hp;
    std::shared_ptr<Util::Image> m_Image;

public:
    GameEntity(const std::string& imagePath, int h) : hp(h) {
        if (!imagePath.empty()) {
            m_Image = std::make_shared<Util::Image>(imagePath);
            m_Drawable = m_Image;
        }
    }

    virtual void Update() = 0;
    
    void TakeDamage(int damage) { hp -= damage; }
    bool IsAlive() const { return hp > 0; }
};

#endif