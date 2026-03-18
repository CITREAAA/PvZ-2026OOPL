#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "GameMap.hpp"
#include "pea.hpp"
#include "seedBank.hpp"
#include "Sun.hpp"
#include "Util/Renderer.hpp"

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
    void ValidTask();
    State m_CurrentState = State::START;

    std::shared_ptr<GameMap> m_Map;
    Util::Renderer m_Root;

    std::shared_ptr<SeedBank> m_SeedBank;

    std::vector<std::shared_ptr<Sun>> m_Suns;
    int m_SunCurrency = 50;

    std::vector<std::shared_ptr<Pea>> m_Peas;
};

#endif