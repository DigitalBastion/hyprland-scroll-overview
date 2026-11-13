#pragma once

#include <algorithm>
#include <cmath>
#include <hyprutils/math/Box.hpp>

namespace OverviewWindow {

// Return zero when the measured title cannot fit inside the preview.
inline float titleBannerHeight(float previewHeight, float scale, float padding, float textHeight) {
    const float height = std::min(std::max(std::round(24.F * scale), textHeight + padding * 2.F), previewHeight);
    return textHeight > height ? 0.F : height;
}

inline Hyprutils::Math::CBox titleBannerBox(const Hyprutils::Math::CBox& preview, float scale, float padding, float textHeight) {
    if (preview.empty() || std::floor(preview.width - padding * 2.F) < 1.F)
        return {};

    const float height = titleBannerHeight(preview.height, scale, padding, textHeight);
    return height > 0.F ? Hyprutils::Math::CBox{preview.x, preview.y, preview.width, height} : Hyprutils::Math::CBox{};
}

} // namespace OverviewWindow
