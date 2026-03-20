#include "App.hpp"
#include "pea.hpp"
#include "seedBank.hpp"
#include "Util/Time.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    // 地圖初始化
    m_Map = std::make_shared<GameMap>("resources/image/map.jpg");
    m_Map->m_Transform.translation = {0.0f, 0.0f};
    m_Map->m_Transform.scale = {2.0f, 2.0f};
    m_Root.AddChild(m_Map);

    // UI 初始化
    m_SeedBank = std::make_shared<SeedBank>();

    // 預覽物件初始化 (滑鼠抓起植物時的圖案)
    m_DragPreview = std::make_shared<Util::GameObject>();
    m_DragPreview->SetZIndex(100);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    // 統一使用 GetDeltaTime() (秒) 避免 Deprecated 警告
    float dt = static_cast<float>(Util::Time::GetDeltaTime());
    m_SeedBank->UpdateCooldown(dt);

    // --- 1. 輸入處理 (點擊與拖曳) ---

    // 按下左鍵：處理收集與抓取
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        bool actionHandled = false;

        // A. 收集陽光
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

        // B. 抓取卡片 (如果沒點到陽光且手上手空的)
        if (!actionHandled && m_SelectedPlantType == 0) {
            int type = m_SeedBank->GetSelectedType(mousePos);
            if (type != 0) {
                m_SelectedPlantType = type;
                std::string imgPath;
                if (type == 1) imgPath = "resources/image/peashooter/peashooter_1.png";
                else if (type == 2) imgPath = "resources/image/sunflower/1.png";
                else if (type == 3) imgPath = "resources/image/wallnut/1.png";

                m_DragPreview->SetDrawable(std::make_shared<Util::Image>(imgPath));
                LOG_DEBUG("Dragging Plant Type: {}", type);
            }
        }
    }

    // 放開左鍵：執行種植
    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_SelectedPlantType != 0) {
        int r, c;
        if (m_Map->GetGridIndex(mousePos, r, c)) {
            bool success = false;
            // 判定植物類型與陽光是否足夠
            if (m_SelectedPlantType == 1 && m_SunCurrency >= 100) {
                auto newPea = std::make_shared<Peashooter>(0.0f, 0.0f);
                if (m_Map->PlacePlant(r, c, newPea)) {
                    m_SunCurrency -= 100;
                    m_SeedBank->StartCooldown(1);
                    success = true;
                }
            } else if (m_SelectedPlantType == 2 && m_SunCurrency >= 50) {
                auto newFlower = std::make_shared<Sunflower>(0.0f, 0.0f);
                if (m_Map->PlacePlant(r, c, newFlower)) {
                    m_SunCurrency -= 50;
                    m_SeedBank->StartCooldown(2);
                    success = true;
                }
            } else if (m_SelectedPlantType == 3 && m_SunCurrency >= 50) {
                // 新增堅果牆種植
                auto newWallnut = std::make_shared<Wallnut>(0.0f, 0.0f);
                if (m_Map->PlacePlant(r, c, newWallnut)) {
                    m_SunCurrency -= 50;
                    m_SeedBank->StartCooldown(3); // 觸發 30 秒冷卻
                    success = true;
                }
            }
        }

        // 重設拖曳狀態
        m_SelectedPlantType = 0;
        m_DragPreview->SetDrawable(nullptr);
    }

    // --- 2. 邏輯更新 ---

    for (auto& sun : m_Suns) sun->Update();
    for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
        (*it)->Update();
        if ((*it)->IsOffScreen()) {
            m_Root.RemoveChild(*it);
            it = m_Peas.erase(it);
        } else { ++it; }
    }

    // 天空陽光生成
    static float skySunTimer = 0.0f;
    skySunTimer += dt;
    if (skySunTimer > 13.0f) {
        float randomX = static_cast<float>(rand() % 611 - 430);
        float targetY = static_cast<float>(rand() % 291 - 140);
        auto skySun = std::make_shared<Sun>(randomX, 320.0f, targetY);
        m_Suns.push_back(skySun);
        m_Root.AddChild(skySun);
        skySunTimer = 0.0f;
    }

    UpdatePlantActions(); // 執行植物射擊與產陽光

    // --- 3. 渲染與繪製 ---

    m_Map->Update();
    m_Root.Update();

    m_SeedBank->SetSunCount(m_SunCurrency);
    m_SeedBank->DrawUI();

    // 繪製拖曳預覽 (在滑鼠位置或格點中心)
    if (m_SelectedPlantType != 0) {
        int r, c;
        if (m_Map->GetGridIndex(mousePos, r, c)) {
            m_DragPreview->m_Transform.translation = m_Map->CalculateGridCenter(r, c);
        } else {
            m_DragPreview->m_Transform.translation = mousePos;
        }
        m_DragPreview->Draw();
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::UpdatePlantActions() {
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto plant = m_Map->GetPlant(r, c);
            if (!plant) continue;

            // 必須呼叫 Update 讓堅果牆檢查血量換圖，或讓射手計時
            plant->Update();

            // 向日葵產陽光邏輯
            auto flower = std::dynamic_pointer_cast<Sunflower>(plant);
            if (flower && flower->CanProduceSun()) {
                auto s = std::make_shared<Sun>(flower->GetPosition().x,
                                               flower->GetPosition().y + 50.0f,
                                               flower->GetPosition().y - 10.0f);
                m_Suns.push_back(s);
                m_Root.AddChild(s);
                flower->ResetSunFlag();
            }

            // 豌豆射手射擊邏輯
            auto shooter = std::dynamic_pointer_cast<Peashooter>(plant);
            if (shooter && shooter->CanFire()) {
                auto p = std::make_shared<Pea>(shooter->GetPosition().x + 30.0f,
                                              shooter->m_Transform.translation.y + 20.0f);
                m_Peas.push_back(p);
                m_Root.AddChild(p);
                shooter->ResetFireFlag();
            }

            // 堅果牆 (Wallnut) 只有 Update 換圖邏輯，沒有主動 Action，故不需額外處理
        }
    }
}

void App::End() {
    LOG_TRACE("End");
}