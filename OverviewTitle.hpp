#pragma once

#include <algorithm>
#include <cmath>

namespace OverviewWindow {

// Return zero when the measured title cannot fit inside the preview.
inline float titleBannerHeight(float previewHeight, float scale, float padding, float textHeight) {
    const float height = std::min(std::max(std::round(24.F * scale), textHeight + padding * 2.F), previewHeight - padding * 2.F);
    return textHeight > height ? 0.F : height;
}

} // namespace OverviewWindow
