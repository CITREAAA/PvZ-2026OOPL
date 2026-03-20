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
        // 背景木框
        SetDrawable(std::make_shared<Util::Image>("resources/image/UI/seedBank.png"));
        m_Transform.scale = {1.f, 1.f};
        m_Transform.translation = {-330.0f, 270.0f};
        SetZIndex(90);

        // 陽光文字
        m_SunText = std::make_shared<Util::Text>("resources/font/impact.ttf", 20, "50", Util::Color::FromRGB(0, 0, 0));
        m_SunTextObject = std::make_shared<Util::GameObject>();
        m_SunTextObject->SetDrawable(m_SunText);
        m_SunTextObject->m_Transform.translation = m_Transform.translation + glm::vec2{-251.0f, -33.0f};
        m_SunTextObject->SetZIndex(100); // 數字最上層

        // 初始化冷卻計時
        m_CooldownTimers[1] = 0.0f;
        m_CooldownTimers[2] = 0.0f;

        InitCards();
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
        if (type != 0) m_CooldownTimers[type] = 7.5f;
    }

    void SetSunCount(int count) {
        m_CurrentSun = count;
        m_SunText->SetText(std::to_string(count));
    }

    void DrawUI() {
        this->Draw();

        for (int i = 0; i < m_Cards.size(); ++i) {
            m_Cards[i]->Draw(); // 先畫底下的卡片

            int type = i + 1;
            float cooldownLeft = m_CooldownTimers[type];
            int cost = (type == 1) ? 100 : 50;

            // --- 第一層：冷卻遮罩 (由下往上亮起) ---
            if (cooldownLeft > 0) {
                float progress = cooldownLeft / 7.5f; // 1.0 -> 0.0
                float originalScaleY = 0.9f;
                float pixelHeight = 80.0f; // 根據你的圖片微調，決定位移幅度

                // 高度縮小
                m_CooldownMasks[i]->m_Transform.scale = {0.9f, originalScaleY * progress};

                // 【由下往上亮起核心】：中心點向上偏移
                // 公式：(1.0 - progress) * 像素高度 / 2
                float yOffset = (1.0f - progress) * (pixelHeight / 2.0f);
                m_CooldownMasks[i]->m_Transform.translation = m_Cards[i]->m_Transform.translation + glm::vec2{0.0f, yOffset};

                m_CooldownMasks[i]->Draw();
            }

            // --- 第二層：陽光遮罩 (全蓋) ---
            if (m_CurrentSun < cost) {
                // 只要陽光不夠，就蓋上一層全尺寸的遮罩
                // 如果同時在冷卻，視覺上就會「疊兩層」，變得很黑，符合你的需求！
                m_SunMasks[i]->Draw();
            }
        }

        m_SunTextObject->Draw();
    }

    int GetSelectedType(glm::vec2 mousePos) {
        float baseLine = m_Transform.translation.x;

        // 豌豆射手
        float peaX = baseLine - 182.0f;
        if (mousePos.x > (peaX - 30.0f) && mousePos.x < (peaX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[1] <= 0 && m_CurrentSun >= 100) return 1;
        }

        // 向日葵
        float sunX = baseLine - 120.0f;
        if (mousePos.x > (sunX - 30.0f) && mousePos.x < (sunX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[2] <= 0 && m_CurrentSun >= 50) return 2;
        }

        return 0;
    }

private:
    void InitCards() {
        m_Cards.clear();
        m_CooldownMasks.clear(); // 冷卻專用
        m_SunMasks.clear();      // 陽光不足專用

        std::vector<float> offsets = {-182.0f, -120.0f};
        std::vector<std::string> paths = {
            "resources/image/UI/peashooter_card.png",
            "resources/image/UI/sunflower_card.png"
        };

        for (int i = 0; i < 2; ++i) {
            auto cardPos = m_Transform.translation + glm::vec2{offsets[i], 0.0f};

            // 1. 卡片 (Z: 95)
            auto card = std::make_shared<Util::GameObject>();
            card->SetDrawable(std::make_shared<Util::Image>(paths[i]));
            card->m_Transform.translation = cardPos;
            card->m_Transform.scale = {0.9f, 0.9f};
            card->SetZIndex(95);
            m_Cards.push_back(card);

            // 2. 冷卻遮罩 (Z: 96) - 負責由下往上亮起
            auto cMask = std::make_shared<Util::GameObject>();
            cMask->SetDrawable(std::make_shared<Util::Image>("resources/image/mask.png"));
            cMask->SetZIndex(96);
            m_CooldownMasks.push_back(cMask);

            // 3. 陽光遮罩 (Z: 97) - 陽光不足就全蓋
            auto sMask = std::make_shared<Util::GameObject>();
            sMask->SetDrawable(std::make_shared<Util::Image>("resources/image/mask.png"));
            sMask->m_Transform.translation = cardPos;
            sMask->m_Transform.scale = {0.9f, 0.9f};
            sMask->SetZIndex(97);
            m_SunMasks.push_back(sMask);
        }
    }

    std::vector<std::shared_ptr<Util::GameObject>> m_CooldownMasks;
    std::vector<std::shared_ptr<Util::GameObject>> m_SunMasks;

    std::vector<std::shared_ptr<Util::GameObject>> m_Cards;
    std::shared_ptr<Util::Text> m_SunText;
    std::shared_ptr<Util::GameObject> m_SunTextObject;

    int m_CurrentSun = 50;
    std::map<int, float> m_CooldownTimers;
};

#endif