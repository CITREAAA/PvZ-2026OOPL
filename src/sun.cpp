#include "Sun.hpp"
#include "Util/Time.hpp"
#include <filesystem>
#include <algorithm>

#include "Util/Logger.hpp"

namespace fs = std::filesystem;

static std::vector<std::string> GetFramesFromFolder(const std::string& folderPath) {
    std::vector<std::string> paths;
    if (!fs::exists(folderPath)) return paths;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string p = entry.path().string();

            if (p.empty()) continue;

            std::replace(p.begin(), p.end(), '\\', '/');
            paths.push_back(p);
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

Sun::Sun(float startX, float targetY) : m_TargetY(targetY) {
    auto frames = GetFramesFromFolder("resources/image/sun");
    auto anim = std::make_shared<Util::Animation>(frames, true, 50, true);
    m_Drawable = anim;

    m_Transform.translation = {startX, 320.0f};

    m_ZIndex = 50;
    m_Transform.scale = {0.8f, 0.8f};
}

void Sun::Update() {
    // 檢查目前高度是否高於目標高度
    if (m_Transform.translation.y > m_TargetY) {
        float fallSpeed = 100.0f; // 每秒掉落的像素
        m_Transform.translation.y -= fallSpeed * static_cast<float>(Util::Time::GetDeltaTime());
    }

    if (m_Transform.translation.y < m_TargetY) {
        m_Transform.translation.y = m_TargetY;
    }
}

bool Sun::IsClicked(glm::vec2 mousePos) {
    // 碰撞偵測：計算滑鼠與陽光中心的距離
    // 陽光圖片大約 80x80 像素，所以判定範圍設為 40
    float distance = glm::distance(mousePos, m_Transform.translation);
    return distance < 40.0f;
}