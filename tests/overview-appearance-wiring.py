#!/usr/bin/env python3
"""Static integration checks complement native appearance-helper regressions."""
from pathlib import Path

root = Path(__file__).resolve().parent.parent
overview = (root / "scrollOverview.cpp").read_text()
window = (root / "Window.cpp").read_text()
config = (root / "Config.cpp").read_text()
assert 'CFloatValue>("plugin:scrolloverview:dim:strength"' in config
assert 'SFloatValueOptions{.min = -1.F, .max = 1.F}' in config
assert 'CFloatValue>("plugin:scrolloverview:title:divider_width"' in config
assert '"divider width between adjoining overview title headers", 0.F,' in config
assert 'CIntValue>("plugin:scrolloverview:title:divider_color"' in config
assert '"overview title divider color", 0x66FFFFFF)' in config
live = overview[overview.index("void CScrollOverview::renderWindowLive("):
                overview.index("void CScrollOverview::redrawAll(")]
workspace = overview[overview.index("void CScrollOverview::renderWorkspaceLive("):
                     overview.index("void CScrollOverview::renderDraggedWindow(")]

# The eligible path and its neighbors use the same projection, scale and
# insertion-aware offsets. Overlays have no workspace backing context.
assert "getOverviewWindowBox(window, monitor, renderScale, viewOffset->value(), WORKSPACEOFFSET, layout)" in workspace
assert "renderWindowLive(monitor, window, windowBox, renderScale, now, &WORKSPACEBOX)" in workspace
assert "workspaceOverviewOffset(i, activeWorkspaceIndex(), getWorkspaceRenderedPitch(monitor, renderScale, layout))" in live
assert "getOverviewWindowBox(neighbor, monitor, renderScale, viewOffset->value(), offset, layout)" in live
assert "workspaceBox && !dragged && !window->m_isFloating && !Fullscreen::controller()->isFullscreen(window)" in live
assert "neighbor == DRAGGED || neighbor->m_isFloating" in live
assert "Fullscreen::controller()->isFullscreen(neighbor)" in live
assert "if (!overviewBoxIntersectsMonitor(box, monitor))" in live
assert ".titleNeighbors       = std::move(titleNeighbors)" in live
assert "renderWindowLive(monitor, WINDOW, windowBox, dragOwner->scale->value(), now, nullptr, true)" in overview
assert "renderWindowLive(monitor, window, windowBox, renderScale, now);" in overview

# Neighbor eligibility is not enough: an actual second title must render and
# fit, and the final rect is clipped to the shared-header divider geometry.
assert "renderOverviewWindowTitle(params.monitor, params.window, params.windowBox, metrics, params.titleNeighbors)" in window
assert "overviewTitleTexture(monitor, neighbor.window, neighbor.box, scale, textColor)" in window
assert "if (!neighborTexture || !neighborTexture->ok())" in window
assert "titleBannerBox(neighbor.box, scale, padding, neighborTexture->m_size.y)" in window
assert "titleDividerBox(titleBox, neighborTitle, dividerWidth, scale)" in window
assert ".roundingPower = metrics.roundingPower, .clipBox = divider" in window
assert "previewSelected(params.selected, bool(params.pseudoFocusWindow), params.window == params.pseudoFocusWindow)" in window
assert "dimState.emplace(window->m_dimPercent->value(), previewDim(" in window
assert 'getValue<float>("plugin:scrolloverview:dim:strength", monitor)' in window
assert "window->m_ruleApplicator->noDim().valueOrDefault()" in window
print("Appearance render-parameter wiring: passed")
