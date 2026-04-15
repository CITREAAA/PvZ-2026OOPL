#include "App.hpp"
#include "pea.hpp"
#include "seedBank.hpp"
#include "Zombie.hpp"
#include "Plant.hpp"
#include "Sun.hpp"
#include "Util/Time.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include <algorithm>

void App::Start() {
    m_ImgPea = std::make_shared<Util::Image>("resources/image/peashooter/peashooter_1.png");
    m_ImgSun = std::make_shared<Util::Image>("resources/image/sunflower/1.png");
    m_ImgNut = std::make_shared<Util::Image>("resources/image/wallnut/1.png");

    m_MenuBackground = std::make_shared<Util::GameObject>();
    m_MenuBackground->SetDrawable(std::make_shared<Util::Image>("resources/image/menu/menu.png"));
    m_MenuBackground->SetZIndex(10);

    m_StartButton = std::make_shared<Util::GameObject>();
    m_StartButton->SetDrawable(std::make_shared<Util::Image>("resources/image/menu/button.png"));
    m_StartButton->m_Transform.translation = {210.0f, 130.0f};
    m_StartButton->SetZIndex(20);

    m_SelectLevelBG = std::make_shared<Util::GameObject>();
    m_SelectLevelBG->SetDrawable(std::make_shared<Util::Image>("resources/image/menu/select.jpg"));
    m_SelectLevelBG->SetZIndex(60);

    for (int i = 0; i < 10; ++i) {
        auto btn = std::make_shared<Util::GameObject>();
        btn->SetDrawable(std::make_shared<Util::Image>("resources/image/menu/level.png"));
        float x = -300.0f + (i % 5) * 150.0f;
        float y = (i < 5) ? 100.0f : -50.0f;
        btn->m_Transform.translation = {x, y};
        btn->SetZIndex(70);
        m_LevelButtons.push_back(btn);

        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(std::make_shared<Util::Text>("resources/font/impact.ttf", 30, std::to_string(i + 1), Util::Color::FromRGB(0, 0, 0, 255)));
        txtObj->m_Transform.translation = {x + 7.0f, y - 32.0f};
        txtObj->SetZIndex(71);
        m_LevelTexts.push_back(txtObj);
    }

    m_Map = std::make_shared<GameMap>("resources/image/map.jpg");
    m_Map->m_Transform.scale = {2.0f, 2.0f};
    m_Map->SetZIndex(0);

    m_SeedBank = std::make_shared<SeedBank>();
    m_DragPreview = std::make_shared<Util::GameObject>();
    m_DragPreview->SetZIndex(100);
    m_DragPreview->SetVisible(false);
    m_Root.AddChild(m_DragPreview);

    m_DefeatScreen = std::make_shared<Util::GameObject>();
    m_DefeatScreen->SetDrawable(std::make_shared<Util::Image>("resources/image/defeat.png"));
    m_DefeatScreen->m_Transform.translation = {0.0f, 0.0f};
    m_DefeatScreen->m_Transform.scale = {2.5f, 2.5f};
    m_DefeatScreen->SetVisible(true);

    m_CurrentState = State::START;
}

void App::LoadLevelConfig(int level) {
    switch (level) {
        case 1: m_CurrentLevelConfig = {15, 8.0f, 70, 20, 10}; break;
        default: m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0}; break;
    }
    m_TotalZombiesToSpawn = m_CurrentLevelConfig.totalZombies;
    m_ZombiesSpawnedInLevel = 0;
    m_ZombieSpawnTimer = 0.0f;
    m_SunCurrency = 50;
    m_StateTimer = 0.0f;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    if (m_CurrentState == State::START) {
        m_MenuBackground->Draw();
        m_StartButton->Draw();
        if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            if (mousePos.x > 50 && mousePos.x < 370 && mousePos.y > 80 && mousePos.y < 180)
                m_CurrentState = State::SELECT_LEVEL;
        }
        m_Root.Update();
    }
    else if (m_CurrentState == State::SELECT_LEVEL) {
        m_SelectLevelBG->Draw();
        for (int i = 0; i < (int)m_LevelButtons.size(); ++i) {
            if (glm::distance(mousePos, m_LevelButtons[i]->m_Transform.translation) < 60.0f) {
                m_LevelButtons[i]->m_Transform.scale = {1.1f, 1.1f};
                m_LevelTexts[i]->m_Transform.scale = {1.1f, 1.1f};
                if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
                    m_CurrentLevel = i + 1;
                    LoadLevelConfig(m_CurrentLevel);
                    m_Root.AddChild(m_Map);
                    m_CurrentState = State::UPDATE;
                }
            } else {
                m_LevelButtons[i]->m_Transform.scale = {1.0f, 1.0f};
                m_LevelTexts[i]->m_Transform.scale = {1.0f, 1.0f};
            }
            m_LevelButtons[i]->Draw();
            m_LevelTexts[i]->Draw();
        }
    }
    else if (m_CurrentState == State::UPDATE) {
        // --- 🚩 優先判斷失敗凍結狀態 ---
        bool zombieInHouse = false;
        for (auto& z : m_zombies) {
            if (z->GetPosition().x < -450.0f && !z->IsDead()) {
                zombieInHouse = true;
                break;
            }
        }

        if (zombieInHouse) {
            m_StateTimer += dt;
            // 凍結期間：只做最後的渲染渲染，不做任何移動邏輯
            m_Map->Update();
            m_Root.Update();
            m_SeedBank->DrawUI();
            if (m_StateTimer >= 2.0f) {
                m_StateTimer = 0.0f;
                m_CurrentState = State::DEFEAT;
            }
            return; // 🚩 強制結束本次 Update，達成畫面卡死效果
        }

        // --- 🚩 勝利凍結狀態 ---
        if (m_ZombiesSpawnedInLevel >= m_TotalZombiesToSpawn && m_zombies.empty()) {
            m_StateTimer += dt;
            m_Map->Update();
            m_Root.Update();
            m_SeedBank->DrawUI();
            if (m_StateTimer >= 2.0f) {
                ResetGame();
                m_CurrentState = State::SELECT_LEVEL;
            }
            return; // 🚩 強制結束本次 Update
        } else {
            m_StateTimer = 0.0f;
        }

        // --- 以下為正常遊戲邏輯更新 ---
        m_SeedBank->UpdateCooldown(dt);

        // 種植採集邏輯
        if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_SelectedPlantType != 0) {
            int r, c;
            if (m_Map->GetGridIndex(mousePos, r, c)) {
                std::shared_ptr<Plant> p = nullptr;
                if (m_SelectedPlantType == 1 && m_SunCurrency >= 100) p = std::make_shared<Peashooter>(0, 0);
                else if (m_SelectedPlantType == 2 && m_SunCurrency >= 50) p = std::make_shared<Sunflower>(0, 0);
                else if (m_SelectedPlantType == 3 && m_SunCurrency >= 50) p = std::make_shared<Wallnut>(0, 0);
                if (p && m_Map->PlacePlant(r, c, p)) {
                    m_SunCurrency -= (m_SelectedPlantType == 1) ? 100 : 50;
                    m_Root.AddChild(p);
                    m_SeedBank->StartCooldown(m_SelectedPlantType);
                }
            }
            m_SelectedPlantType = 0;
            m_DragPreview->SetVisible(false);
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            bool actionHandled = false;
            for (auto it = m_Suns.begin(); it != m_Suns.end(); ) {
                if ((*it)->IsClicked(mousePos)) {
                    m_SunCurrency += 25; (*it)->Collect(); m_Root.RemoveChild(*it);
                    it = m_Suns.erase(it); actionHandled = true; break;
                } else ++it;
            }
            if (!actionHandled && m_SelectedPlantType == 0) {
                int type = m_SeedBank->GetSelectedType(mousePos);
                if (type != 0) {
                    m_SelectedPlantType = type;
                    if (type == 1)      m_DragPreview->SetDrawable(m_ImgPea);
                    else if (type == 2) m_DragPreview->SetDrawable(m_ImgSun);
                    else if (type == 3) m_DragPreview->SetDrawable(m_ImgNut);
                    m_DragPreview->SetVisible(true);
                }
            }
        }

        if (m_SelectedPlantType != 0) {
            int r, c;
            m_DragPreview->m_Transform.translation = (m_Map->GetGridIndex(mousePos, r, c))
                ? m_Map->CalculateGridCenter(r, c) : mousePos;
        }

        UpdatePlantActions();

        for (auto& sun : m_Suns) sun->Update(dt);
        m_Suns.erase(std::remove_if(m_Suns.begin(), m_Suns.end(), [this](const std::shared_ptr<Sun>& s) {
            if (s->ShouldRemove()) { m_Root.RemoveChild(s); return true; } return false;
        }), m_Suns.end());

        for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
            (*it)->Update(dt);
            bool peaHit = false;
            for (auto& z : m_zombies) {
                if (!z->IsDead() && glm::distance((*it)->GetPosition(), z->GetPosition()) < 40.0f) {
                    z->TakeDamage(20); peaHit = true; break;
                }
            }
            if (peaHit || (*it)->IsOffScreen()) { m_Root.RemoveChild(*it); it = m_Peas.erase(it); } else ++it;
        }

        for (auto& z : m_zombies) {
            z->Update(dt);
            auto head = z->SpawnHead();
            if (head) { m_ZombieHeads.push_back(head); m_Root.AddChild(head); }

            if (z->IsDead()) continue;
            int r, c;
            if (m_Map->GetGridIndex(z->GetPosition() + glm::vec2{-25, 0}, r, c)) {
                auto p = m_Map->GetPlant(r, c);
                if (p) { z->SetState(Zombie::State::EATING); p->TakeDamage(static_cast<int>(200 * dt)); }
                else if (z->GetState() == Zombie::State::EATING) z->SetState(Zombie::State::WALKING);
            }
        }

        // 生成殭屍
        if (m_ZombiesSpawnedInLevel < m_TotalZombiesToSpawn) {
            m_ZombieSpawnTimer += dt;
            if (m_ZombieSpawnTimer > m_CurrentLevelConfig.spawnInterval) {
                int r = rand() % 5;
                int totalW = m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket;
                int randVal = rand() % (totalW > 0 ? totalW : 1);
                Zombie::Type type;
                if (randVal < m_CurrentLevelConfig.weightNormal) type = Zombie::Type::NORMAL;
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone) type = Zombie::Type::CONEHEAD;
                else type = Zombie::Type::BUCKETHEAD;

                auto newZ = std::make_shared<Zombie>(750.0f, m_Map->CalculateGridCenter(r, 8).y + 20.0f, type);
                m_zombies.push_back(newZ); m_Root.AddChild(newZ);
                m_ZombiesSpawnedInLevel++;
                m_ZombieSpawnTimer = 0.0f;
            }
        }

        static float skySunTimer = 0.0f;
        skySunTimer += dt;
        if (skySunTimer > 13.0f) {
            auto s = std::make_shared<Sun>(static_cast<float>(rand() % 611 - 430), 320.0f, static_cast<float>(rand() % 291 - 140));
            m_Suns.push_back(s); m_Root.AddChild(s);
            skySunTimer = 0.0f;
        }

        // 清理邏輯
        for (auto it = m_ZombieHeads.begin(); it != m_ZombieHeads.end(); ) {
            (*it)->Update(dt);
            if ((*it)->CanRemove()) { m_Root.RemoveChild(*it); it = m_ZombieHeads.erase(it); } else ++it;
        }
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 9; ++c) {
                auto p = m_Map->GetPlant(r, c);
                if (p && p->IsDead()) { m_Root.RemoveChild(p); m_Map->RemovePlant(r, c); }
            }
        }
        m_zombies.erase(std::remove_if(m_zombies.begin(), m_zombies.end(), [this](const std::shared_ptr<Zombie>& z) {
            if (z->CanRemove()) { m_Root.RemoveChild(z); return true; } return false;
        }), m_zombies.end());

        m_Map->Update();
        m_Root.Update();
        m_SeedBank->SetSunCount(m_SunCurrency);
        m_SeedBank->DrawUI();
    }
    else if (m_CurrentState == State::DEFEAT) {
        if (m_DefeatScreen) m_DefeatScreen->Draw();
        if (Util::Input::IsKeyDown(Util::Keycode::SPACE)) { ResetGame(); m_CurrentState = State::START; }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) m_CurrentState = State::END;
}

void App::ResetGame() {
    m_Root.RemoveChild(m_Map);
    for (auto& z : m_zombies) m_Root.RemoveChild(z);
    for (auto& p : m_Peas) m_Root.RemoveChild(p);
    for (auto& s : m_Suns) m_Root.RemoveChild(s);
    for (auto& h : m_ZombieHeads) m_Root.RemoveChild(h);
    m_zombies.clear(); m_Peas.clear(); m_Suns.clear(); m_ZombieHeads.clear();
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 9; c++) {
            auto p = m_Map->GetPlant(r, c);
            if (p) { m_Root.RemoveChild(p); m_Map->RemovePlant(r, c); }
        }
    }
    m_SunCurrency = 50;
    m_SelectedPlantType = 0;
    m_DragPreview->SetVisible(false);
    m_ZombiesSpawnedInLevel = 0;
    m_StateTimer = 0.0f;
}

void App::UpdatePlantActions() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
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
            plant->Update(dt);
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
                } else if (!rowHasZombie[r]) shooter->ResetFireFlag();
            }
        }
    }
}

void App::End() { LOG_DEBUG("Game Ended."); }