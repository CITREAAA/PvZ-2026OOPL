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
        SetDrawable(std::make_shared<Util::Image>("resources/image/UI/seedBank.png"));
        m_Transform.scale = {1.f, 1.f};
        m_Transform.translation = {-330.0f, 270.0f};
        SetZIndex(90);

        m_SunText = std::make_shared<Util::Text>("resources/font/impact.ttf", 20, "50", Util::Color::FromRGB(0, 0, 0));
        m_SunTextObject = std::make_shared<Util::GameObject>();
        m_SunTextObject->SetDrawable(m_SunText);
        m_SunTextObject->m_Transform.translation = m_Transform.translation + glm::vec2{-251.0f, -33.0f};
        m_SunTextObject->SetZIndex(100);

        // 初始化冷卻 (1:豌豆, 2:向日葵, 3:堅果牆)
        m_CooldownTimers[1] = 0.0f;
        m_CooldownTimers[2] = 0.0f;
        m_CooldownTimers[3] = 0.0f;

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
        if (type == 3) {
            m_CooldownTimers[3] = 30.0f; // 堅果牆 30 秒
        } else if (type != 0) {
            m_CooldownTimers[type] = 7.5f; // 其他 7.5 秒
        }
    }

    void SetSunCount(int count) {
        m_CurrentSun = count;
        m_SunText->SetText(std::to_string(count));
    }

    void DrawUI() {
        this->Draw();

        for (int i = 0; i < m_Cards.size(); ++i) {
            m_Cards[i]->Draw();

            int type = i + 1;
            float cooldownLeft = m_CooldownTimers[type];
            float maxCooldown = (type == 3) ? 30.0f : 7.5f; // 取得該植物的總冷卻
            int cost = (type == 1) ? 100 : 50;

            // --- 第一層：冷卻遮罩 (由下往上亮起) ---
            if (cooldownLeft > 0) {
                float progress = cooldownLeft / maxCooldown;
                float originalScaleY = 0.9f;
                float pixelHeight = 80.0f;

                m_CooldownMasks[i]->m_Transform.scale = {0.9f, originalScaleY * progress};
                float yOffset = (1.0f - progress) * (pixelHeight / 2.0f);
                m_CooldownMasks[i]->m_Transform.translation = m_Cards[i]->m_Transform.translation + glm::vec2{0.0f, yOffset};
                m_CooldownMasks[i]->Draw();
            }

            // --- 第二層：陽光遮罩 (全蓋) ---
            if (m_CurrentSun < cost) {
                m_SunMasks[i]->Draw();
            }
        }
        m_SunTextObject->Draw();
    }

    int GetSelectedType(glm::vec2 mousePos) {
        float baseLine = m_Transform.translation.x;

        // 1. 豌豆射手 (100)
        float peaX = baseLine - 182.0f;
        if (mousePos.x > (peaX - 30.0f) && mousePos.x < (peaX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[1] <= 0 && m_CurrentSun >= 100) return 1;
        }

        // 2. 向日葵 (50)
        float sunX = baseLine - 120.0f;
        if (mousePos.x > (sunX - 30.0f) && mousePos.x < (sunX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[2] <= 0 && m_CurrentSun >= 50) return 2;
        }

        // 3. 堅果牆 (50)
        float wallX = baseLine - 58.0f;
        if (mousePos.x > (wallX - 30.0f) && mousePos.x < (wallX + 30.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) {
            if (m_CooldownTimers[3] <= 0 && m_CurrentSun >= 50) return 3;
        }

        return 0;
    }

private:
    void InitCards() {
        m_Cards.clear();
        m_CooldownMasks.clear();
        m_SunMasks.clear();

        // X 軸偏移：豌豆(-182), 向日葵(-120), 堅果牆(-58)
        std::vector<float> offsets = {-182.0f, -120.0f, -58.0f};
        std::vector<std::string> paths = {
            "resources/image/UI/peashooter_card.png",
            "resources/image/UI/sunflower_card.png",
            "resources/image/UI/wallnut_card.png"
        };

        for (int i = 0; i < 3; ++i) {
            auto cardPos = m_Transform.translation + glm::vec2{offsets[i], 0.0f};

            // 卡片
            auto card = std::make_shared<Util::GameObject>();
            card->SetDrawable(std::make_shared<Util::Image>(paths[i]));
            card->m_Transform.translation = cardPos;
            card->m_Transform.scale = {0.9f, 0.9f};
            card->SetZIndex(95);
            m_Cards.push_back(card);

            // 冷卻遮罩
            auto cMask = std::make_shared<Util::GameObject>();
            cMask->SetDrawable(std::make_shared<Util::Image>("resources/image/mask.png"));
            cMask->SetZIndex(96);
            m_CooldownMasks.push_back(cMask);

            // 陽光遮罩
            auto sMask = std::make_shared<Util::GameObject>();
            sMask->SetDrawable(std::make_shared<Util::Image>("resources/image/mask.png"));
            sMask->m_Transform.translation = cardPos;
            sMask->m_Transform.scale = {0.9f, 0.9f};
            sMask->SetZIndex(97);
            m_SunMasks.push_back(sMask);
        }
    }

    std::vector<std::shared_ptr<Util::GameObject>> m_Cards;
    std::vector<std::shared_ptr<Util::GameObject>> m_CooldownMasks;
    std::vector<std::shared_ptr<Util::GameObject>> m_SunMasks;

    std::shared_ptr<Util::Text> m_SunText;
    std::shared_ptr<Util::GameObject> m_SunTextObject;

    int m_CurrentSun = 50;
    std::map<int, float> m_CooldownTimers;
};

#endif