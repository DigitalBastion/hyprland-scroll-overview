#!/usr/bin/env python3
"""Run production backing geometry/pass clipping without a compositor or GL.

Usage: python3 tests/overview-backing.py /absolute/ignored/scratch/directory
Only the supplied scratch directory receives generated sources and binaries.
"""

from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
source = (root / "scrollOverview.cpp").read_text()


def function(name):
    # Locate the definition rather than a forward declaration.
    start = source.rfind("static ", 0, source.index(name + "("))
    opening = source.index("{", source.index(name + "(", start))
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


backing_class = source[source.index("class COverviewBackingSurfacePassElement"):
                       source.index("// renderLayer queues surface passes")]
production = backing_class + "\n" + "\n".join(function(name) for name in (
    "getOverviewBox", "getOverviewWorkspaceBox",
    "getOverviewWorkspaceBackingBox", "clipOverviewLayerPassElements",
))

harness = r'''
#include <hyprutils/math/Box.hpp>
#include <hyprutils/math/Region.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
using Hyprutils::Math::CBox;
using Hyprutils::Math::Vector2D;
using Hyprutils::Math::CRegion;
namespace ScrollOverview::Config { enum class ELayout { HORIZONTAL, VERTICAL }; }
struct Monitor {
    double m_scale = 1.25;
    Vector2D m_size = {1920, 1080}, m_position = {-1920, 120};
    CBox usable = {m_position, m_size};
    CBox logicalBoxMinusReserved() const { return usable; }
};
using PHLMONITOR = std::shared_ptr<Monitor>;
static Vector2D axisOffsetVector(float offset, ScrollOverview::Config::ELayout layout) {
    return layout == ScrollOverview::Config::ELayout::HORIZONTAL ? Vector2D{offset, 0.F} : Vector2D{0.F, offset};
}
template<class T, class U> T dc(U* value) { return dynamic_cast<T>(value); }
template<class T, class U> auto makeUnique(const U& data) { return std::make_unique<T>(data); }
struct IPassElement { virtual ~IPassElement() = default; virtual CRegion opaqueRegion() { return {}; } };
struct CSurfacePassElement : IPassElement {
    struct SRenderData { CBox clipBox; } m_data;
    CSurfacePassElement() = default;
    CSurfacePassElement(const SRenderData& data) : m_data(data) {}
};
// An opaque native rect must report occlusion, unlike IPassElement's default.
// The production backing subclass must explicitly suppress that native result.
struct CRectPassElement : IPassElement {
    struct SRectData { CBox clipBox; CBox box; float alpha = 1.F; } m_data;
    CRectPassElement() = default;
    CRectPassElement(const SRectData& data) : m_data(data) {}
    CRegion opaqueRegion() override {
        return m_data.alpha == 1.F ? CRegion{m_data.box} : CRegion{};
    }
};
struct Hints : IPassElement {};
struct Entry { std::unique_ptr<IPassElement> element; };
struct Renderer { struct { std::vector<Entry> m_passElements; } m_renderPass; };
static Renderer renderer;
static auto* g_pHyprRenderer = &renderer;
'''

checks = r'''
static void equal(const CBox& a, const CBox& b) {
    assert(a.pos().distance(b.pos()) < 0.001);
    assert(a.size().distance(b.size()) < 0.001);
}
int main() {
    const auto monitor = std::make_shared<Monitor>();
    constexpr auto V = ScrollOverview::Config::ELayout::VERTICAL;
    constexpr auto H = ScrollOverview::Config::ELayout::HORIZONTAL;
    const Vector2D pan = {13.25, -7.5};
    for (const double monitorScale : {0.75, 1.0, 1.25, 2.0}) {
      monitor->m_scale = monitorScale;
    for (const auto layout : {V, H}) {
        for (const float scale : {1.F, 0.5F, 0.63F}) {
            const auto full = getOverviewWorkspaceBox(monitor, scale, pan, 243.5F, layout);
            // Explicit edge cases plus combined asymmetric reservations.
            for (const auto edges : {CBox{0,0,0,0}, CBox{0,32,0,0}, CBox{0,0,0,48},
                                     CBox{40,0,0,0}, CBox{0,0,64,0}, CBox{19,31,47,53}}) {
                const double left = edges.x, top = edges.y, right = edges.width, bottom = edges.height;
                monitor->usable = {monitor->m_position + Vector2D{left,top}, monitor->m_size - Vector2D{left+right,top+bottom}};
                auto expected = full;
                // Projection rounds origin/extent independently; derive expected
                // from the unrounded full-monitor transform for fractional scales.
                const auto raw = getOverviewBox({{},monitor->m_size}, monitor, scale, pan, 243.5F, layout, false);
                expected = CBox{raw.pos() + Vector2D{left,top} * (scale*monitor->m_scale),
                                (monitor->m_size - Vector2D{left+right,top+bottom}) * (scale*monitor->m_scale)}.round();
                equal(getOverviewWorkspaceBackingBox(monitor, scale, pan, 243.5F, layout), expected);
                equal(getOverviewWorkspaceBox(monitor, scale, pan, 243.5F, layout), full);
            }
        }
    }
    }
    monitor->usable = {monitor->m_position,{0,1080}};
    assert(getOverviewWorkspaceBackingBox(monitor, .5F, {}, 0, V).empty());
    monitor->usable = {monitor->m_position,{1920,0}};
    assert(getOverviewWorkspaceBackingBox(monitor, .5F, {}, 0, V).empty());
    // Only elements newly queued by a backing layer are clipped. Existing
    // native clips intersect, disjoint clips discard rather than disable clip.
    auto& pass = renderer.m_renderPass.m_passElements;
    pass.push_back({std::make_unique<CSurfacePassElement>()});
    pass.push_back({std::make_unique<Hints>()});
    pass.push_back({std::make_unique<CSurfacePassElement>()});
    auto rect = std::make_unique<CRectPassElement>();
    rect->m_data.clipBox = {20,20,100,100};
    rect->m_data.box = {0,0,200,200};
    assert(!rect->opaqueRegion().empty());
    pass.push_back({std::move(rect)});
    auto outside = std::make_unique<CSurfacePassElement>();
    outside->m_data.clipBox = {500,500,20,20};
    pass.push_back({std::move(outside)});
    clipOverviewLayerPassElements(1, {10,10,50,50});
    assert(pass.size() == 4);
    assert(dynamic_cast<CSurfacePassElement*>(pass[0].element.get())->m_data.clipBox.empty());
    equal(dynamic_cast<CSurfacePassElement*>(pass[2].element.get())->m_data.clipBox, {10,10,50,50});
    assert(dynamic_cast<COverviewBackingSurfacePassElement*>(pass[2].element.get()));
    assert(pass[2].element->opaqueRegion().empty());
    equal(dynamic_cast<CRectPassElement*>(pass[3].element.get())->m_data.clipBox, {20,20,40,40});
    equal(dynamic_cast<CRectPassElement*>(pass[3].element.get())->m_data.box, {0,0,200,200});
    assert(dynamic_cast<COverviewBackingRectPassElement*>(pass[3].element.get()));
    assert(pass[3].element->opaqueRegion().empty());
    pass.push_back({std::make_unique<CSurfacePassElement>()});
    assert(dynamic_cast<CSurfacePassElement*>(pass.back().element.get())->m_data.clipBox.empty());
    std::cout << "Backing geometry and deferred layer clipping passed\n";
}
'''

scratch = Path(sys.argv[1]).resolve()
assert scratch.is_dir(), "Supply an existing ignored scratch directory"
test_source = scratch / "overview-backing-test.cpp"
binary = scratch / "overview-backing-test"
test_source.write_text(harness + production + checks)
flags = shlex.split(subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "hyprutils", "pixman-1"], text=True))
subprocess.run(["g++", "-std=c++23", "-Wall", "-Wextra", "-Werror", str(test_source),
                *flags, "-o", str(binary)], check=True)
subprocess.run([str(binary)], check=True)

# Wiring checks complement the executable helpers: only backing layer passes
# receive the clip, and live/ghost wallpaper retain the old transform origin.
assert "renderOverviewWorkspaceShadow(monitor, BACKINGBOX," in source
assert "OverviewRender::queueBlur(BACKINGBOX," in source
assert "renderWallpaperLayers(monitor, WORKSPACEBOX, BACKINGBOX," in source
assert "renderOverviewWorkspaceShadow(MONITOR, BACKINGBOX," in source
assert "renderWallpaperLayers(MONITOR, GHOSTBOX, BACKINGBOX," in source
assert "LAYER_BOTTOM, WORKSPACEBOX, renderScale, now, 1.F, &BACKINGBOX)" in source
assert "LAYER_BOTTOM, GHOSTBOX, SCALE, NOW, 1.F, &BACKINGBOX)" in source
layer_render = source[source.index("static void renderOverviewLayerLevel("):
                      source.index("void CScrollOverview::renderGlobalWallpaper(")]
assert "RMOD_TYPE_TRANSLATE, workspaceBox.pos()" in layer_render
assert "if (!monitor || (clipBox && clipBox->empty()))" in layer_render
assert "clipOverviewLayerPassElements(FIRSTELEMENT, *clipBox)" in layer_render
assert source.count("clipOverviewLayerPassElements(") == 2
assert source.count("getOverviewWorkspaceBackingBox(") == 3
print("Backing/ghost wiring and clip scope passed")
