#include "Sun.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

static std::vector<std::string> GetFramesFromFolder(const std::string& folderPath) {
    std::vector<std::string> paths;
    // 檢查路徑是否存在，避免 fs 拋出異常
    if (!fs::exists(folderPath)) return paths;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string p = entry.path().string();
            std::string ext = entry.path().extension().string();

            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                std::replace(p.begin(), p.end(), '\\', '/'); // 統一斜線格式
                paths.push_back(p);
            }
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

Sun::Sun(float startX, float startY, float targetY) : m_TargetY(targetY) {
    auto frames = GetFramesFromFolder("resources/image/sun");
    // 確保有抓到圖片再設定 Drawable
    if (!frames.empty()) {
        auto anim = std::make_shared<Util::Animation>(frames, true, 50, true);
        m_Drawable = anim;
    }

    // 設定起始座標
    m_Transform.translation = {startX, startY};

    m_ZIndex = 100; // 確保在植物之上
    m_Transform.scale = {0.8f, 0.8f};

    // 初始化計時器
    m_SurvivalTimer = 0.0f;
    m_IsExpired = false;
}

void Sun::Update(float dt) {
    if (m_Transform.translation.y > m_TargetY) {
        float fallSpeed = 100.0f; // 掉落速度
        m_Transform.translation.y -= fallSpeed * dt;
    }

    // 2. 存活計時邏輯：生成後 15 秒自動標記為過期
    m_SurvivalTimer += dt;
    if (m_SurvivalTimer >= 15.0f) {
        m_IsExpired = true;
    }
}

bool Sun::IsClicked(glm::vec2 mousePos) {
    // 簡單的圓形碰撞判定 (半徑 40)
    float distance = glm::distance(mousePos, m_Transform.translation);
    return distance < 40.0f;
}