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
        int weightPole;
        int weightFootball;
        int weightScreenDoor;
        std::vector<int> allowedPlants;
    };

    float m_StateTimer = 0.0f;
    int m_ZombiesSpawnedInLevel = 0;
    int m_TotalZombiesToSpawn = 0;
    float m_ZombieSpawnTimer = 0.0f;
    LevelConfig m_CurrentLevelConfig;

    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::shared_ptr<Util::Image> m_ImgPea;
    std::shared_ptr<Util::Image> m_ImgSun;
    std::shared_ptr<Util::Image> m_ImgNut;
    std::shared_ptr<Util::Image> m_ImgPotatoMine;
    std::shared_ptr<Util::Image> m_ImgSnowPea;
    std::shared_ptr<Util::Image> m_ImgCherry;
    std::shared_ptr<Util::Image> m_ImgSunShroom;
    std::shared_ptr<Util::Image> m_ImgPuff;
    std::shared_ptr<Util::Image> m_ImgFume;
    std::shared_ptr<Util::Image> m_ImgScaredy;
    std::shared_ptr<Util::Image> m_ImgRepeater;
    std::shared_ptr<Util::Image> m_ImgShovel;

    std::shared_ptr<Util::GameObject> m_MenuBackground;
    std::shared_ptr<Util::GameObject> m_StartButton;
    std::shared_ptr<Util::GameObject> m_SelectLevelBG;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelButtons;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelTexts;
    int m_CurrentLevel = 0;

    std::shared_ptr<GameMap> m_Map;
    std::vector<std::shared_ptr<Pea>> m_Peas;
    std::vector<std::shared_ptr<Zombie>> m_zombies;
    std::vector<std::shared_ptr<ZombieHead>> m_ZombieHeads;
    std::vector<std::shared_ptr<Sun>> m_Suns;
    int m_SunCurrency = 50;

    std::shared_ptr<SeedBank> m_SeedBank;
    std::shared_ptr<Util::GameObject> m_DragPreview;
    int m_SelectedPlantType = 0;
    std::shared_ptr<Util::GameObject> m_DefeatScreen;

    std::shared_ptr<Util::BGM> m_MenuBGM;
    std::shared_ptr<Util::BGM> m_GameBGM;
    std::shared_ptr<Util::SFX> m_SunCollectSFX;
    std::shared_ptr<Util::SFX> m_PlantSeedSFX;
    std::shared_ptr<Util::SFX> m_PeaHitSFX;
    std::shared_ptr<Util::SFX> m_DefeatSFX;
    std::shared_ptr<Util::SFX> m_ExplodeSFX;

    std::vector<std::shared_ptr<LawnMowerData>> m_LawnMowers;
    std::shared_ptr<Util::SFX> m_LawnMowerSFX;

    // 內部子函式
    void ResetGame();
    void LoadLevelConfig(int level);
    void UpdatePlantActions();
    void ExecutePlanting(glm::vec2 mousePos);
    void UpdateLawnMowers(float dt);
    void UpdateSuns(float dt);
    void UpdateProjectiles(float dt);
    void UpdateZombies(float dt);
};

#endif