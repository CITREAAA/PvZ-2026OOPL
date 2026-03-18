#ifndef SEEDBANK_HPP
#define SEEDBANK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"
#include "Util/Color.hpp"
#include <vector>
#include <memory>
#include <string>

class SeedBank : public Util::GameObject {
public:
    SeedBank() {
        // 背景木框
        SetDrawable(std::make_shared<Util::Image>("resources/image/UI/seedBank.png"));

        m_Transform.scale = {1.f, 1.f};
        m_Transform.translation = {-330.0f, 270.0f};
        SetZIndex(90);

        m_SunText = std::make_shared<Util::Text>("resources/font/impact.ttf", 20, "50", Util::Color::FromRGB(0, 0, 0));

        m_SunTextObject = std::make_shared<Util::GameObject>();
        m_SunTextObject->SetDrawable(m_SunText);
        m_SunTextObject->m_Transform.translation = m_Transform.translation + glm::vec2{-250.0f, -33.0f};
        m_SunTextObject->SetZIndex(95); // 數字要最上層

        InitCards();
    }

    void SetSunCount(int count) {
        m_SunText->SetText(std::to_string(count));
    }

    // 讓 App.cpp 呼叫，畫出板子上的卡片
    void DrawUI() {
        this->Draw(); // 畫自己 (背景)
        for (auto& card : m_Cards) card->Draw(); // 畫卡片
        m_SunTextObject->Draw(); // 畫文字
    }

    int GetSelectedType(glm::vec2 mousePos) {
        // 取得板子當前中心 X 座標作為基準 (目前是 -330.0f)
        float baseLine = m_Transform.translation.x;

        // 1. 豌豆卡片判定 (中心在 baseLine - 182 = -512)
        // 範圍抓中心點左右各 30 像素
        float peaX = baseLine - 182.0f;
        if (mousePos.x > (peaX - 30.0f) && mousePos.x < (peaX + 30.0f)) {
            // 也要檢查 Y 軸，確保點在板子高度內 (板子 Y 是 270，高度約 100)
            if (mousePos.y > 220.0f && mousePos.y < 320.0f) {
                return 1; // Peashooter
            }
        }

        // 2. 向日葵卡片判定 (中心在 baseLine - 120 = -450)
        float sunX = baseLine - 120.0f;
        if (mousePos.x > (sunX - 30.0f) && mousePos.x < (sunX + 30.0f)) {
            if (mousePos.y > 220.0f && mousePos.y < 320.0f) {
                return 2; // Sunflower
            }
        }

        return 0; // 沒點到任何卡片
    }

private:
    void InitCards() {
        // --- 關鍵：清除舊卡片防止重複生成 ---
        m_Cards.clear();

        // 1. 豌豆卡片
        auto peaCard = std::make_shared<Util::GameObject>();
        peaCard->SetDrawable(std::make_shared<Util::Image>("resources/image/UI/peashooter_card.png"));

        peaCard->m_Transform.translation = m_Transform.translation + glm::vec2{-182.0f, 0.0f};
        peaCard->m_Transform.scale = {0.9f, 0.9f};
        peaCard->SetZIndex(95); // 務必比板子的 90 高！
        m_Cards.push_back(peaCard);

        // 2. 向日葵卡片
        auto sunCard = std::make_shared<Util::GameObject>();
        sunCard->SetDrawable(std::make_shared<Util::Image>("resources/image/UI/sunflower_card.png"));

        sunCard->m_Transform.translation = m_Transform.translation + glm::vec2{-120.0f, 0.0f};
        sunCard->m_Transform.scale = {0.9f, 0.9f};
        sunCard->SetZIndex(95);
        m_Cards.push_back(sunCard);
    }

    std::vector<std::shared_ptr<Util::GameObject>> m_Cards;
    std::shared_ptr<Util::Text> m_SunText;
    std::shared_ptr<Util::GameObject> m_SunTextObject;
};

#endif