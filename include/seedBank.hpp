#ifndef SEEDBANK_HPP
#define SEEDBANK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Color.hpp"
#include "Util/Time.hpp"
#include <vector>
#include <memory>
#include <string>
#include <map>

class SeedBank : public Util::GameObject {
public:
    SeedBank() {
        // 載入工具欄背景
        m_ToolbarFull = std::make_shared<Util::Image>("resources/image/UI/toolbar.png");
        m_ToolbarNoShovel = std::make_shared<Util::Image>("resources/image/UI/toolbarWithoutShovel.png");

        SetDrawable(m_ToolbarFull);
        m_Transform.scale = {1.f, 1.f};
        m_Transform.translation = {-270.0f, 270.0f};
        SetZIndex(90);

        // 陽光文字初始化
        m_SunText = std::make_shared<Util::Text>("resources/font/impact.ttf", 20, "50", Util::Color::FromRGB(0, 0, 0, 255));
        m_SunTextObject = std::make_shared<Util::GameObject>();
        m_SunTextObject->SetDrawable(m_SunText);
        m_SunTextObject->m_Transform.translation = m_Transform.translation + glm::vec2{-301.0f, -33.0f};
        m_SunTextObject->SetZIndex(100);

        // 初始化冷卻計時
        m_CooldownTimers[1] = 0.0f;
        m_CooldownTimers[2] = 0.0f;
        m_CooldownTimers[3] = 0.0f;

        InitCards();
    }

    // 🚩 只切換背景圖片，不再控制卡片顯示
    void SetShovelVisible(bool visible) {
        m_IsShovelVisible = visible;
        SetDrawable(visible ? m_ToolbarFull : m_ToolbarNoShovel);
    }

    void UpdateCooldown(float dt) {
        for (auto& pair : m_CooldownTimers) {
            if (pair.second > 0) {
                pair.second -= dt;
                if (pair.second < 0) pair.second = 0.0f;
            }
        }
    }

    void StartCooldown(int type) {
        if (type == 3) {
            m_CooldownTimers[3] = 30.0f;
        } else if (type > 0 && type < 4) {
            m_CooldownTimers[type] = 7.5f;
        }
    }

    void SetSunCount(int count) {
        m_CurrentSun = count;
        m_SunText->SetText(std::to_string(count));
    }

    void DrawUI() {
        this->Draw(); // 畫工具欄背景（含內建的鏟子或空位）

        for (int i = 0; i < m_Cards.size(); ++i) {
            m_Cards[i]->Draw();

            int type = i + 1;
            float cooldownLeft = m_CooldownTimers[type];
            float maxCooldown = (type == 3) ? 30.0f : 7.5f;
            int cost = (type == 1) ? 100 : 50;

            if (cooldownLeft > 0) {
                float progress = cooldownLeft / maxCooldown;
                float originalScaleY = 0.9f;
                float pixelHeight = 80.0f;

                m_CooldownMasks[i]->m_Transform.scale = {0.9f, originalScaleY * progress};
                float yOffset = (1.0f - progress) * (pixelHeight / 2.0f);
                m_CooldownMasks[i]->m_Transform.translation = m_Cards[i]->m_Transform.translation + glm::vec2{0.0f, yOffset};
                m_CooldownMasks[i]->Draw();
            }

            if (m_CurrentSun < cost) {
                m_SunMasks[i]->Draw();
            }
        }
        m_SunTextObject->Draw();
    }

    int GetSelectedType(glm::vec2 mousePos) {
        float baseLine = m_Transform.translation.x;

        // 植物卡片判定
        float peaX = baseLine - 232.0f;
        if (mousePos.x > (peaX - 30.0f) && mousePos.x < (peaX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[1] <= 0 && m_CurrentSun >= 100) return 1;
        }

        float sunX = baseLine - 170.0f;
        if (mousePos.x > (sunX - 30.0f) && mousePos.x < (sunX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[2] <= 0 && m_CurrentSun >= 50) return 2;
        }

        float wallX = baseLine - 108.0f;
        if (mousePos.x > (wallX - 30.0f) && mousePos.x < (wallX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[3] <= 0 && m_CurrentSun >= 50) return 3;
        }

        // 🚩 鏟子區域判定：只要點在背景圖的鏟子位置即可
        float shovelX = baseLine + 280.0f;
        if (m_IsShovelVisible &&
            mousePos.x > (shovelX - 35.0f) && mousePos.x < (shovelX + 70.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            return 4;
        }

        return 0;
    }

private:
    void InitCards() {
        m_Cards.clear();
        m_CooldownMasks.clear();
        m_SunMasks.clear();

        std::vector<float> offsets = {-232.0f, -170.0f, -108.0f};
        std::vector<std::string> paths = {
            "resources/image/UI/peashooter_card.png",
            "resources/image/UI/sunflower_card.png",
            "resources/image/UI/wallnut_card.png"
        };

        for (int i = 0; i < 3; ++i) {
            auto cardPos = m_Transform.translation + glm::vec2{offsets[i], 0.0f};

            auto card = std::make_shared<Util::GameObject>();
            card->SetDrawable(std::make_shared<Util::Image>(paths[i]));
            card->m_Transform.translation = cardPos;
            card->m_Transform.scale = {0.9f, 0.9f};
            card->SetZIndex(95);
            m_Cards.push_back(card);

            auto cMask = std::make_shared<Util::GameObject>();
            cMask->SetDrawable(std::make_shared<Util::Image>("resources/image/mask.png"));
            cMask->SetZIndex(96);
            m_CooldownMasks.push_back(cMask);

            auto sMask = std::make_shared<Util::GameObject>();
            sMask->SetDrawable(std::make_shared<Util::Image>("resources/image/mask.png"));
            sMask->m_Transform.translation = cardPos;
            sMask->m_Transform.scale = {0.9f, 0.9f};
            sMask->SetZIndex(97);
            m_SunMasks.push_back(sMask);
        }

        m_IsShovelVisible = true;
    }

    std::vector<std::shared_ptr<Util::GameObject>> m_Cards;
    std::vector<std::shared_ptr<Util::GameObject>> m_CooldownMasks;
    std::vector<std::shared_ptr<Util::GameObject>> m_SunMasks;

    // 🚩 移除 m_ShovelCard 物件，只保留背景控制
    std::shared_ptr<Util::Image> m_ToolbarFull;
    std::shared_ptr<Util::Image> m_ToolbarNoShovel;
    bool m_IsShovelVisible = true;

    std::shared_ptr<Util::Text> m_SunText;
    std::shared_ptr<Util::GameObject> m_SunTextObject;

    int m_CurrentSun = 50;
    std::map<int, float> m_CooldownTimers;
};

#endif