#include "App.hpp"

#include "pea.hpp"
#include "seedBank.hpp"
#include "Util/Time.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    m_Map = std::make_shared<GameMap>("resources/image/map.jpg");
    m_Root.AddChild(m_Map);
    m_Map->m_Transform.translation = {0.0f, 0.0f};
    m_Map->m_Transform.scale = {2.0f, 2.0f};
    m_SeedBank = std::make_shared<SeedBank>();
    m_Root.AddChild(m_SeedBank);
    m_CurrentState = State::UPDATE;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    static int selectedType = 1; // 1: 豌豆, 2: 太陽花

    // --- 1. 處理滑鼠點擊 ---
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        bool actionHandled = false;

        // A. 優先判定是否點擊 SeedBank (UI 優先)
        // 假設 SeedBank 高度在 230 以上
        if (mousePos.y > 200.0f) {
            int type = m_SeedBank->GetSelectedType(mousePos);
            if (type != 0) {
                selectedType = type;
                LOG_DEBUG("Seed Selected: {}", (selectedType == 1 ? "Peashooter" : "Sunflower"));
                actionHandled = true;
            }
        }

        // B. 收集陽光
        if (!actionHandled) {
            for (auto it = m_Suns.begin(); it != m_Suns.end(); ) {
                if ((*it)->IsClicked(mousePos)) {
                    m_SunCurrency += 25;
                    (*it)->Collect();
                    m_Root.RemoveChild(*it);
                    it = m_Suns.erase(it);
                    actionHandled = true;
                    LOG_DEBUG("Sun Collected! Total: {}", m_SunCurrency);
                    break;
                } else { ++it; }
            }
        }

        // C. 種植邏輯
        if (!actionHandled) {
            int r, c;
            if (m_Map->GetGridIndex(mousePos, r, c)) {
                if (selectedType == 1 && m_SunCurrency >= 100) {
                    auto newPea = std::make_shared<Peashooter>(0.0f, 0.0f);
                    if (m_Map->PlacePlant(r, c, newPea)) {
                        m_SunCurrency -= 100;
                    }
                }
                else if (selectedType == 2 && m_SunCurrency >= 50) {
                    auto newFlower = std::make_shared<Sunflower>(0.0f, 0.0f);
                    if (m_Map->PlacePlant(r, c, newFlower)) {
                        m_SunCurrency -= 50;
                    }
                }
            }
        }
    }

    // --- 2. 物件邏輯更新 (太陽、子彈) ---
    for (auto& sun : m_Suns) sun->Update();

    for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
        (*it)->Update();
        if ((*it)->IsOffScreen()) {
            m_Root.RemoveChild(*it);
            it = m_Peas.erase(it);
        } else { ++it; }
    }

    // --- 3. 天空太陽生成 ---
    static float skySunTimer = 0.0f;
    skySunTimer += static_cast<float>(Util::Time::GetDeltaTime());
    if (skySunTimer > 10.0f) {
        float randomX = static_cast<float>(rand() % 611 - 430);
        float targetY = static_cast<float>(rand() % 291 - 140);
        auto skySun = std::make_shared<Sun>(randomX, 320.0f, targetY);
        m_Suns.push_back(skySun);
        m_Root.AddChild(skySun);
        skySunTimer = 0.0f;
    }

    // --- 4. 偵測植物行為 (產太陽、射豆子) ---
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto plant = m_Map->GetPlant(r, c);
            if (!plant) continue;

            // 向日葵行為
            auto flower = std::dynamic_pointer_cast<Sunflower>(plant);
            if (flower && flower->CanProduceSun()) {
                auto s = std::make_shared<Sun>(flower->GetPosition().x,
                                               flower->GetPosition().y + 50.0f,
                                               flower->GetPosition().y - 10.0f);
                m_Suns.push_back(s);
                m_Root.AddChild(s);
                flower->ResetSunFlag();
            }

            // 豌豆射手行為
            auto shooter = std::dynamic_pointer_cast<Peashooter>(plant);
            if (shooter && shooter->CanFire()) {
                auto p = std::make_shared<Pea>(shooter->GetPosition().x + 30.0f,
                                              shooter->m_Transform.translation.y + 20.0f);
                m_Peas.push_back(p);
                m_Root.AddChild(p);
                shooter->ResetFireFlag();
            }
        }
    }

    m_SeedBank->SetSunCount(m_SunCurrency);
    m_SeedBank->DrawUI();

    m_Map->Update();
    m_Root.Update();

    /*
     * Do not touch the code below as they serve the purpose for
     * closing the window.
     */
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
