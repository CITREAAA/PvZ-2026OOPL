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
#include <vector>
#include <memory>

class App {
public:
    enum class State {
        START,
        UPDATE,
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

    // --- 地圖與植物系統 ---
    std::shared_ptr<GameMap> m_Map;
    std::vector<std::shared_ptr<Pea>> m_Peas;

    // --- 殭屍系統 ---
    std::vector<std::shared_ptr<Zombie>> m_zombies;
    std::vector<std::shared_ptr<Util::GameObject>> m_ZombieHeads;

    // --- 陽光系統 ---
    std::vector<std::shared_ptr<Sun>> m_Suns; // 畫面上的陽光
    int m_SunCurrency = 50;                   // 目前持有的陽光數量

    // --- UI 與 拖曳系統 ---
    std::shared_ptr<SeedBank> m_SeedBank;     // 上方的卡槽
    std::shared_ptr<Util::GameObject> m_DragPreview; // 拖曳時跟隨滑鼠的植物預覽
    int m_SelectedPlantType = 0;              // 目前選中的類型 (0:無, 1:豌豆, 2:向日葵, 3:堅果牆)

    void UpdatePlantActions(); // 植物行為
    void ValidTask();
};

#endif