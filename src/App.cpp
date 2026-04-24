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
    // --- 載入圖片資源 ---
    m_ImgPea = std::make_shared<Util::Image>("resources/image/peashooter/peashooter_1.png");
    m_ImgSun = std::make_shared<Util::Image>("resources/image/sunflower/1.png");
    m_ImgNut = std::make_shared<Util::Image>("resources/image/wallnut/1.png");
    // 🚩 載入鏟子預覽圖
    m_ImgShovel = std::make_shared<Util::Image>("resources/image/UI/Shovel.png");

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
    m_SeedBank->InitCards({1, 2, 3});
    m_DragPreview = std::make_shared<Util::GameObject>();
    m_DragPreview->SetZIndex(100);
    m_DragPreview->SetVisible(false);
    m_Root.AddChild(m_DragPreview);

    m_DefeatScreen = std::make_shared<Util::GameObject>();
    m_DefeatScreen->SetDrawable(std::make_shared<Util::Image>("resources/image/defeat.png"));
    m_DefeatScreen->m_Transform.translation = {0.0f, 0.0f};
    m_DefeatScreen->m_Transform.scale = {2.5f, 2.5f};

    // --- 🎵 載入所有音訊 ---
    m_MenuBGM = std::make_shared<Util::BGM>("resources/music/startUIBgm.mp3");
    m_GameBGM = std::make_shared<Util::BGM>("resources/music/gamingBgm2.mp3");
    m_SunCollectSFX = std::make_shared<Util::SFX>("resources/music/collectSunshine.wav");
    m_PlantSeedSFX = std::make_shared<Util::SFX>("resources/music/cardLift.wav");
    m_PeaHitSFX = std::make_shared<Util::SFX>("resources/music/hit3.wav");
    m_DefeatSFX = std::make_shared<Util::SFX>("resources/music/gameLose.wav");

    if (m_MenuBGM) m_MenuBGM->Play(-1);
    m_CurrentState = State::START;
}

void App::LoadLevelConfig(int level) {
    std::vector<int> allowed;
    switch (level) {
        case 1:
            allowed = {1, 2, 3};
            m_CurrentLevelConfig = {15, 8.0f, 70, 20, 10,allowed}; break;
        case 2:
            allowed = {1, 2, 3, 5};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 3:
            allowed = {1, 2, 3, 5, 6};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 4:
            allowed = {1, 2, 3, 5, 6};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 5:
            allowed = {1, 2, 3, 5, 6, 7};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 6:
            allowed = {1, 8, 9, 10, 3, 7};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 7:
            allowed = {1, 8, 9, 10, 3, 7};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 8:
            allowed = {1, 8, 6, 10, 3, 7};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 9:
            allowed = {1, 8, 6, 10, 3, 7, 11};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
        case 10:
            allowed = {8, 6, 10, 3, 7, 11, 12};
            m_CurrentLevelConfig = {2, 8.0f, 100, 0, 0,allowed}; break;
    }
    m_SeedBank->InitCards(allowed);
    m_TotalZombiesToSpawn = m_CurrentLevelConfig.totalZombies;
    m_ZombiesSpawnedInLevel = 0;
    m_ZombieSpawnTimer = 0.0f;
    m_SunCurrency = 50;
    m_StateTimer = 0.0f;
}

void App::Update() {
    glm::vec2 mousePos = Util::Input::GetCursorPosition();
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    // --- [ 狀態 A: 首頁 ] ---
    if (m_CurrentState == State::START) {
        m_MenuBackground->Draw();
        m_StartButton->Draw();
        if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            if (mousePos.x > 50 && mousePos.x < 370 && mousePos.y > 80 && mousePos.y < 180)
                m_CurrentState = State::SELECT_LEVEL;
        }
        m_Root.Update();
    }
    // --- [ 狀態 B: 選關 ] ---
    else if (m_CurrentState == State::SELECT_LEVEL) {
        m_SelectLevelBG->Draw();
        for (int i = 0; i < (int)m_LevelButtons.size(); ++i) {
            if (glm::distance(mousePos, m_LevelButtons[i]->m_Transform.translation) < 60.0f) {
                m_LevelButtons[i]->m_Transform.scale = {1.1f, 1.1f};
                m_LevelTexts[i]->m_Transform.scale = {1.1f, 1.1f};
                if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
                    m_CurrentLevel = i + 1;
                    LoadLevelConfig(m_CurrentLevel);
                    ResetGame();
                    m_Root.AddChild(m_Map);
                    m_CurrentState = State::UPDATE;
                    if (m_MenuBGM) m_MenuBGM->Pause();
                    if (m_GameBGM) m_GameBGM->Play(-1);
                }
            } else {
                m_LevelButtons[i]->m_Transform.scale = {1.0f, 1.0f};
                m_LevelTexts[i]->m_Transform.scale = {1.0f, 1.0f};
            }
            m_LevelButtons[i]->Draw();
            m_LevelTexts[i]->Draw();
        }
    }
    // --- [ 狀態 C: 遊戲更新 ] ---
    else if (m_CurrentState == State::UPDATE) {


        // --- 🚩 處理除草機移動與殺殭屍 ---
        for (auto& mower : m_LawnMowers) {
            if (mower->state == LawnMowerData::State::MOVING) {
                mower->obj->m_Transform.translation.x += mower->speed * dt;
                mower->obj->Draw();

                // 檢查該行所有殭屍，若碰到移動中的除草機則秒殺
                for (auto& z : m_zombies) {
                    if (!z->IsDead() && std::abs(z->GetPosition().y - mower->obj->m_Transform.translation.y) < 50.0f) {
                        if (std::abs(z->GetPosition().x - mower->obj->m_Transform.translation.x) < 60.0f) {
                            z->TakeDamage(9999); // 秒殺殭屍
                        }
                    }
                }
                // 跑出螢幕邊界則標記移除
                if (mower->obj->m_Transform.translation.x > 600.0f) mower->state = LawnMowerData::State::REMOVED;
            }
            else if (mower->state == LawnMowerData::State::IDLE) {
                mower->obj->Draw(); // 靜止狀態也要繪製
            }
        }

        // 🚩 增加：清理跑出螢幕的除草機
        m_LawnMowers.erase(std::remove_if(m_LawnMowers.begin(), m_LawnMowers.end(),
            [this](const std::shared_ptr<LawnMowerData>& m){
                if (m->state == LawnMowerData::State::REMOVED) {
                    m_Root.RemoveChild(m->obj); return true;
                }
                return false;
            }), m_LawnMowers.end());


        bool zombieInHouse = false;
        for (auto& z : m_zombies) {
            if (z->IsDead()) continue;

            // 🚩 修正點 1：將觸發界線從 -480 挪到 -400 試試看
            // 這能確保殭屍還沒進屋前就先觸發除草機
            if (z->GetPosition().x < -430.0f) {
                int r, c;
                m_Map->GetGridIndex(z->GetPosition(), r, c);

                // 🚩 修正點 2：不要只依賴 GetGridIndex，直接用 Y 軸距離來找除草機
                // 這樣可以避免地圖索引計算誤差
                auto it = std::find_if(m_LawnMowers.begin(), m_LawnMowers.end(),
                    [&](const std::shared_ptr<LawnMowerData>& m) {
                        return m->state == LawnMowerData::State::IDLE &&
                               std::abs(m->obj->m_Transform.translation.y - z->GetPosition().y) < 60.0f;
                    });

                if (it != m_LawnMowers.end()) {
                    (*it)->state = LawnMowerData::State::MOVING;
                    if (m_LawnMowerSFX) m_LawnMowerSFX->Play();
                    LOG_DEBUG("除草機第 {} 排已觸發！", (*it)->row);
                } else {
                    // 🚩 修正點 3：只有當「真的越過最後防線 (-500)」且「該行沒除草機」才算輸
                    if (z->GetPosition().x < -470.0f) {
                        zombieInHouse = true;
                        break;
                    }
                }
            }
        }

        if (zombieInHouse) {
            if (m_StateTimer == 0.0f) {
                if (m_DefeatSFX) m_DefeatSFX->Play();
                if (m_GameBGM) m_GameBGM->Pause();
            }
            m_StateTimer += dt;
            m_Map->Update();
            m_Root.Update();
            m_SeedBank->DrawUI();
            if (m_StateTimer >= 1.0f) {
                m_StateTimer = 0.0f;
                m_CurrentState = State::DEFEAT;
            }
            return;
        }

        // --- 🚩 勝利判定與凍結時間 ---
        if (m_ZombiesSpawnedInLevel >= m_TotalZombiesToSpawn && m_zombies.empty()) {
            m_StateTimer += dt;
            m_Map->Update();
            m_Root.Update();
            m_SeedBank->DrawUI();
            if (m_StateTimer >= 1.0f) {
                ResetGame();
                m_CurrentState = State::SELECT_LEVEL;
            }
            return;
        }

        // --- 遊戲邏輯更新 ---
        m_SeedBank->UpdateCooldown(dt);

        // --- 🚩 放開滑鼠鍵：處理種植或剷除 ---
        if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_SelectedPlantType != 0) {
            int r, c;
            if (m_Map->GetGridIndex(mousePos, r, c)) {
                if (m_SelectedPlantType == 4) {
                    // 🚩 鏟子邏輯
                    auto p = m_Map->GetPlant(r, c);
                    if (p) {
                        m_Root.RemoveChild(p);
                        m_Map->RemovePlant(r, c);
                        LOG_DEBUG("Plant shoveled at {}, {}", r, c);
                    }
                } else {
                    // 植物種植邏輯
                    std::shared_ptr<Plant> p = nullptr;
                    if (m_SelectedPlantType == 1 && m_SunCurrency >= 100) p = std::make_shared<Peashooter>(0, 0);
                    else if (m_SelectedPlantType == 2 && m_SunCurrency >= 50) p = std::make_shared<Sunflower>(0, 0);
                    else if (m_SelectedPlantType == 3 && m_SunCurrency >= 50) p = std::make_shared<Wallnut>(0, 0);
                    if (p && m_Map->PlacePlant(r, c, p)) {
                        m_SunCurrency -= (m_SelectedPlantType == 1) ? 100 : 50;
                        m_Root.AddChild(p);
                        m_SeedBank->StartCooldown(m_SelectedPlantType);
                        if (m_PlantSeedSFX) m_PlantSeedSFX->Play();
                    }
                }
            }
            // 🚩 無論如何都要還原鏟子與狀態
            m_SelectedPlantType = 0;
            m_DragPreview->SetVisible(false);
            m_SeedBank->SetShovelVisible(true);
        }
        // --- 🚩 按下滑鼠鍵：選擇植物或鏟子 ---
        else if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            bool actionHandled = false;
            // 優先檢查陽光採集
            for (auto it = m_Suns.begin(); it != m_Suns.end(); ) {
                if ((*it)->IsClicked(mousePos)) {
                    m_SunCurrency += 25; (*it)->Collect(); m_Root.RemoveChild(*it);
                    if (m_SunCollectSFX) m_SunCollectSFX->Play();
                    it = m_Suns.erase(it); actionHandled = true; break;
                } else ++it;
            }
            // 選擇工具欄物件
            if (!actionHandled && m_SelectedPlantType == 0) {
                int type = m_SeedBank->GetSelectedType(mousePos);
                if (type != 0) {
                    m_SelectedPlantType = type;
                    if (type == 1)      m_DragPreview->SetDrawable(m_ImgPea);
                    else if (type == 2) m_DragPreview->SetDrawable(m_ImgSun);
                    else if (type == 3) m_DragPreview->SetDrawable(m_ImgNut);
                    else if (type == 4) {
                        m_DragPreview->SetDrawable(m_ImgShovel);
                        m_SeedBank->SetShovelVisible(false); // 🚩 拿起鏟子，工具欄變空
                    }
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
                    z->TakeDamage(20);
                    if (m_PeaHitSFX) m_PeaHitSFX->Play();
                    peaHit = true; break;
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
                if (p) {
                    z->SetState(Zombie::State::EATING);
                    p->TakeDamage(static_cast<int>(z->GetAttackPower() * dt));
                }
                else if (z->GetState() == Zombie::State::EATING) z->SetState(Zombie::State::WALKING);
            }
        }

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
    // --- [ 狀態 D: 失敗畫面 ] ---
    else if (m_CurrentState == State::DEFEAT) {
        if (m_DefeatScreen) m_DefeatScreen->Draw();
        m_StateTimer += dt;
        if (m_StateTimer >= 3.0f) {
            ResetGame();
            m_CurrentState = State::SELECT_LEVEL;
        }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) m_CurrentState = State::END;
}

void App::ResetGame() {

    for (auto& m : m_LawnMowers) m_Root.RemoveChild(m->obj);
    m_LawnMowers.clear();

    for (int r = 0; r < 5; ++r) {
        auto mowerData = std::make_shared<LawnMowerData>();
        mowerData->obj = std::make_shared<Util::GameObject>();
        mowerData->obj->SetDrawable(std::make_shared<Util::Image>("resources/image/weeder.png"));

        float posY = m_Map->CalculateGridCenter(r, 0).y + 20.0f;

        // 設定在螢幕左側外緣，對齊每一行的 Y 軸中心
        mowerData->obj->m_Transform.translation = {-450.0f, m_Map->CalculateGridCenter(r, 0).y};
        mowerData->obj->SetZIndex(45); // 層級在殭屍下方
        mowerData->row = r;
        mowerData->state = LawnMowerData::State::IDLE;

        m_LawnMowers.push_back(mowerData);
        m_Root.AddChild(mowerData->obj);
    }



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
    m_SunCurrency = 50; m_SelectedPlantType = 0;
    m_DragPreview->SetVisible(false);
    m_ZombiesSpawnedInLevel = 0; m_StateTimer = 0.0f;
    m_SeedBank->SetShovelVisible(true); // 🚩 重置時確保鏟子顯示
    if (m_GameBGM) m_GameBGM->Pause();
    if (m_MenuBGM) m_MenuBGM->Play(-1);
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