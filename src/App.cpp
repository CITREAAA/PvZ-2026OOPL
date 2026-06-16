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
    // 圖片預載
    m_ImgPea        = std::make_shared<Util::Image>("resources/image/peashooter/peashooter_1.png");
    m_ImgSun        = std::make_shared<Util::Image>("resources/image/sunflower/1.png");
    m_ImgNut        = std::make_shared<Util::Image>("resources/image/wallnut/1.png");
    m_ImgShovel     = std::make_shared<Util::Image>("resources/image/UI/Shovel.png");
    m_ImgPotatoMine = std::make_shared<Util::Image>("resources/image/potatomine/underground/underground.png");
    m_ImgSnowPea    = std::make_shared<Util::Image>("resources/image/snowpea/1.png");
    m_ImgSunShroom  = std::make_shared<Util::Image>("resources/image/sunshroom/1.png");
    m_ImgPuff       = std::make_shared<Util::Image>("resources/image/puffshroom/1.png");
    m_ImgFume       = std::make_shared<Util::Image>("resources/image/fumeshroom/1.png");
    m_ImgScaredy    = std::make_shared<Util::Image>("resources/image/scaredyshroom/1.png");
    m_ImgCherry     = std::make_shared<Util::Image>("resources/image/cherrybomb/1.png");
    m_ImgRepeater   = std::make_shared<Util::Image>("resources/image/repeaterpea/1.png");

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

    m_MenuBGM       = std::make_shared<Util::BGM>("resources/music/startUIBgm.mp3");
    m_GameBGM       = std::make_shared<Util::BGM>("resources/music/gamingBgm2.mp3");
    m_SunCollectSFX = std::make_shared<Util::SFX>("resources/music/collectSunshine.wav");
    m_PlantSeedSFX  = std::make_shared<Util::SFX>("resources/music/cardLift.wav");
    m_PeaHitSFX     = std::make_shared<Util::SFX>("resources/music/hit3.wav");
    m_DefeatSFX     = std::make_shared<Util::SFX>("resources/music/gameLose.wav");
    m_ExplodeSFX    = std::make_shared<Util::SFX>("resources/music/cherrybomb.wav");

    if (m_MenuBGM) m_MenuBGM->Play(-1);
    m_CurrentState = State::START;
}

void App::LoadLevelConfig(int level) {
    // 🚩 修正：先建立新地圖與配置
    std::string mapPath = (level >= 6) ? "resources/image/map_night.jpg" : "resources/image/map.jpg";
    m_Map = std::make_shared<GameMap>(mapPath);
    m_Map->m_Transform.scale = {2.0f, 2.0f};
    m_Map->SetZIndex(0);

    // 🚩 修正：建立除草機（因為現在有了新地圖的座標）
    for (int r = 0; r < 5; ++r) {
        auto mower = std::make_shared<LawnMowerData>();
        mower->obj = std::make_shared<Util::GameObject>();
        mower->obj->SetDrawable(std::make_shared<Util::Image>("resources/image/weeder.png"));
        mower->obj->m_Transform.translation = {-450.0f, m_Map->CalculateGridCenter(r, 0).y};
        mower->obj->SetZIndex(45);
        mower->row   = r;
        mower->state = LawnMowerData::State::IDLE;
        m_LawnMowers.push_back(mower);
        m_Root.AddChild(mower->obj);
    }

    std::vector<int> allowed;
    switch (level) {
        case 1:  allowed = {1,2,3};            m_CurrentLevelConfig = {15,8.0f,70,20,10, 0, 0, 0,allowed}; break;
        case 2:  allowed = {1,2,3,5};          m_CurrentLevelConfig = {20,8.0f,70,10,10,10, 0, 0,allowed}; break;
        case 3:  allowed = {1,2,3,5,6};        m_CurrentLevelConfig = {25,8.0f,50,20,20,10, 0, 0,allowed}; break;
        case 4:  allowed = {1,2,3,5,6};        m_CurrentLevelConfig = {30,8.0f,40,25,25,10, 0, 0,allowed}; break;
        case 5:  allowed = {1,2,3,5,6,7};      m_CurrentLevelConfig = {40,8.0f,50,15,15,10,10, 0,allowed}; break;
        case 6:  allowed = {1,8,9,10,3,7};     m_CurrentLevelConfig = {15,8.0f,50,15,15,10,10, 0,allowed}; break;
        case 7:  allowed = {1,8,9,10,3,7};     m_CurrentLevelConfig = {20,8.0f,50,15,15,10,10, 0,allowed}; break;
        case 8:  allowed = {1,8,6,10,3,7};     m_CurrentLevelConfig = {25,8.0f,50,15,15,10,10, 0,allowed}; break;
        case 9:  allowed = {1,8,6,10,3,7,11};  m_CurrentLevelConfig = {30,8.0f,50,10,10,10,10,10,allowed}; break;
        case 10: allowed = {8,6,10,3,7,11,12}; m_CurrentLevelConfig = {40,8.0f,30,10,15,15,15,15,allowed}; break;
        default: allowed = {1,2,3};            m_CurrentLevelConfig = {2,8.0f,100,0,0, 0, 0, 0,allowed}; break;
    }
    m_SeedBank->InitCards(allowed);
    m_TotalZombiesToSpawn   = m_CurrentLevelConfig.totalZombies;
    m_ZombiesSpawnedInLevel = 0;
    m_ZombieSpawnTimer      = 0.0f;
    m_SunCurrency           = 50;
    m_StateTimer            = 0.0f;
}

void App::ResetGame() {
    // 🚩 最重要的 Bug 修復點：確實把舊地圖與植物從 Root 中拔除
    if (m_Map) {
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 9; c++) {
                auto p = m_Map->GetPlant(r, c);
                if (p) { m_Root.RemoveChild(p); m_Map->RemovePlant(r, c); }
            }
        }
        m_Root.RemoveChild(m_Map);
    }

    for (auto& z : m_zombies) m_Root.RemoveChild(z);
    for (auto& p : m_Peas) m_Root.RemoveChild(p);
    for (auto& s : m_Suns) m_Root.RemoveChild(s);
    for (auto& h : m_ZombieHeads) m_Root.RemoveChild(h);
    for (auto& m : m_LawnMowers) m_Root.RemoveChild(m->obj);

    m_zombies.clear(); m_Peas.clear(); m_Suns.clear(); m_ZombieHeads.clear(); m_LawnMowers.clear();

    m_SunCurrency = 50;
    m_SelectedPlantType = 0;
    m_ZombiesSpawnedInLevel = 0;
    m_StateTimer = 0.0f;
    if (m_DragPreview) m_DragPreview->SetVisible(false);

    if (m_GameBGM) m_GameBGM->Pause();
    if (m_MenuBGM) m_MenuBGM->Play(-1);
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
                    ResetGame();                     // 1. 確保舊東西清空
                    LoadLevelConfig(m_CurrentLevel); // 2. 生成新地圖與新除草機
                    m_Root.AddChild(m_Map);          // 3. 放進場景
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
    else if (m_CurrentState == State::UPDATE) {
        // --- 作弊碼 ---
        if (Util::Input::IsKeyDown(Util::Keycode::F1)) { m_SunCurrency += 1000; if (m_SunCollectSFX) m_SunCollectSFX->Play(); }
        if (Util::Input::IsKeyDown(Util::Keycode::F2)) { for (auto& z : m_zombies) if (!z->IsDead()) z->TakeDamage(9999); if (m_ExplodeSFX) m_ExplodeSFX->Play(); }
        if (Util::Input::IsKeyDown(Util::Keycode::F3)) { m_ZombiesSpawnedInLevel = m_TotalZombiesToSpawn; for (auto& z : m_zombies) z->TakeDamage(9999); }
        if (Util::Input::IsKeyDown(Util::Keycode::F4)) { if (m_SeedBank) m_SeedBank->ResetAllCooldowns(); if (m_PlantSeedSFX) m_PlantSeedSFX->Play(); }

        // --- 優先判斷失敗凍結狀態 ---
        bool zombieInHouse = false;
        for (auto& z : m_zombies) {
            if (z->IsDead()) continue;
            if (z->GetPosition().x < -430.0f) {
                auto it = std::find_if(m_LawnMowers.begin(), m_LawnMowers.end(),
                    [&](const std::shared_ptr<LawnMowerData>& m) {
                        return m->state == LawnMowerData::State::IDLE &&
                               std::abs(m->obj->m_Transform.translation.y - z->GetPosition().y) < 60.0f;
                    });
                if (it != m_LawnMowers.end()) {
                    (*it)->state = LawnMowerData::State::MOVING;
                    if (m_LawnMowerSFX) m_LawnMowerSFX->Play();
                } else if (z->GetPosition().x < -470.0f) {
                    zombieInHouse = true; break;
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
            if (m_StateTimer >= 2.0f) {
                m_StateTimer = 0.0f;
                m_CurrentState = State::DEFEAT;
            }
            return; // 達成畫面卡死
        }

        // --- 勝利凍結狀態 ---
        if (m_ZombiesSpawnedInLevel >= m_TotalZombiesToSpawn && m_zombies.empty()) {
            m_StateTimer += dt;
            m_Map->Update();
            m_Root.Update();
            m_SeedBank->DrawUI();
            if (m_StateTimer >= 2.0f) {
                ResetGame();
                m_CurrentState = State::SELECT_LEVEL;
            }
            return;
        } else {
            m_StateTimer = 0.0f;
        }

        // --- 正常遊戲邏輯 ---
        m_SeedBank->UpdateCooldown(dt);

        // 種植採集邏輯
        if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB) && m_SelectedPlantType != 0) {
            ExecutePlanting(mousePos);
            m_SelectedPlantType = 0;
            m_DragPreview->SetVisible(false);
            m_SeedBank->SetShovelVisible(true);
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            bool actionHandled = false;
            for (auto it = m_Suns.begin(); it != m_Suns.end(); ) {
                if ((*it)->IsClicked(mousePos)) {
                    m_SunCurrency += (*it)->GetSunValue();
                    (*it)->Collect(); m_Root.RemoveChild(*it);
                    if (m_SunCollectSFX) m_SunCollectSFX->Play();
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
                    else if (type == 4) m_DragPreview->SetDrawable(m_ImgShovel);
                    else if (type == 5) m_DragPreview->SetDrawable(m_ImgPotatoMine);
                    else if (type == 6) m_DragPreview->SetDrawable(m_ImgSnowPea);
                    else if (type == 7) m_DragPreview->SetDrawable(m_ImgCherry);
                    else if (type == 8) m_DragPreview->SetDrawable(m_ImgSunShroom);
                    else if (type == 9) m_DragPreview->SetDrawable(m_ImgPuff);
                    else if (type == 10) m_DragPreview->SetDrawable(m_ImgFume);
                    else if (type == 11) m_DragPreview->SetDrawable(m_ImgScaredy);
                    else if (type == 12) m_DragPreview->SetDrawable(m_ImgRepeater);
                    m_DragPreview->SetVisible(true);
                    if (type == 4) m_SeedBank->SetShovelVisible(false);
                }
            }
        }

        if (m_SelectedPlantType != 0) {
            int r, c;
            m_DragPreview->m_Transform.translation = (m_Map->GetGridIndex(mousePos, r, c))
                ? m_Map->CalculateGridCenter(r, c) : mousePos;
        }

        UpdateLawnMowers(dt);
        UpdatePlantActions();
        UpdateSuns(dt);
        UpdateProjectiles(dt);
        UpdateZombies(dt);

        // 生成殭屍
        if (m_ZombiesSpawnedInLevel < m_TotalZombiesToSpawn) {
            m_ZombieSpawnTimer += dt;
            if (m_ZombieSpawnTimer > m_CurrentLevelConfig.spawnInterval) {
                int r = rand() % 5;
                int totalW = m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket + m_CurrentLevelConfig.weightPole + m_CurrentLevelConfig.weightFootball + m_CurrentLevelConfig.weightScreenDoor;
                int randVal = rand() % (totalW > 0 ? totalW : 1);
                Zombie::Type type;
                if      (randVal < m_CurrentLevelConfig.weightNormal) type = Zombie::Type::NORMAL;
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone) type = Zombie::Type::CONEHEAD;
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket) type = Zombie::Type::BUCKETHEAD;
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket + m_CurrentLevelConfig.weightPole) type = Zombie::Type::POLEVAULTER;
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket + m_CurrentLevelConfig.weightPole + m_CurrentLevelConfig.weightFootball) type = Zombie::Type::FOOTBALL;
                else type = Zombie::Type::SCREENDOOR;

                float spawnY = m_Map->CalculateGridCenter(r, 8).y + 20.0f;
                if (type == Zombie::Type::POLEVAULTER) spawnY -= 10.0f;

                auto newZ = std::make_shared<Zombie>(750.0f, spawnY, type);
                m_zombies.push_back(newZ); m_Root.AddChild(newZ);
                m_ZombiesSpawnedInLevel++;
                m_ZombieSpawnTimer = 0.0f;
            }
        }

        // 天降陽光
        if (m_CurrentLevel < 6) {
            static float skySunTimer = 0.0f;
            skySunTimer += dt;
            if (skySunTimer > 13.0f) {
                auto s = std::make_shared<Sun>(static_cast<float>(rand() % 611 - 430), 320.0f, static_cast<float>(rand() % 291 - 140));
                m_Suns.push_back(s); m_Root.AddChild(s);
                skySunTimer = 0.0f;
            }
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
    else if (m_CurrentState == State::DEFEAT) {
        // 1. 畫出黑屏與失敗圖片
        if (m_DefeatScreen) m_DefeatScreen->Draw();

        // 2. 累積經過的時間
        m_StateTimer += dt;

        // 3. 如果滿 3 秒了，就自動回到選關畫面
        if (m_StateTimer >= 3.0f) {
            ResetGame();                               // 清除場上所有東西
            m_StateTimer = 0.0f;                       // 計時器歸零備用
            m_CurrentState = State::SELECT_LEVEL;      // 切換回選關狀態
        }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) m_CurrentState = State::END;
}

// =============================================================================
// [ 內部獨立系統實作 ]
// =============================================================================

void App::ExecutePlanting(glm::vec2 mousePos) {
    int r, c;
    if (!m_Map->GetGridIndex(mousePos, r, c)) return;

    if (m_SelectedPlantType == 4) {
        auto p = m_Map->GetPlant(r, c);
        if (p) { m_Root.RemoveChild(p); m_Map->RemovePlant(r, c); }
        return;
    }

    std::shared_ptr<Plant> p = nullptr;
    int cost = 0;
    if      (m_SelectedPlantType == 1  && m_SunCurrency >= 100) { p = std::make_shared<Peashooter>(0,0);   cost = 100; }
    else if (m_SelectedPlantType == 2  && m_SunCurrency >= 50)  { p = std::make_shared<Sunflower>(0,0);    cost = 50;  }
    else if (m_SelectedPlantType == 3  && m_SunCurrency >= 50)  { p = std::make_shared<Wallnut>(0,0);      cost = 50;  }
    else if (m_SelectedPlantType == 5  && m_SunCurrency >= 25)  { p = std::make_shared<PotatoMine>(0,0);   cost = 25;  }
    else if (m_SelectedPlantType == 6  && m_SunCurrency >= 175) { p = std::make_shared<SnowPea>(0,0);      cost = 175; }
    else if (m_SelectedPlantType == 7  && m_SunCurrency >= 150) { p = std::make_shared<CherryBomb>(0,0);   cost = 150; }
    else if (m_SelectedPlantType == 8  && m_SunCurrency >= 25)  { p = std::make_shared<SunShroom>(0,0);    cost = 25;  }
    else if (m_SelectedPlantType == 9  && m_SunCurrency >= 0)   { p = std::make_shared<PuffShroom>(0,0);   cost = 0;   }
    else if (m_SelectedPlantType == 10 && m_SunCurrency >= 75)  { p = std::make_shared<FumeShroom>(0,0);   cost = 75;  }
    else if (m_SelectedPlantType == 11 && m_SunCurrency >= 25)  { p = std::make_shared<ScaredyShroom>(0,0);cost = 25;  }
    else if (m_SelectedPlantType == 12 && m_SunCurrency >= 200) { p = std::make_shared<Repeater>(0,0);     cost = 200; }

    if (p && m_Map->PlacePlant(r, c, p)) {
        m_SunCurrency -= cost;
        m_Root.AddChild(p);
        m_SeedBank->StartCooldown(m_SelectedPlantType);
        if (m_PlantSeedSFX) m_PlantSeedSFX->Play();
    }
}

void App::UpdateLawnMowers(float dt) {
    for (auto& mower : m_LawnMowers) {
        if (mower->state == LawnMowerData::State::MOVING) {
            mower->obj->m_Transform.translation.x += mower->speed * dt;
            mower->obj->Draw();
            for (auto& z : m_zombies) {
                if (!z->IsDead() && std::abs(z->GetPosition().y - mower->obj->m_Transform.translation.y) < 50.0f &&
                    std::abs(z->GetPosition().x - mower->obj->m_Transform.translation.x) < 60.0f) { z->TakeDamage(9999); }
            }
            if (mower->obj->m_Transform.translation.x > 600.0f) mower->state = LawnMowerData::State::REMOVED;
        } else if (mower->state == LawnMowerData::State::IDLE) {
            mower->obj->Draw();
        }
    }
    m_LawnMowers.erase(std::remove_if(m_LawnMowers.begin(), m_LawnMowers.end(),
        [this](const std::shared_ptr<LawnMowerData>& m) {
            if (m->state == LawnMowerData::State::REMOVED) { m_Root.RemoveChild(m->obj); return true; } return false;
        }), m_LawnMowers.end());
}

void App::UpdatePlantActions() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    bool rowHasZombie[5] = {};
    for (auto& z : m_zombies) {
        if (z->IsDead()) continue;
        int r, c;
        if (m_Map->GetGridIndex(z->GetPosition(), r, c) && r >= 0 && r < 5) rowHasZombie[r] = true;
    }

    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto plant = m_Map->GetPlant(r, c);
            if (!plant) continue;
            plant->Update(dt);

            if (auto flower = std::dynamic_pointer_cast<Sunflower>(plant)) {
                if (flower->CanProduceSun()) {
                    auto s = std::make_shared<Sun>(flower->GetPosition().x, flower->GetPosition().y+50.0f, flower->GetPosition().y-10.0f);
                    m_Suns.push_back(s); m_Root.AddChild(s); flower->ResetSunFlag();
                }
            } else if (auto shroom = std::dynamic_pointer_cast<SunShroom>(plant)) {
                if (shroom->CanProduceSun()) {
                    auto s = std::make_shared<Sun>(shroom->GetPosition().x, shroom->GetPosition().y+30.0f, shroom->GetPosition().y-10.0f);
                    s->SetSunValue(shroom->GetSunAmount());
                    m_Suns.push_back(s); m_Root.AddChild(s); shroom->ResetSunFlag();
                }
            } else if (auto shooter = std::dynamic_pointer_cast<Peashooter>(plant)) {
                if (rowHasZombie[r] && shooter->CanFire()) {
                    auto p = std::make_shared<Pea>(shooter->GetPosition().x+30.0f, shooter->GetPosition().y+35.0f, Pea::Type::NORMAL);
                    m_Peas.push_back(p); m_Root.AddChild(p); shooter->ResetFireFlag();
                } else if (!rowHasZombie[r]) shooter->ResetFireFlag();
            } else if (auto snow = std::dynamic_pointer_cast<SnowPea>(plant)) {
                if (rowHasZombie[r] && snow->CanFire()) {
                    auto p = std::make_shared<Pea>(snow->GetPosition().x+30.0f, snow->GetPosition().y+35.0f, Pea::Type::ICE);
                    m_Peas.push_back(p); m_Root.AddChild(p); snow->ResetFireFlag();
                } else if (!rowHasZombie[r]) snow->ResetFireFlag();
            } else if (auto rep = std::dynamic_pointer_cast<Repeater>(plant)) {
                if (rowHasZombie[r] && rep->CanFire()) {
                    auto p1 = std::make_shared<Pea>(rep->GetPosition().x+30.0f,  rep->GetPosition().y+35.0f, Pea::Type::NORMAL);
                    auto p2 = std::make_shared<Pea>(rep->GetPosition().x-10.0f,  rep->GetPosition().y+35.0f, Pea::Type::NORMAL);
                    m_Peas.push_back(p1); m_Peas.push_back(p2); m_Root.AddChild(p1); m_Root.AddChild(p2); rep->ResetFireFlag();
                } else if (!rowHasZombie[r]) rep->ResetFireFlag();
            } else if (auto puff = std::dynamic_pointer_cast<PuffShroom>(plant)) {
                bool inRange = false;
                for (auto& z : m_zombies) {
                    if (z->IsDead()) continue;
                    float dist = z->GetPosition().x - puff->GetPosition().x;
                    if (dist > 0 && dist < 240.0f && std::abs(z->GetPosition().y - puff->GetPosition().y) < 50.0f) { inRange = true; break; }
                }
                if (inRange && puff->CanFire()) {
                    auto p = std::make_shared<Pea>(puff->GetPosition().x+20.0f, puff->GetPosition().y+20.0f, Pea::Type::MUSHROOM, 240.0f);
                    m_Peas.push_back(p); m_Root.AddChild(p); puff->ResetFireFlag();
                }
            } else if (auto fume = std::dynamic_pointer_cast<FumeShroom>(plant)) {
                if (rowHasZombie[r] && fume->CanFire()) {
                    float fumeX = fume->GetPosition().x;
                    float maxX  = 400.0f;
                    for (auto& z : m_zombies) {
                        if (z->IsDead()) continue;
                        if (z->GetPosition().x - fumeX > 0 && z->GetPosition().x <= maxX && std::abs(z->GetPosition().y - fume->GetPosition().y) < 50.0f)
                            z->TakePenetratingDamage(20);
                    }
                    float travelDist = maxX - (fumeX + 30.0f);
                    if (travelDist > 0) {
                        auto p = std::make_shared<Pea>(fumeX+30.0f, fume->GetPosition().y+20.0f, Pea::Type::FUME, travelDist);
                        m_Peas.push_back(p); m_Root.AddChild(p);
                    }
                    if (m_PeaHitSFX) m_PeaHitSFX->Play();
                    fume->ResetFireFlag();
                }
            } else if (auto scaredy = std::dynamic_pointer_cast<ScaredyShroom>(plant)) {
                bool nearby = false;
                int pr, pc; m_Map->GetGridIndex(scaredy->GetPosition(), pr, pc);
                for (auto& z : m_zombies) {
                    if (z->IsDead()) continue;
                    int zr, zc;
                    if (m_Map->GetGridIndex(z->GetPosition(), zr, zc))
                        if (std::abs(zr-pr) <= 1 && std::abs(zc-pc) <= 1) { nearby = true; break; }
                }
                scaredy->SetScared(nearby);
                if (!scaredy->IsScared() && rowHasZombie[r] && scaredy->CanFire()) {
                    auto p = std::make_shared<Pea>(scaredy->GetPosition().x+30.0f, scaredy->GetPosition().y+35.0f, Pea::Type::MUSHROOM);
                    m_Peas.push_back(p); m_Root.AddChild(p); scaredy->ResetFireFlag();
                }
            } else if (auto mine = std::dynamic_pointer_cast<PotatoMine>(plant)) {
                if (mine->GetMineState() == PotatoMine::MineState::READY) {
                    for (auto& z : m_zombies) {
                        if (z->IsDead() || (z->GetType() == Zombie::Type::POLEVAULTER && !z->HasJumped())) continue;
                        if (glm::distance(mine->GetPosition(), z->GetPosition()) < 45.0f) { mine->Trigger(); z->TakeDamage(1800); break; }
                    }
                }
            } else if (auto cherry = std::dynamic_pointer_cast<CherryBomb>(plant)) {
                if (cherry->LogicReady()) {
                    if (m_ExplodeSFX) m_ExplodeSFX->Play();
                    for (auto& z : m_zombies) {
                        if (z->IsDead()) continue;
                        int zr, zc;
                        if (m_Map->GetGridIndex(z->GetPosition(), zr, zc))
                            if (std::abs(cherry->GetPosition().x - z->GetPosition().x) <= 150.0f && std::abs(zr-r) <= 1)
                                z->TakePenetratingDamage(1800);
                    }
                }
            }
            if (plant->IsDead()) { m_Root.RemoveChild(plant); m_Map->RemovePlant(r, c); }
        }
    }
}

void App::UpdateSuns(float dt) {
    for (auto& sun : m_Suns) sun->Update(dt);
    m_Suns.erase(std::remove_if(m_Suns.begin(), m_Suns.end(),
        [this](const std::shared_ptr<Sun>& s) { if (s->ShouldRemove()) { m_Root.RemoveChild(s); return true; } return false; }), m_Suns.end());
}

void App::UpdateProjectiles(float dt) {
    for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
        Pea* pea = it->get();
        pea->Update(dt);
        if (pea->IsOffScreen() || (pea->GetPosition().x - pea->GetStartX() > pea->GetMaxTravelDist())) { m_Root.RemoveChild(*it); it = m_Peas.erase(it); continue; }
        if (pea->GetPeaType() == Pea::Type::FUME) { ++it; continue; }

        bool hit = false;
        for (auto& z : m_zombies) {
            if (z->IsDead()) continue;
            if (glm::distance(pea->GetPosition(), z->GetPosition()) < 40.0f) {
                z->TakeDamage(20);
                if (pea->GetPeaType() == Pea::Type::ICE) z->SlowDown(10.0f);
                if (m_PeaHitSFX) m_PeaHitSFX->Play();
                hit = true; break;
            }
        }
        if (hit) { m_Root.RemoveChild(*it); it = m_Peas.erase(it); } else { ++it; }
    }
}

void App::UpdateZombies(float dt) {
    for (auto& z : m_zombies) {
        z->Update(dt);
        auto head = z->SpawnHead();
        if (head) { m_ZombieHeads.push_back(head); m_Root.AddChild(head); }
        if (z->IsDead()) continue;

        int r, c;
        if (m_Map->GetGridIndex(z->GetPosition() + glm::vec2{-25, 0}, r, c)) {
            auto p = m_Map->GetPlant(r, c);
            if (p) {
                if (z->GetType() == Zombie::Type::POLEVAULTER && !z->HasJumped() && !z->IsJumping()) {
                    if (z->GetPosition().x - p->GetPosition().x < 0.0f) z->StartJump();
                } else if (!z->IsJumping()) {
                    bool canEat = true;
                    if (auto mine = std::dynamic_pointer_cast<PotatoMine>(p))
                        if (mine->GetMineState() == PotatoMine::MineState::READY) canEat = false;
                    if (canEat) {
                        z->SetState(Zombie::State::EATING); p->TakeDamage(static_cast<int>(z->GetAttackPower() * dt));
                    } else if (z->GetState() == Zombie::State::EATING) { z->SetState(Zombie::State::WALKING); }
                }
            } else if (z->GetState() == Zombie::State::EATING) { z->SetState(Zombie::State::WALKING); }
        }
    }
}

void App::End() { LOG_DEBUG("Game Ended."); }