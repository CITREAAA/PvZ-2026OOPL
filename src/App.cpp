#include "App.hpp"
#include "pea.hpp"
#include "seedBank.hpp"
#include "Zombie.hpp"
#include "Plant.hpp"
#include "Util/Time.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include <algorithm>

void App::Start() {
    // 1. 你的預載入圖片 (確保 App.hpp 裡有這些成員變數)
    m_ImgPea = std::make_shared<Util::Image>("resources/image/peashooter/peashooter_1.png");
    m_ImgSun = std::make_shared<Util::Image>("resources/image/sunflower/1.png");
    m_ImgNut = std::make_shared<Util::Image>("resources/image/wallnut/1.png");

    // 2. 首頁選單
    m_MenuBackground = std::make_shared<Util::GameObject>();
    m_MenuBackground->SetDrawable(std::make_shared<Util::Image>("resources/image/menu/menu.png"));
    m_MenuBackground->SetZIndex(10);

    m_StartButton = std::make_shared<Util::GameObject>();
    m_StartButton->SetDrawable(std::make_shared<Util::Image>("resources/image/menu/button.png"));
    m_StartButton->m_Transform.translation = {210.0f, 130.0f};
    m_StartButton->SetZIndex(20);

    // 3. 遊戲地圖
    m_Map = std::make_shared<GameMap>("resources/image/map.jpg");
    m_Map->m_Transform.scale = {2.0f, 2.0f};
    m_Map->SetZIndex(0);

    m_SeedBank = std::make_shared<SeedBank>();

    // 4. 你的預覽物件 (大絕招版)
    m_DragPreview = std::make_shared<Util::GameObject>();
    m_DragPreview->SetZIndex(100);
    m_DragPreview->SetVisible(false);
    m_Root.AddChild(m_DragPreview);

    m_CurrentState = State::START;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    // ----- [ 狀態 A: 首頁選單 ] -----
    if (m_CurrentState == State::START) {
        if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            if (mousePos.x > 50 && mousePos.x < 370 && mousePos.y > 80 && mousePos.y < 180) {
                m_Root.AddChild(m_Map);
                m_CurrentState = State::UPDATE;
            }
        }
        m_MenuBackground->Draw();
        m_StartButton->Draw();
        m_Root.Update();
    }
    // ----- [ 狀態 B: 遊戲進行中 ] -----
    else if (m_CurrentState == State::UPDATE) {
        m_SeedBank->UpdateCooldown(dt);

        // --- 1. 滑鼠放開：種植邏輯 ---
        if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_SelectedPlantType != 0) {
            int r, c;
            if (m_Map->GetGridIndex(mousePos, r, c)) {
                std::shared_ptr<Plant> p = nullptr;
                if (m_SelectedPlantType == 1 && m_SunCurrency >= 100) {
                    p = std::make_shared<Peashooter>(0,0);
                    if (m_Map->PlacePlant(r, c, p)) m_SunCurrency -= 100; else p = nullptr;
                } else if (m_SelectedPlantType == 2 && m_SunCurrency >= 50) {
                    p = std::make_shared<Sunflower>(0,0);
                    if (m_Map->PlacePlant(r, c, p)) m_SunCurrency -= 50; else p = nullptr;
                } else if (m_SelectedPlantType == 3 && m_SunCurrency >= 50) {
                    p = std::make_shared<Wallnut>(0,0);
                    if (m_Map->PlacePlant(r, c, p)) m_SunCurrency -= 50; else p = nullptr;
                }

                if (p) {
                    m_Root.AddChild(p);
                    m_SeedBank->StartCooldown(m_SelectedPlantType);
                }
            }
            m_SelectedPlantType = 0;
            m_DragPreview->SetVisible(false);
        }
        // --- 2. 滑鼠按下：陽光收集 / 抓取卡片 ---
        else if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            bool actionHandled = false;

            // 收集陽光 (天空或植物產生的都能點)
            for (auto it = m_Suns.begin(); it != m_Suns.end(); ) {
                if ((*it)->IsClicked(mousePos)) {
                    m_SunCurrency += 25;
                    (*it)->Collect();
                    m_Root.RemoveChild(*it);
                    it = m_Suns.erase(it);
                    actionHandled = true;
                    break;
                } else { ++it; }
            }

            // 抓取卡片
            if (!actionHandled && m_SelectedPlantType == 0) {
                int type = m_SeedBank->GetSelectedType(mousePos);
                if (type != 0) {
                    m_SelectedPlantType = type;
                    if (type == 1)      m_DragPreview->SetDrawable(m_ImgPea);
                    else if (type == 2) m_DragPreview->SetDrawable(m_ImgSun);
                    else if (type == 3) m_DragPreview->SetDrawable(m_ImgNut);
                    m_DragPreview->SetVisible(true);
                    m_DragPreview->m_Transform.scale = {1.2f, 1.2f};
                }
            }
        }

        // --- 3. 邏輯更新 (預覽圖位置、子彈、殭屍、產能) ---

        if (m_SelectedPlantType != 0) {
            int r, c;
            if (m_Map->GetGridIndex(mousePos, r, c))
                m_DragPreview->m_Transform.translation = m_Map->CalculateGridCenter(r, c);
            else
                m_DragPreview->m_Transform.translation = mousePos;
        }

        UpdatePlantActions();

        for (auto& sun : m_Suns) sun->Update();

        // 子彈飛行與碰撞
        for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
            (*it)->Update();
            bool peaHit = false;
            for (auto& zombie : m_zombies) {
                if (!zombie->IsDead() && glm::distance((*it)->GetPosition(), zombie->GetPosition()) < 40.0f) {
                    zombie->TakeDamage(20);
                    peaHit = true; break;
                }
            }
            if (peaHit || (*it)->IsOffScreen()) {
                m_Root.RemoveChild(*it); it = m_Peas.erase(it);
            } else { ++it; }
        }

        // 殭屍行為 (加入組員的噴頭邏輯)
        for (auto& zombie : m_zombies) {
            zombie->Update();

            // 👉 縫合組員功能：檢查噴頭
            auto droppedHead = zombie->SpawnHead();
            if (droppedHead) {
                m_ZombieHeads.push_back(droppedHead);
                m_Root.AddChild(droppedHead);
            }

            if (zombie->IsDead()) continue;

            int r, c;
            if (m_Map->GetGridIndex(zombie->GetPosition() + glm::vec2{-25, 0}, r, c)) {
                auto plant = m_Map->GetPlant(r, c);
                if (plant) {
                    zombie->SetState(Zombie::State::EATING);
                    plant->TakeDamage(static_cast<int>(100 * dt));
                    if (plant->IsDead()) {
                        m_Map->RemovePlant(r, c);
                        m_Root.RemoveChild(plant);
                        zombie->SetState(Zombie::State::WALKING);
                    }
                } else {
                    if (zombie->GetState() == Zombie::State::EATING)
                        zombie->SetState(Zombie::State::WALKING);
                }
            }
        }

        // 👉 縫合組員功能：掉落的頭往下掉
        for (auto it = m_ZombieHeads.begin(); it != m_ZombieHeads.end(); ) {
            (*it)->m_Transform.translation.y -= 100.0f * dt;
            if ((*it)->m_Transform.translation.y < -800.0f) {
                m_Root.RemoveChild(*it);
                it = m_ZombieHeads.erase(it);
            } else { ++it; }
        }

        // 移除死透的殭屍
        m_zombies.erase(std::remove_if(m_zombies.begin(), m_zombies.end(),
            [this](const std::shared_ptr<Zombie>& z) {
                if (z->CanRemove()) { m_Root.RemoveChild(z); return true; }
                return false;
            }), m_zombies.end());

        // 自動生成殭屍
        static float zombieTimer = 0.0f;
        zombieTimer += dt;
        if (zombieTimer > 10.0f) {
            int r = rand() % 5;
            float spawnY = m_Map->CalculateGridCenter(r, 8).y + 20.0f;
            auto newZ = std::make_shared<Zombie>(700.0f, spawnY);
            m_zombies.push_back(newZ); m_Root.AddChild(newZ);
            zombieTimer = 0.0f;
        }

        // 👉 縫合組員功能：天空陽光生成
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

        // --- 4. 渲染 ---
        m_Map->Update();
        m_Root.Update();
        m_SeedBank->SetSunCount(m_SunCurrency);
        m_SeedBank->DrawUI();
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) m_CurrentState = State::END;
}

void App::UpdatePlantActions() {
    bool rowHasZombie[5] = {false, false, false, false, false};
    for (auto& zombie : m_zombies) {
        if (!zombie->IsDead()) {
            int r, c;
            if (m_Map->GetGridIndex(zombie->GetPosition(), r, c)) {
                if (r >= 0 && r < 5) rowHasZombie[r] = true;
            }
        }
    }

    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto plant = m_Map->GetPlant(r, c);
            if (!plant) continue;
            plant->Update();

            if (auto flower = std::dynamic_pointer_cast<Sunflower>(plant)) {
                if (flower->CanProduceSun()) {
                    auto s = std::make_shared<Sun>(flower->GetPosition().x, flower->GetPosition().y + 50.0f, flower->GetPosition().y - 10.0f);
                    m_Suns.push_back(s); m_Root.AddChild(s); flower->ResetSunFlag();
                }
            }
            if (auto shooter = std::dynamic_pointer_cast<Peashooter>(plant)) {
                if (rowHasZombie[r] && shooter->CanFire()) {
                    auto p = std::make_shared<Pea>(shooter->GetPosition().x + 30.0f, shooter->GetPosition().y + 35.0f);
                    m_Peas.push_back(p); m_Root.AddChild(p); shooter->ResetFireFlag();
                } else if (!rowHasZombie[r]) {
                    shooter->ResetFireFlag();
                }
            }
        }
    }
}

void App::End() { LOG_DEBUG("Game Ended."); }