//
// Created by user on 2026/3/13.
//

#ifndef PVZ_ZOMBIE_HPP
#define PVZ_ZOMBIE_HPP

#include "GameEntity.hpp"
#include "plant.hpp"

class Zombie : public GameEntity {
protected:
    float speed;
    int attackPower;
public:
    Zombie(std::string n, int h, int x, int y, float s, int ap)
        : GameEntity(n, h, x, y), speed(s), attackPower(ap) {}

    virtual void move();
    virtual void eat(Plant& target);
    void Update() override;
};

class NormalZombie : public Zombie {
public:
    NormalZombie(int x, int y) : Zombie("NormalZombie", 150, x, y, 0.5f, 10) {}
};

#endif //PVZ_ZOMBIE_HPP