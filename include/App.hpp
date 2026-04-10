#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "GameMap.hpp"
#include "pea.hpp"
#include "seedBank.hpp"
#include "Sun.hpp"
#include "Zombie.hpp"
#include "Plant.hpp"
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include <vector>
#include <memory>

class App {
public:
    enum class State {
        START,
        SELECT_LEVEL,
        UPDATE,
        VICTORY,
        DEFEAT,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

private:
    // --- 核心狀態管理 ---
    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::shared_ptr<Util::Image> m_ImgPea;
    std::shared_ptr<Util::Image> m_ImgSun;
    std::shared_ptr<Util::Image> m_ImgNut;

    // --- 首頁選單物件 ---
    std::shared_ptr<Util::GameObject> m_MenuBackground;
    std::shared_ptr<Util::GameObject> m_StartButton;

    // --- 關卡選擇物件 ---
    std::shared_ptr<Util::GameObject> m_SelectLevelBG;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelButtons;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelTexts;
    int m_CurrentLevel = 0;

    // --- 地圖與植物系統 ---
    std::shared_ptr<GameMap> m_Map;
    std::vector<std::shared_ptr<Pea>> m_Peas;

    // --- 殭屍系統 ---
    std::vector<std::shared_ptr<Zombie>> m_zombies;
    std::vector<std::shared_ptr<ZombieHead>> m_ZombieHeads;

    // --- 陽光系統 ---
    std::vector<std::shared_ptr<Sun>> m_Suns;
    int m_SunCurrency = 50;

    // --- UI 與 拖曳系統 ---
    std::shared_ptr<SeedBank> m_SeedBank;
    std::shared_ptr<Util::GameObject> m_DragPreview;
    int m_SelectedPlantType = 0;

    std::shared_ptr<Util::GameObject> m_DefeatScreen;

    void ResetGame();
    void UpdatePlantActions();
    void ValidTask();
};

#endif