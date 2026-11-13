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

// One strip inside the left header, only along its shared edge with a right
// header. Half-pixel rounding tolerance does not bridge deliberate gaps.
inline Hyprutils::Math::CBox titleDividerBox(const Hyprutils::Math::CBox& left, const Hyprutils::Math::CBox& right, float width, float scale) {
    if (left.empty() || right.empty() || width <= 0.F || std::abs(left.x + left.width - right.x) > 0.5)
        return {};
    const double top = std::max(left.y, right.y);
    const double bottom = std::min(left.y + left.height, right.y + right.height);
    if (bottom <= top)
        return {};
    const double pixels = std::min(left.width, double(std::max(1.F, std::round(width * scale))));
    return {left.x + left.width - pixels, top, pixels, bottom - top};
}

} // namespace OverviewWindow
