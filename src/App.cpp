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
    m_ImgPotatoMine = std::make_shared<Util::Image>("resources/image/potatomine/underground/underground.png");
    m_ImgSnowPea = std::make_shared<Util::Image>("resources/image/snowpea/1.png");
    m_ImgSunShroom = std::make_shared<Util::Image>("resources/image/sunshroom/1.png");
    m_ImgPuff = std::make_shared<Util::Image>("resources/image/puffshroom/1.png");
    m_ImgFume = std::make_shared<Util::Image>("resources/image/fumeshroom/1.png");
    m_ImgScaredy = std::make_shared<Util::Image>("resources/image/scaredyshroom/1.png");
    m_ImgCherry = std::make_shared<Util::Image>("resources/image/cherrybomb/1.png");
    m_ImgRepeater = std::make_shared<Util::Image>("resources/image/repeaterpea/1.png");

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
    m_ExplodeSFX = std::make_shared<Util::SFX>("resources/music/cherrybomb.wav");

    if (m_MenuBGM) m_MenuBGM->Play(-1);
    m_CurrentState = State::START;
}

void App::LoadLevelConfig(int level) {
    // 🚩 補回！根據關卡判斷要載入白天還是夜晚地圖，以及對應的縮放比例
    std::string mapPath = (level >= 6) ? "resources/image/map_night.jpg" : "resources/image/map.jpg";
    float mapScale = (level >= 6) ? 2.0f : 2.0f;

    // 重新建立地圖物件
    m_Map = std::make_shared<GameMap>(mapPath);
    m_Map->m_Transform.scale = {mapScale, mapScale};
    m_Map->SetZIndex(0);

    std::vector<int> allowed;
    switch (level) {
        case 1:allowed = {1, 2, 3};
            m_CurrentLevelConfig = {15, 8.0f, 70, 20, 10,0,50,0,allowed}; break;
        case 2:allowed = {1, 2, 3, 5, 6, 7, 12};
            m_CurrentLevelConfig = {20, 8.0f, 70, 10, 10,10,0,0,allowed}; break;
        case 3:allowed = {1, 2, 3, 5, 6};
            m_CurrentLevelConfig = {25, 8.0f, 50, 20, 20,10,50,0,allowed}; break;
        case 4:allowed = {1, 2, 3, 5, 6};
            m_CurrentLevelConfig = {30, 8.0f, 40, 25, 25,10,50,0,allowed}; break;
        case 5:allowed = {1, 2, 3, 5, 6, 7};
            m_CurrentLevelConfig = {40, 8.0f, 50, 15, 15,10,10,0,allowed}; break;
        case 6:allowed = {1, 8, 9, 10, 3, 7};
            m_CurrentLevelConfig = {15, 8.0f, 50, 15, 15,10,10,0,allowed}; break;
        case 7:allowed = {1, 8, 9, 10, 3, 7};
            m_CurrentLevelConfig = {20, 8.0f, 50, 15, 15,10,10,0,allowed}; break;
        case 8:allowed = {1, 8, 6, 10, 3, 7};
            m_CurrentLevelConfig = {25, 8.0f, 50, 15, 15,10,10,0,allowed}; break;
        case 9:allowed = {1, 8, 6, 10, 3, 7, 11};
            m_CurrentLevelConfig = {30, 8.0f, 20, 15, 15,15,15,20,allowed}; break;
        case 10:allowed = {8, 6, 10, 3, 7, 11, 12};
            m_CurrentLevelConfig = {40, 8.0f, 20, 15, 15,15,15,20,allowed}; break;
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
    m_StateTimer = 0.0f;
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

            // 按下 F1：獲得 1000 陽光
            if (Util::Input::IsKeyDown(Util::Keycode::F1)) {
                m_SunCurrency += 1000;
                if (m_SunCollectSFX) m_SunCollectSFX->Play();
                LOG_DEBUG("作弊碼觸發：獲得 1000 陽光！目前陽光：{}", m_SunCurrency);
            }

            // 按下 F2：全畫面殭屍直接暴斃
            if (Util::Input::IsKeyDown(Util::Keycode::F2)) {
                for (auto& z : m_zombies) {
                    if (!z->IsDead()) {
                        z->TakeDamage(9999);
                    }
                }
                if (m_ExplodeSFX) m_ExplodeSFX->Play();
                LOG_DEBUG("作弊碼觸發：全場殭屍秒殺！");
            }

            // 按下 F3：強制直接過關
            if (Util::Input::IsKeyDown(Util::Keycode::F3)) {
                // 直接把剩餘要生的殭屍歸零，並把場上的清空
                m_ZombiesSpawnedInLevel = m_TotalZombiesToSpawn;
                for (auto& z : m_zombies) z->TakeDamage(9999);
                LOG_DEBUG("作弊碼觸發：強制勝利通關！");
            }
            if (Util::Input::IsKeyDown(Util::Keycode::F4)) {
                if (m_SeedBank) {
                    m_SeedBank->ResetAllCooldowns();
                    if (m_PlantSeedSFX) m_PlantSeedSFX->Play(); // 播放拿起卡片的音效
                    LOG_DEBUG("作弊碼觸發：全植物冷卻完畢！");
                }
            }

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
                    int cost = 0;

                    if (m_SelectedPlantType == 1 && m_SunCurrency >= 100) { p = std::make_shared<Peashooter>(0, 0); cost = 100; }
                    else if (m_SelectedPlantType == 2 && m_SunCurrency >= 50)  { p = std::make_shared<Sunflower>(0, 0);  cost = 50; }
                    else if (m_SelectedPlantType == 3 && m_SunCurrency >= 50)  { p = std::make_shared<Wallnut>(0, 0);    cost = 50; }
                    else if (m_SelectedPlantType == 5 && m_SunCurrency >= 25)  { p = std::make_shared<PotatoMine>(0, 0); cost = 25; }
                    else if (m_SelectedPlantType == 6 && m_SunCurrency >= 175) { p = std::make_shared<SnowPea>(0, 0);    cost = 175; }
                    else if (m_SelectedPlantType == 7 && m_SunCurrency >= 150) { p = std::make_shared<CherryBomb>(0, 0); cost = 150; }
                    else if (m_SelectedPlantType == 8 && m_SunCurrency >= 25)  { p = std::make_shared<SunShroom>(0, 0);  cost = 25; }
                    else if (m_SelectedPlantType == 9 && m_SunCurrency >= 0)   { p = std::make_shared<PuffShroom>(0, 0);  cost = 0; }
                    else if (m_SelectedPlantType == 10 && m_SunCurrency >= 75) { p = std::make_shared<FumeShroom>(0, 0);  cost = 75; }
                    else if (m_SelectedPlantType == 11 && m_SunCurrency >= 25) { p = std::make_shared<ScaredyShroom>(0, 0); cost = 25; }
                    else if (m_SelectedPlantType == 12 && m_SunCurrency >= 200) { p = std::make_shared<Repeater>(0, 0);  cost = 200; }

                    if (p && m_Map->PlacePlant(r, c, p)) {
                        m_SunCurrency -= cost;
                        m_Root.AddChild(p);
                        m_SeedBank->StartCooldown(m_SelectedPlantType);
                        if (m_PlantSeedSFX) m_PlantSeedSFX->Play();
                    }
                }
            }
            // 🚩 無難如何都要還原鏟子與狀態
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
                    m_SunCurrency += (*it)->GetSunValue(); (*it)->Collect(); m_Root.RemoveChild(*it);
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
                    else if (type == 5) m_DragPreview->SetDrawable(m_ImgPotatoMine);
                    else if (type == 6) m_DragPreview->SetDrawable(m_ImgSnowPea);
                    else if (type == 7) m_DragPreview->SetDrawable(m_ImgCherry);
                    else if (type == 8) m_DragPreview->SetDrawable(m_ImgSunShroom);
                    else if (type == 9) m_DragPreview->SetDrawable(m_ImgPuff);
                    else if (type == 10) m_DragPreview->SetDrawable(m_ImgFume);
                    else if (type == 11) m_DragPreview->SetDrawable(m_ImgScaredy);
                    else if (type == 12) m_DragPreview->SetDrawable(m_ImgRepeater);
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

        // 🚩 修正：呼叫我們補好、具備所有夜晚植物和射手逻辑的行為更新函式
        UpdatePlantActions();

        for (auto& sun : m_Suns) sun->Update(dt);
        m_Suns.erase(std::remove_if(m_Suns.begin(), m_Suns.end(), [this](const std::shared_ptr<Sun>& s) {
            if (s->ShouldRemove()) { m_Root.RemoveChild(s); return true; } return false;
        }), m_Suns.end());

        for (auto it = m_Peas.begin(); it != m_Peas.end(); ) {
            Pea* peaPtr = it->get();
            peaPtr->Update(dt);

            // 1. 超出螢幕邊界 -> 刪除
            if (peaPtr->IsOffScreen()) {
                m_Root.RemoveChild(*it);
                it = m_Peas.erase(it);
                continue;
            }

            // 2. 超過最大射程 (小噴菇、大噴菇) -> 刪除
            float traveledDist = peaPtr->GetPosition().x - peaPtr->GetStartX();
            if (traveledDist > peaPtr->GetMaxTravelDist()) {
                m_Root.RemoveChild(*it);
                it = m_Peas.erase(it);
                continue;
            }

            // 3. 🚩 如果是大噴菇的子彈 (FUME)，直接跳過碰撞檢查，讓它繼續飛！
            if (peaPtr->GetPeaType() == Pea::Type::FUME) {
                ++it;
                continue;
            }

            // 4. 實體子彈 (豌豆、冰雪、小噴菇) 的碰撞判定
            bool peaDestroyed = false;
            for (auto& z : m_zombies) {
                if (z->IsDead()) continue;
                if (glm::distance(peaPtr->GetPosition(), z->GetPosition()) < 40.0f) {
                    z->TakeDamage(20); // 扣血
                    if (peaPtr->GetPeaType() == Pea::Type::ICE) {
                        z->SlowDown(10.0f); // 冰凍減速
                    }
                    if (m_PeaHitSFX) m_PeaHitSFX->Play();
                    peaDestroyed = true;
                    break;
                }
            }

            // 撞到就刪除
            if (peaDestroyed) {
                m_Root.RemoveChild(*it);
                it = m_Peas.erase(it);
            } else {
                ++it;
            }
        }

        // =====================================================================
        // 🚩 殭屍邏輯更新 (原本直接寫在主迴圈內)
        // =====================================================================
        for (auto& z : m_zombies) {
            z->Update(dt);

            auto head = z->SpawnHead();
            if (head) {
                m_ZombieHeads.push_back(head);
                m_Root.AddChild(head);
            }

            if (z->IsDead()) continue;

            int r, c;
            if (m_Map->GetGridIndex(z->GetPosition() + glm::vec2{-25, 0}, r, c)) {
                auto p = m_Map->GetPlant(r, c);

                if (p) {
                    if (z->GetType() == Zombie::Type::POLEVAULTER && !z->HasJumped()) {
                        float dist = z->GetPosition().x - p->GetPosition().x;
                        if (dist < 0.0f && !z->IsJumping()) {
                            z->StartJump();
                        }
                    }
                    else if (!z->IsJumping()) {
                        // 🚩 修正點：若前方是地雷且已經準備好(READY)，殭屍就不能吃，要等著被炸！
                        bool canEat = true;
                        if (auto mine = std::dynamic_pointer_cast<PotatoMine>(p)) {
                            if (mine->GetMineState() == PotatoMine::MineState::READY) {
                                canEat = false;
                            }
                        }

                        if (canEat) {
                            z->SetState(Zombie::State::EATING);
                            p->TakeDamage(static_cast<int>(z->GetAttackPower() * dt));
                        } else if (z->GetState() == Zombie::State::EATING) {
                            z->SetState(Zombie::State::WALKING); // 恢復走路去踩雷
                        }
                    }
                }
                else {
                    if (z->GetState() == Zombie::State::EATING) {
                        z->SetState(Zombie::State::WALKING);
                    }
                }
            }
        }

        if (m_ZombiesSpawnedInLevel < m_TotalZombiesToSpawn) {
            m_ZombieSpawnTimer += dt;

            if (m_ZombieSpawnTimer > m_CurrentLevelConfig.spawnInterval) {
                int r = rand() % 5;

                int totalW = m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone +
                         m_CurrentLevelConfig.weightBucket + m_CurrentLevelConfig.weightPole +
                         m_CurrentLevelConfig.weightFootball+ m_CurrentLevelConfig.weightScreenDoor;;
                if (totalW <= 0) totalW = 1;

                int randVal = rand() % totalW;

                Zombie::Type type;

                if (randVal < m_CurrentLevelConfig.weightNormal) { type = Zombie::Type::NORMAL; }
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone) { type = Zombie::Type::CONEHEAD; }
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket) { type = Zombie::Type::BUCKETHEAD; }
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket + m_CurrentLevelConfig.weightPole) { type = Zombie::Type::POLEVAULTER; }
                else if (randVal < m_CurrentLevelConfig.weightNormal + m_CurrentLevelConfig.weightCone + m_CurrentLevelConfig.weightBucket + m_CurrentLevelConfig.weightPole + m_CurrentLevelConfig.weightFootball) {
                    type = Zombie::Type::FOOTBALL;
                }
                else {
                    type = Zombie::Type::SCREENDOOR;
                }

                float spawnY =m_Map->CalculateGridCenter(r, 8).y + 20.0f;
                if (type == Zombie::Type::POLEVAULTER) { spawnY -= 10.0f; }

                auto newZ = std::make_shared<Zombie>(750.0f, spawnY, type);
                m_zombies.push_back(newZ);
                m_Root.AddChild(newZ);

                m_ZombiesSpawnedInLevel++;
                m_ZombieSpawnTimer = 0.0f;
            }
        }

        // 🚩 只有白天會掉陽光
        if (m_CurrentLevel < 6) {
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


// =============================================================================
// [ 狀態更新分流 ]
// =============================================================================

void App::UpdateStartState(glm::vec2 mousePos) {
    m_MenuBackground->Draw();
    m_StartButton->Draw();
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        if (mousePos.x > 50 && mousePos.x < 370 && mousePos.y > 80 && mousePos.y < 180)
            m_CurrentState = State::SELECT_LEVEL;
    }
    m_Root.Update();
}

void App::UpdateSelectLevelState(glm::vec2 mousePos) {
    m_SelectLevelBG->Draw();
    for (int i = 0; i < (int)m_LevelButtons.size(); ++i) {
        if (glm::distance(mousePos, m_LevelButtons[i]->m_Transform.translation) < 60.0f) {
            m_LevelButtons[i]->m_Transform.scale = {1.1f, 1.1f};
            m_LevelTexts[i]->m_Transform.scale = {1.1f, 1.1f};
            if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
                m_CurrentLevel = i + 1;
                m_Root.RemoveChild(m_Map);
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

void App::UpdateGameState(float dt, glm::vec2 mousePos) {
    // 🚩 空殼不啟用
}

void App::UpdateDefeatState(float dt) {
    if (m_DefeatScreen) m_DefeatScreen->Draw();
    m_StateTimer += dt;
    if (m_StateTimer >= 3.0f) {
        ResetGame();
        m_CurrentState = State::SELECT_LEVEL;
    }
}

void App::UpdatePlants(float dt) {
    // 🚩 已停用
}

// =============================================================================
// [ 核心植物戰鬥子模組 ]
// =============================================================================

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

            // 1. 向日葵類
            if (auto flower = std::dynamic_pointer_cast<Sunflower>(plant)) {
                if (flower->CanProduceSun()) {
                    auto s = std::make_shared<Sun>(flower->GetPosition().x, flower->GetPosition().y + 50.0f, flower->GetPosition().y - 10.0f);
                    m_Suns.push_back(s); m_Root.AddChild(s); flower->ResetSunFlag();
                }
            }
            else if (auto sunShroom = std::dynamic_pointer_cast<SunShroom>(plant)) {
                if (sunShroom->CanProduceSun()) {
                    auto s = std::make_shared<Sun>(sunShroom->GetPosition().x, sunShroom->GetPosition().y + 30.0f, sunShroom->GetPosition().y - 10.0f);
                    s->SetSunValue(sunShroom->GetSunAmount());
                    m_Suns.push_back(s); m_Root.AddChild(s); sunShroom->ResetSunFlag();
                }
            }
            // 2. 射手類 (每一種都必須獨立 if)
            else if (auto shooter = std::dynamic_pointer_cast<Peashooter>(plant)) {
                if (rowHasZombie[r] && shooter->CanFire()) {
                    auto p = std::make_shared<Pea>(shooter->GetPosition().x + 30.0f, shooter->GetPosition().y + 35.0f, Pea::Type::NORMAL);
                    m_Peas.push_back(p); m_Root.AddChild(p); shooter->ResetFireFlag();
                } else if (!rowHasZombie[r]) shooter->ResetFireFlag();
            }
            else if (auto snowShooter = std::dynamic_pointer_cast<SnowPea>(plant)) {
                if (rowHasZombie[r] && snowShooter->CanFire()) {
                    auto p = std::make_shared<Pea>(snowShooter->GetPosition().x + 30.0f, snowShooter->GetPosition().y + 35.0f, Pea::Type::ICE);
                    m_Peas.push_back(p); m_Root.AddChild(p); snowShooter->ResetFireFlag();
                } else if (!rowHasZombie[r]) snowShooter->ResetFireFlag();
            }
            else if (auto repeater = std::dynamic_pointer_cast<Repeater>(plant)) {
                if (rowHasZombie[r] && repeater->CanFire()) {
                    auto p1 = std::make_shared<Pea>(repeater->GetPosition().x + 30.0f, repeater->GetPosition().y + 35.0f, Pea::Type::NORMAL);
                    auto p2 = std::make_shared<Pea>(repeater->GetPosition().x - 10.0f, repeater->GetPosition().y + 35.0f, Pea::Type::NORMAL);
                    m_Peas.push_back(p1); m_Peas.push_back(p2);
                    m_Root.AddChild(p1); m_Root.AddChild(p2);
                    repeater->ResetFireFlag();
                } else if (!rowHasZombie[r]) repeater->ResetFireFlag();
            }
            // 3. 蘑菇類
            else if (auto puff = std::dynamic_pointer_cast<PuffShroom>(plant)) {
                bool zombieInRange = false;
                for (auto& z : m_zombies) {
                    if (z->IsDead()) continue;
                    float dist = z->GetPosition().x - puff->GetPosition().x;
                    // 三格距離約為 240.0f
                    if (dist > 0 && dist < 240.0f && std::abs(z->GetPosition().y - puff->GetPosition().y) < 50.0f) {
                        zombieInRange = true; break;
                    }
                }
                if (zombieInRange && puff->CanFire()) {
                    // 🚩 這裡傳入 240.0f 當作子彈的最大射程！
                    auto p = std::make_shared<Pea>(puff->GetPosition().x + 20.0f, puff->GetPosition().y + 20.0f, Pea::Type::MUSHROOM, 240.0f);
                    m_Peas.push_back(p); m_Root.AddChild(p); puff->ResetFireFlag();
                }
            }
            // --- 7. 大噴菇 (群體傷害) ---
            else if (auto fume = std::dynamic_pointer_cast<FumeShroom>(plant)) {
                if (rowHasZombie[r] && fume->CanFire()) {
                    bool hitAny = false;
                    float fumeX = fume->GetPosition().x;
                    float maxScreenX = 400.0f;

                    for (auto& z : m_zombies) {
                        if (z->IsDead()) continue;
                        float zombieX = z->GetPosition().x;
                        float dist = zombieX - fumeX;

                        // 🚩 修改 1：條件變成「殭屍在右邊 (dist > 0) 且 殭屍的 X 坐標小於等於 530」
                        if (dist > 0 && zombieX <= maxScreenX && std::abs(z->GetPosition().y - fume->GetPosition().y) < 50.0f) {
                            z->TakePenetratingDamage(20);
                            hitAny = true;
                        }
                    }

                    if (hitAny && m_PeaHitSFX) m_PeaHitSFX->Play();

                    // 🚩 修改 2：計算子彈「實際需要飛多遠」才會碰到 530 的空氣牆
                    float startX = fumeX + 30.0f;
                    float travelDist = maxScreenX - startX;

                    if (travelDist > 0) { // 確保大噴菇沒有被種在超過 530 的地方
                        auto p = std::make_shared<Pea>(startX, fume->GetPosition().y + 20.0f, Pea::Type::FUME, travelDist);
                        m_Peas.push_back(p);
                        m_Root.AddChild(p);
                    }

                    fume->ResetFireFlag();
                }
            }
            // --- 8. 膽小菇 (3x3 隱藏判定) ---
            else if (auto scaredy = std::dynamic_pointer_cast<ScaredyShroom>(plant)) {
                bool nearby = false;
                for (auto& z : m_zombies) {
                    if (z->IsDead()) continue;
                    float dx = std::abs(z->GetPosition().x - scaredy->GetPosition().x);
                    float dy = std::abs(z->GetPosition().y - scaredy->GetPosition().y);
                    // 3x3 街區判定：上下各一排，左右各一格
                    if (dx <= 120.0f && dy <= 120.0f) {
                        nearby = true; break;
                    }
                }
                scaredy->SetScared(nearby);
                if (!scaredy->IsScared() && rowHasZombie[r] && scaredy->CanFire()) {
                    // 膽小菇射程無限，不傳 maxDist
                    auto p = std::make_shared<Pea>(scaredy->GetPosition().x + 30.0f, scaredy->GetPosition().y + 35.0f, Pea::Type::MUSHROOM);
                    m_Peas.push_back(p); m_Root.AddChild(p); scaredy->ResetFireFlag();
                }
            }
            // 4. 特殊植物
            else if (auto mine = std::dynamic_pointer_cast<PotatoMine>(plant)) {
                if (mine->GetMineState() == PotatoMine::MineState::READY) {
                    for (auto& z : m_zombies) {
                        if (!z->IsDead() && glm::distance(mine->GetPosition(), z->GetPosition()) < 45.0f) {
                            mine->Trigger(); z->TakeDamage(1800); break;
                        }
                    }
                }
            }
            else if (auto cherry = std::dynamic_pointer_cast<CherryBomb>(plant)) {
                if (cherry->LogicReady()) {
                    if (m_ExplodeSFX) m_ExplodeSFX->Play();
                    for (auto& z : m_zombies) {
                        if (z->IsDead()) continue;

                        int zRow, zCol;
                        if (m_Map->GetGridIndex(z->GetPosition(), zRow, zCol)) {
                            float distX = std::abs(cherry->GetPosition().x - z->GetPosition().x);

                            if (distX <= 150.0f && std::abs(zRow - r) <= 1) {
                                z->TakeDamage(1800);
                            }
                        }
                    }
                }
            }
            if (plant->IsDead()) { m_Root.RemoveChild(plant); m_Map->RemovePlant(r, c); }
        }
    }
}

void App::ResetGame() {
    for (auto& m : m_LawnMowers) m_Root.RemoveChild(m->obj);
    m_LawnMowers.clear();
    for (int r = 0; r < 5; ++r) {
        auto mower = std::make_shared<LawnMowerData>();
        mower->obj = std::make_shared<Util::GameObject>();
        mower->obj->SetDrawable(std::make_shared<Util::Image>("resources/image/weeder.png"));
        mower->obj->m_Transform.translation = {-450.0f, m_Map->CalculateGridCenter(r, 0).y};
        mower->obj->SetZIndex(45); mower->row = r; mower->state = LawnMowerData::State::IDLE;
        m_LawnMowers.push_back(mower); m_Root.AddChild(mower->obj);
    }
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
    m_SunCurrency = 50; m_SelectedPlantType = 0; m_DragPreview->SetVisible(false);
    m_ZombiesSpawnedInLevel = 0; m_StateTimer = 0.0f; skySunTimer = 0.0f;
    m_SeedBank->SetShovelVisible(true);
    if (m_GameBGM) m_GameBGM->Pause();
    if (m_MenuBGM) m_MenuBGM->Play(-1);
}

void App::End() {
    LOG_DEBUG("Game Ended.");
}