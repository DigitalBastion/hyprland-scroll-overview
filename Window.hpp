#pragma once

#include "globals.hpp"
#include <hyprland/src/helpers/time/Time.hpp>
#include <vector>

namespace OverviewWindow {

bool shouldBlurBackground(const PHLWINDOW& window);
bool shouldUsePrecomputedBlur(const PHLWINDOW& window, PHLMONITOR monitor = nullptr, const CBox* workspaceBox = nullptr, const CBox* windowBox = nullptr, bool dragged = false);
bool shouldUseBlurFramebuffer(const PHLWINDOW& window, PHLMONITOR monitor = nullptr, const CBox* workspaceBox = nullptr, const CBox* windowBox = nullptr);
void forceDecoRecalc(const PHLWINDOW& window);

struct STitleNeighbor {
    PHLWINDOW window;
    CBox      box;
};

struct SRenderParams {
    PHLMONITOR            monitor;
    PHLWINDOW             window;
    CBox                  windowBox;
    float                 renderScale          = 1.F;
    Time::steady_tp       now                  = {};
    const CBox*           workspaceBox         = nullptr;
    bool                  selected             = false;
    bool                  dragged              = false;
    PHLWINDOW             pseudoFocusWindow;
    std::vector<STitleNeighbor> titleNeighbors;
};

void renderOverviewWindow(const SRenderParams& params);

}
