#include "App.hpp"
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
    m_CurrentState = State::UPDATE;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();

    // --- 1. 處理滑鼠點擊 ---
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        bool actionHandled = false;

        // A. 收集陽光 (優先處理)
        for (auto it = m_Suns.begin(); it != m_Suns.end(); ) {
            if ((*it)->IsClicked(mousePos)) {
                m_SunCurrency += 25;
                (*it)->Collect();
                LOG_DEBUG("Sun Collected! Current Sun: {}", m_SunCurrency);
                m_Root.RemoveChild(*it);
                it = m_Suns.erase(it);
                actionHandled = true;
                break;
            } else {
                ++it;
            }
        }

        // B. 種植向日葵 (如果沒點到陽光)
        if (!actionHandled) {
            int r, c;
            if (m_Map->GetGridIndex(mousePos, r, c)) {
                if (m_SunCurrency >= 50) { // 向日葵 50 元
                    auto newSunflower = std::make_shared<Sunflower>(0.0f, 0.0f);
                    if (m_Map->PlacePlant(r, c, newSunflower)) {
                        m_SunCurrency -= 50;
                        LOG_DEBUG("Sunflower planted at [{}, {}]! Remaining Sun: {}", r, c, m_SunCurrency);
                    } else {
                        LOG_DEBUG("Grid [{}, {}] is already occupied!", r, c);
                    }
                } else {
                    LOG_DEBUG("Not enough sun! (Need 50, Current: {})", m_SunCurrency);
                }
            }
        }
    }

    // --- 2. 更新太陽位置 (掉落邏輯) ---
    for (auto& sun : m_Suns) {
        sun->Update();
    }

    // --- 3. 天空太陽生成計時器 (每 10 秒) ---
    static float skySunTimer = 0.0f;
    skySunTimer += static_cast<float>(Util::Time::GetDeltaTime());

    if (skySunTimer > 10.0f) {
        float randomX = static_cast<float>(rand() % 611 - 430);
        float targetY = static_cast<float>(rand() % 291 - 140);
        auto newSun = std::make_shared<Sun>(randomX, targetY);
        m_Suns.push_back(newSun);
        m_Root.AddChild(newSun);
        skySunTimer = 0.0f;
    }
    // --- 3. 向日葵太陽生成 ---
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto plant = m_Map->GetPlant(r, c); // 假設你有這個 Getter
            if (plant) {
                // 嘗試將 Plant 轉型為 Sunflower
                auto sunflower = std::dynamic_pointer_cast<Sunflower>(plant);
                if (sunflower && sunflower->CanProduceSun()) {

                    // 在向日葵的位置稍微偏移一點產出太陽
                    float sunX = sunflower->m_Transform.translation.x + 20.0f;
                    float sunY = sunflower->m_Transform.translation.y;

                    // 這裡我們直接產生一個目標高度就在原地附近的太陽 (讓它像彈出來一樣)
                    auto newSun = std::make_shared<Sun>(sunX, sunY - 20.0f);

                    // 加入管理清單
                    m_Suns.push_back(newSun);
                    m_Root.AddChild(newSun);

                    // 重要：重置向日葵的生產旗標，不然它會每一幀都噴一顆太陽
                    sunflower->ResetSunFlag();

                    LOG_DEBUG("A Sunflower produced a sun! (at {}, {})", r, c);
                }
            }
        }
    }

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
