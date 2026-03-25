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
    m_Map = std::make_shared<GameMap>("resources/image/map.jpg");
    m_Map->m_Transform.translation = {0.0f, 0.0f};
    m_Map->m_Transform.scale = {2.0f, 2.0f};
    m_Root.AddChild(m_Map);

    m_SeedBank = std::make_shared<SeedBank>();

    m_DragPreview = std::make_shared<Util::GameObject>();
    m_DragPreview->SetZIndex(100);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    m_SeedBank->UpdateCooldown(dt);

    // --- 1. 輸入處理 (點擊與拖曳) ---
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        bool actionHandled = false;
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

        if (!actionHandled && m_SelectedPlantType == 0) {
            int type = m_SeedBank->GetSelectedType(mousePos);
            if (type != 0) {
                m_SelectedPlantType = type;
                std::string imgPath;
                if (type == 1) imgPath = "resources/image/peashooter/peashooter_1.png";
                else if (type == 2) imgPath = "resources/image/sunflower/1.png";
                else if (type == 3) imgPath = "resources/image/wallnut/1.png";
                m_DragPreview->SetDrawable(std::make_shared<Util::Image>(imgPath));
            }
        }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_SelectedPlantType != 0) {
        int r, c;
        if (m_Map->GetGridIndex(mousePos, r, c)) {
            if (m_SelectedPlantType == 1 && m_SunCurrency >= 100) {
                auto newPea = std::make_shared<Peashooter>(0.0f, 0.0f);
                if (m_Map->PlacePlant(r, c, newPea)) {
                    m_SunCurrency -= 100;
                    m_SeedBank->StartCooldown(1);
                }
            } else if (m_SelectedPlantType == 2 && m_SunCurrency >= 50) {
                auto newFlower = std::make_shared<Sunflower>(0.0f, 0.0f);
                if (m_Map->PlacePlant(r, c, newFlower)) {
                    m_SunCurrency -= 50;
                    m_SeedBank->StartCooldown(2);
                }
            } else if (m_SelectedPlantType == 3 && m_SunCurrency >= 50) {
                auto newWallnut = std::make_shared<Wallnut>(0.0f, 0.0f);
                if (m_Map->PlacePlant(r, c, newWallnut)) {
                    m_SunCurrency -= 50;
                    m_SeedBank->StartCooldown(3);
                }
            }
        }
        m_SelectedPlantType = 0;
        m_DragPreview->SetDrawable(nullptr);
    }

    // --- 2. 邏輯更新 ---
    for (auto& sun : m_Suns) sun->Update();
    for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
        (*it)->Update();
        bool peaHit = false;
        for (auto& zombie : m_zombies) {
            if (zombie->IsDead()) continue;
            if (glm::distance((*it)->GetPosition(), zombie->GetPosition()) < 40.0f) {
                zombie->TakeDamage(20);
                peaHit = true;
                break;
            }
        }
        if (peaHit || (*it)->IsOffScreen()) {
            m_Root.RemoveChild(*it);
            it = m_Peas.erase(it);
        } else { ++it; }
    }

    for (auto& zombie : m_zombies) {
        zombie->Update();
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
            }
        }
        // 若沒在啃食但狀態是 EATING，切回 WALKING
        if (zombie->GetState() == Zombie::State::EATING) {
            int r2, c2;
            if (!m_Map->GetGridIndex(zombie->GetPosition() + glm::vec2{-25, 0}, r2, c2) || !m_Map->GetPlant(r2, c2)) {
                 zombie->SetState(Zombie::State::WALKING);
            }
        }
    }

    m_zombies.erase(std::remove_if(m_zombies.begin(), m_zombies.end(),
        [this](const std::shared_ptr<Zombie>& z) {
            if (z->CanRemove()) {
                m_Root.RemoveChild(z);
                return true;
            }
            return false;
        }), m_zombies.end());

    static float zombieTimer = 0.0f;
    zombieTimer += dt;
    if (zombieTimer > 10.0f) {
        int r = rand() % 5;
        float spawnY = m_Map->CalculateGridCenter(r, 8).y;
        auto newZ = std::make_shared<Zombie>(650.0f, spawnY);
        m_zombies.push_back(newZ);
        m_Root.AddChild(newZ);
        zombieTimer = 0.0f;
    }

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

    UpdatePlantActions();

    m_Map->Update();
    m_Root.Update();
    m_SeedBank->SetSunCount(m_SunCurrency);
    m_SeedBank->DrawUI();

    if (m_SelectedPlantType != 0) {
        int r, c;
        if (m_Map->GetGridIndex(mousePos, r, c)) m_DragPreview->m_Transform.translation = m_Map->CalculateGridCenter(r, c);
        else m_DragPreview->m_Transform.translation = mousePos;
        m_DragPreview->Draw();
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) m_CurrentState = State::END;
}

// --- 核心修改部分 ---
void App::UpdatePlantActions() {
    // 1. 偵測每一排是否有「活著」的殭屍
    bool rowHasZombie[5] = {false, false, false, false, false};
    for (auto& zombie : m_zombies) {
        // 只有沒死的殭屍才算目標
        if (!zombie->IsDead()) {
            int r, c;
            // 透過地圖函式取得殭屍所在的排數
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

            // 向日葵產陽光 (不受殭屍影響)
            auto flower = std::dynamic_pointer_cast<Sunflower>(plant);
            if (flower && flower->CanProduceSun()) {
                auto s = std::make_shared<Sun>(flower->GetPosition().x,
                                               flower->GetPosition().y + 50.0f,
                                               flower->GetPosition().y - 10.0f);
                m_Suns.push_back(s);
                m_Root.AddChild(s);
                flower->ResetSunFlag();
            }

            // 豌豆射手射擊 (檢查該排有無殭屍)
            auto shooter = std::dynamic_pointer_cast<Peashooter>(plant);
            if (shooter) {
                if (rowHasZombie[r]) {
                    if (shooter->CanFire()) {
                        auto p = std::make_shared<Pea>(shooter->GetPosition().x + 30.0f,
                                                      shooter->GetPosition().y + 20.0f);
                        m_Peas.push_back(p);
                        m_Root.AddChild(p);
                        shooter->ResetFireFlag();
                    }
                } else {
                    // 如果該排沒殭屍，強行重置發射旗標，確保殭屍死後子彈不會剛好噴出來
                    shooter->ResetFireFlag();
                }
            }
        }
    }
}

void App::End() {
    LOG_TRACE("End");
}