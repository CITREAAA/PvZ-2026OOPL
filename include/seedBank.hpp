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
        m_ToolbarFull = std::make_shared<Util::Image>("resources/image/UI/toolbar.png");
        m_ToolbarNoShovel = std::make_shared<Util::Image>("resources/image/UI/toolbarWithoutShovel.png");

        SetDrawable(m_ToolbarFull);
        m_Transform.scale = {1.f, 1.f};
        m_Transform.translation = {-270.0f, 270.0f};
        SetZIndex(90);

        m_SunText = std::make_shared<Util::Text>("resources/font/impact.ttf", 20, "50", Util::Color::FromRGB(0, 0, 0, 255));
        m_SunTextObject = std::make_shared<Util::GameObject>();
        m_SunTextObject->SetDrawable(m_SunText);
        m_SunTextObject->m_Transform.translation = m_Transform.translation + glm::vec2{-301.0f, -33.0f};
        m_SunTextObject->SetZIndex(100);

        // 初始化所有可能植物的冷卻計時器
        // 1:豌豆, 2:向日葵, 3:堅果, 5:地雷, 6:雪花, 7:櫻桃, 8:陽光菇, 9:小噴菇, 10:大噴菇, 11:膽小菇, 12:連發豌豆
        std::vector<int> all = {1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12};
        for (int t : all) m_CooldownTimers[t] = 0.0f;
    }

    void InitCards(const std::vector<int>& allowedTypes) {
        m_Cards.clear();
        m_CooldownMasks.clear();
        m_SunMasks.clear();
        m_CurrentAllowedTypes = allowedTypes;

        // 植物資料庫（路徑與你的檔名對應）
        std::map<int, std::string> cardPaths = {
            {1, "resources/image/UI/peashooter_card.png"},
            {2, "resources/image/UI/sunflower_card.png"},
            {3, "resources/image/UI/wallnut_card.png"},
            {5, "resources/image/UI/potatomine_card.png"},
            {6, "resources/image/UI/snowpea_card.png"},
            {7, "resources/image/UI/cherrybomb_card.png"},
            {8, "resources/image/UI/sunshroom_card.png"},
            {9, "resources/image/UI/puffshroom_card.png"},
            {10, "resources/image/UI/fumeshroom_card.png"},
            {11, "resources/image/UI/scaredyshroom_card.png"},
            {12, "resources/image/UI/repeaterpea_card.png"}
        };

        for (int i = 0; i < (int)allowedTypes.size(); ++i) {
            int type = allowedTypes[i];
            float offsetX = -232.0f + (i * 63.0f); // 每個卡片間距 62 像素
            auto cardPos = m_Transform.translation + glm::vec2{offsetX, 0.0f};

            auto card = std::make_shared<Util::GameObject>();
            card->SetDrawable(std::make_shared<Util::Image>(cardPaths[type]));
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
        // 設定冷卻時間
        if (type == 3 || type == 5) m_CooldownTimers[type] = 30.0f;
        else if (type == 7) m_CooldownTimers[type] = 50.0f;
        else m_CooldownTimers[type] = 7.5f; // 其餘蘑菇與豌豆多為 7.5s
    }

    void SetSunCount(int count) {
        m_CurrentSun = count;
        m_SunText->SetText(std::to_string(count));
    }

    void DrawUI() {
        this->Draw();

        for (int i = 0; i < (int)m_Cards.size(); ++i) {
            m_Cards[i]->Draw();

            int type = m_CurrentAllowedTypes[i];
            float cooldownLeft = m_CooldownTimers[type];

            float maxCD = (type == 3 || type == 5) ? 30.0f : (type == 7 ? 50.0f : 7.5f);
            int cost = GetCost(type);

            if (cooldownLeft > 0) {
                float progress = cooldownLeft / maxCD;
                m_CooldownMasks[i]->m_Transform.scale = {0.9f, 0.9f * progress};
                float yOffset = (1.0f - progress) * (80.0f / 2.0f);
                m_CooldownMasks[i]->m_Transform.translation = m_Cards[i]->m_Transform.translation + glm::vec2{0.0f, yOffset};
                m_CooldownMasks[i]->Draw();
            }

            if (m_CurrentSun < cost) m_SunMasks[i]->Draw();
        }
        m_SunTextObject->Draw();
    }

    int GetSelectedType(glm::vec2 mousePos) {
        float baseLine = m_Transform.translation.x;

        for (int i = 0; i < (int)m_CurrentAllowedTypes.size(); ++i) {
            int type = m_CurrentAllowedTypes[i];
            float cardX = baseLine - 232.0f + (i * 62.0f);
            int cost = GetCost(type);

            if (mousePos.x > (cardX - 30.0f) && mousePos.x < (cardX + 30.0f) &&
                mousePos.y > 220.0f && mousePos.y < 320.0f) {
                if (m_CooldownTimers[type] <= 0 && m_CurrentSun >= cost) return type;
            }
        }

        float shovelX = baseLine + 280.0f;
        if (m_IsShovelVisible && mousePos.x > (shovelX - 35.0f) && mousePos.x < (shovelX + 70.0f) &&
            mousePos.y > 220.0f && mousePos.y < 320.0f) return 4;

        return 0;
    }

private:
    // 🚩 封裝陽光花費邏輯
    int GetCost(int type) {
        switch(type) {
            case 1: return 100; // 豌豆
            case 2: return 50;  // 向日葵
            case 3: return 50;  // 堅果
            case 5: return 25;  // 地雷
            case 6: return 175; // 雪花
            case 7: return 150; // 櫻桃
            case 8: return 25;  // 陽光菇
            case 9: return 0;   // 小噴菇
            case 10: return 75; // 大噴菇
            case 11: return 25; // 膽小菇
            case 12: return 200;// 連發豌豆
            default: return 0;
        }
    }

    std::vector<std::shared_ptr<Util::GameObject>> m_Cards;
    std::vector<std::shared_ptr<Util::GameObject>> m_CooldownMasks;
    std::vector<std::shared_ptr<Util::GameObject>> m_SunMasks;
    std::vector<int> m_CurrentAllowedTypes;

    std::shared_ptr<Util::Image> m_ToolbarFull;
    std::shared_ptr<Util::Image> m_ToolbarNoShovel;
    bool m_IsShovelVisible = true;

    std::shared_ptr<Util::Text> m_SunText;
    std::shared_ptr<Util::GameObject> m_SunTextObject;

    int m_CurrentSun = 50;
    std::map<int, float> m_CooldownTimers;
};

#endif