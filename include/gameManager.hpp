//
// Created by user on 2026/3/13.
//

#ifndef PVZ_GAMEMANAGER_HPP
#define PVZ_GAMEMANAGER_HPP

#include "GameMap.hpp"
#include <vector>

class GameManager {
private:
    GameMap map;
    int totalSun;
    int currentLevel;
    bool isGameOver;

public:
    GameManager();

    void startGame();
    void update();
    void draw();

    void addSun(int amount);
    bool spendSun(int amount);

    void nextLevel();
    bool checkWinCondition();
};

#endif //PVZ_GAMEMANAGER_HPP