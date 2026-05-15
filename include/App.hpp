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
    // =========================================================================
    // [ 內部結構定義 ]
    // =========================================================================
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
        std::vector<int> allowedPlants;
    };

    // =========================================================================
    // [ 狀態更新函式 (重構後新增) ]
    // =========================================================================
    void UpdateStartState(glm::vec2 mousePos);
    void UpdateSelectLevelState(glm::vec2 mousePos);
    void UpdateGameState(float dt, glm::vec2 mousePos);
    void UpdateDefeatState(float dt);

    // =========================================================================
    // [ 遊戲邏輯子模組 (重構後新增) ]
    // =========================================================================
    void HandleInput(glm::vec2 mousePos);
    void UpdatePlants(float dt);
    void UpdateZombies(float dt);
    void UpdateProjectiles(float dt);
    void UpdateLawnMowers(float dt);
    void UpdateSuns(float dt);

    // 輔助邏輯
    void ExecutePlanting(glm::vec2 mousePos);
    bool CheckGameOver(float dt);
    void ResetGame();
    void LoadLevelConfig(int level);

    // 植物行為工廠 (選配，若你在 ExecutePlanting 內處理則可移除)
    // std::shared_ptr<Plant> CreatePlantByType(int type);

    // =========================================================================
    // [ 成員變數 ]
    // =========================================================================

    // 核心狀態
    State m_CurrentState = State::START;
    Util::Renderer m_Root;
    float m_StateTimer = 0.0f;
    float skySunTimer = 0.0f;

    // 關卡配置
    int mapScale = 1.0f;
    int m_CurrentLevel = 0;
    int m_ZombiesSpawnedInLevel = 0;
    int m_TotalZombiesToSpawn = 0;
    float m_ZombieSpawnTimer = 0.0f;
    LevelConfig m_CurrentLevelConfig;

    // 資源預覽圖
    std::shared_ptr<Util::Image> m_ImgPea;
    std::shared_ptr<Util::Image> m_ImgSun;
    std::shared_ptr<Util::Image> m_ImgNut;
    std::shared_ptr<Util::Image> m_ImgPotatoMine;
    std::shared_ptr<Util::Image> m_ImgShovel;
    std::shared_ptr<Util::Image> m_ImgSnowPea;
    std::shared_ptr<Util::Image> m_ImgCherry;
    std::shared_ptr<Util::Image> m_ImgSunShroom;
    std::shared_ptr<Util::Image> m_ImgPuff;
    std::shared_ptr<Util::Image> m_ImgFume;
    std::shared_ptr<Util::Image> m_ImgScaredy;
    std::shared_ptr<Util::Image> m_ImgRepeater;

    // 選單與關卡 UI
    std::shared_ptr<Util::GameObject> m_MenuBackground;
    std::shared_ptr<Util::GameObject> m_StartButton;
    std::shared_ptr<Util::GameObject> m_SelectLevelBG;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelButtons;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelTexts;

    // 遊戲物件系統
    std::shared_ptr<GameMap> m_Map;
    std::vector<std::shared_ptr<Pea>> m_Peas;
    std::vector<std::shared_ptr<Zombie>> m_zombies;
    std::vector<std::shared_ptr<ZombieHead>> m_ZombieHeads;
    std::vector<std::shared_ptr<Sun>> m_Suns;
    std::vector<std::shared_ptr<LawnMowerData>> m_LawnMowers;

    // 經濟與 UI
    int m_SunCurrency = 50;
    std::shared_ptr<SeedBank> m_SeedBank;
    std::shared_ptr<Util::GameObject> m_DragPreview;
    int m_SelectedPlantType = 0;
    std::shared_ptr<Util::GameObject> m_DefeatScreen;

    // 音訊
    std::shared_ptr<Util::BGM> m_MenuBGM;
    std::shared_ptr<Util::BGM> m_GameBGM;
    std::shared_ptr<Util::SFX> m_SunCollectSFX;
    std::shared_ptr<Util::SFX> m_PlantSeedSFX;
    std::shared_ptr<Util::SFX> m_PeaHitSFX;
    std::shared_ptr<Util::SFX> m_DefeatSFX;
    std::shared_ptr<Util::SFX> m_LawnMowerSFX;
    std::shared_ptr<Util::SFX> m_ExplodeSFX;
};

#endif