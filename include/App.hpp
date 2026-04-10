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
#include <vector>
#include <memory>

class App {
public:
    enum class State { START, SELECT_LEVEL, UPDATE, END };

    State GetCurrentState() const { return m_CurrentState; }
    void Start();
    void Update();
    void End();

private:
    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::shared_ptr<Util::Image> m_ImgPea, m_ImgSun, m_ImgNut;
    std::shared_ptr<Util::GameObject> m_MenuBackground, m_StartButton, m_SelectLevelBG;
    std::vector<std::shared_ptr<Util::GameObject>> m_LevelButtons, m_LevelTexts;
    int m_CurrentLevel = 1;

    std::shared_ptr<GameMap> m_Map;
    std::vector<std::shared_ptr<Pea>> m_Peas;
    std::vector<std::shared_ptr<Zombie>> m_zombies;
    std::vector<std::shared_ptr<ZombieHead>> m_ZombieHeads;

    std::vector<std::shared_ptr<Sun>> m_Suns;
    int m_SunCurrency = 50;
    std::shared_ptr<SeedBank> m_SeedBank;
    std::shared_ptr<Util::GameObject> m_DragPreview;
    int m_SelectedPlantType = 0;

    void UpdatePlantActions();
};

#endif // APP_HPP