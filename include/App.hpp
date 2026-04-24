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
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/BGM.hpp"
#include "Util/SFX.hpp"
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


    struct LawnMowerData {
        enum class State { IDLE, MOVING, REMOVED };
        std::shared_ptr<Util::GameObject> obj;
        State state = State::IDLE;
        int row;
        float speed = 500.0f;
    };

    struct LevelConfig {
        int totalZombies;
        float spawnInterval;
        int weightNormal;
        int weightCone;
        int weightBucket;
    };

    float m_StateTimer = 0.0f;

    int m_ZombiesSpawnedInLevel = 0;
    int m_TotalZombiesToSpawn = 0;
    float m_ZombieSpawnTimer = 0.0f;
    LevelConfig m_CurrentLevelConfig;

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
    std::shared_ptr<Util::Image> m_ImgShovel;
    std::shared_ptr<SeedBank> m_SeedBank;
    std::shared_ptr<Util::GameObject> m_DragPreview;
    int m_SelectedPlantType = 0;

    std::shared_ptr<Util::GameObject> m_DefeatScreen;

    // --- 音訊系統 ---
    std::shared_ptr<Util::BGM> m_MenuBGM;
    std::shared_ptr<Util::BGM> m_GameBGM;
    std::shared_ptr<Util::SFX> m_SunCollectSFX; // 收集陽光
    std::shared_ptr<Util::SFX> m_PlantSeedSFX;  // 種下植物
    std::shared_ptr<Util::SFX> m_PeaHitSFX;     // 豌豆打到殭屍 (給 App.cpp 判定碰撞用)
    std::shared_ptr<Util::SFX> m_DefeatSFX;

    std::vector<std::shared_ptr<LawnMowerData>> m_LawnMowers;
    std::shared_ptr<Util::SFX> m_LawnMowerSFX;

    void ResetGame();
    void LoadLevelConfig(int level);
    void UpdatePlantActions();
    void ValidTask();
};

#endif