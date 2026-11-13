#include "../OverviewTitle.hpp"
#include "../OverviewAppearance.hpp"

#include <pango/pangocairo.h>

#include <cstdlib>
#include <iostream>

static void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

// Headless native text metrics; no compositor, window, or GL context needed.
static int measureTitleHeight(int fontSize) {
    auto* context = pango_font_map_create_context(pango_cairo_font_map_get_default());
    auto* layout  = pango_layout_new(context);
    auto* font    = pango_font_description_from_string("DejaVu Sans Bold");
    pango_font_description_set_absolute_size(font, fontSize * PANGO_SCALE);
    pango_layout_set_font_description(layout, font);
    pango_layout_set_text(layout, "Readable overview title", -1);
    pango_layout_set_width(layout, 288 * PANGO_SCALE);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
    pango_layout_set_single_paragraph_mode(layout, true);
    int height = 0;
    pango_layout_get_pixel_size(layout, nullptr, &height);
    pango_font_description_free(font);
    g_object_unref(layout);
    g_object_unref(context);
    return height;
}

int main() {
    // User configuration: font_size=24, overview scale=0.5, monitor scale=1.
    constexpr float scale   = 0.5F;
    constexpr float padding = 3.F;
    const int textHeight    = measureTitleHeight(12);
    const float height      = OverviewWindow::titleBannerHeight(200.F, scale, padding, textHeight);
    std::cout << "font_size=24, scale=0.5: measured text=" << textHeight
              << "px, banner=" << height << "px" << std::endl;
    require(textHeight > 12, "native text metrics must exercise the original 12px banner overflow");
    require(height > 0.F, "font_size=24 must not make a fitting title disappear");
    require(height >= textHeight + padding * 2.F, "banner must include measured text and vertical padding");

    // Preserve the old minimum banner with a smaller/default title.
    require(OverviewWindow::titleBannerHeight(200.F, scale, padding, 6.F) == 12.F,
            "small text must retain the default scaled banner height");
    // Use remaining space without shrinking or clipping text when padding cannot all fit.
    require(OverviewWindow::titleBannerHeight(textHeight + 1.F, scale, padding, textHeight) == textHeight + 1.F,
            "attached banner must use the full preview height without exterior padding");
    require(OverviewWindow::titleBannerHeight(textHeight, scale, padding, textHeight) == textHeight,
            "an exact text fit must remain visible without exterior padding");
    require(OverviewWindow::titleBannerHeight(textHeight - 1.F, scale, padding, textHeight) == 0.F,
            "text taller than available preview space must be omitted");
    require(OverviewWindow::titleBannerHeight(1.F, scale, padding, textHeight) == 0.F,
            "tiny previews must omit titles");
    for (const float pixelScale : {0.5F, 1.F, 1.5F, 2.F}) {
        const float inset = std::round(6.F * pixelScale);
        const int measured = measureTitleHeight(std::lround(24.F * pixelScale));
        const float banner = OverviewWindow::titleBannerHeight(400.F, pixelScale, inset, measured);
        require(banner >= measured + inset * 2.F, "larger preview/monitor scales must also fit requested text");
        require(banner <= 400.F, "banner must remain within preview bounds");
    }
    for (const float pixelScale : {0.375F, 0.5F, 0.7875F, 1.25F, 2.F}) {
        const float innerPadding = std::round(6.F * pixelScale);
        const int measured = measureTitleHeight(std::lround(24.F * pixelScale));
        for (const auto origin : {Hyprutils::Math::Vector2D{0,0}, Hyprutils::Math::Vector2D{-140.25,37.75}}) {
            const Hyprutils::Math::CBox preview{origin, {350.5, 200.25}};
            const auto banner = OverviewWindow::titleBannerBox(preview, pixelScale, innerPadding, measured);
            require(banner.x == preview.x && banner.y == preview.y && banner.width == preview.width,
                    "background must attach to the preview top and full width at every origin/scale");
            require(banner.height >= measured + 2.F * innerPadding && banner.height <= preview.height,
                    "attached header must contain measured text and internal padding");
            const auto exact = OverviewWindow::titleBannerBox({origin,{350.5, double(measured)}}, pixelScale, innerPadding, measured);
            require(exact.height == measured && exact.y == origin.y,
                    "exact text fit must not reserve exterior height");
            require(OverviewWindow::titleBannerBox({origin,{350.5, double(measured-1)}}, pixelScale, innerPadding, measured).empty(),
                    "too-short previews must omit the header");
            require(OverviewWindow::titleBannerBox({origin,{double(2.F*innerPadding),200.25}}, pixelScale, innerPadding, measured).empty(),
                    "too-narrow previews must omit the header rather than lose internal text padding");
        }
    }
    std::cout << "Overview title geometry: passed\n";
    for (const float scale : {0.5F, 1.F, 1.25F, 2.F}) {
        const Hyprutils::Math::CBox left{-80.25,30.5,200,24*scale};
        const Hyprutils::Math::CBox right{119.75,30.5,300,30*scale};
        const auto seam = OverviewWindow::titleDividerBox(left, right, 1.5F, scale);
        require(seam.width == std::max(1.F, std::round(1.5F*scale)), "divider width scales in pixels");
        require(seam.y == left.y && seam.height == left.height && seam.x + seam.width == right.x,
                "divider is inside the shared header edge, never full preview height");
        require(OverviewWindow::titleDividerBox(left, right, 0, scale).empty(), "zero width disables divider");
        require(OverviewWindow::titleDividerBox(left, {130,30.5,300,30}, 1.5F, scale).empty(), "no divider across gaps");
        require(OverviewWindow::titleDividerBox(left, {-80.25,150,200,30}, 1.5F, scale).empty(), "no divider for vertical stacks");
        require(OverviewWindow::titleDividerBox(left, {}, 1.5F, scale).empty(), "no divider without a second header");
        const auto partial = OverviewWindow::titleDividerBox(left, {119.75,left.y + left.height/2,300,200}, 1.5F, scale);
        require(partial.y == left.y + left.height/2 && partial.height == left.height/2,
                "unequal header tops restrict divider to shared title height");
        const auto wide = OverviewWindow::titleDividerBox(left, right, 1000, scale);
        require(wide.x == left.x && wide.width == left.width, "divider cannot escape the header");
    }
    using namespace OverviewWindow;
    require(previewSelected(true,false,false), "navigation selection is bright");
    require(!previewSelected(true,true,false) && previewSelected(false,true,true), "drag pseudo-selection takes precedence");
    require(previewDim(false,false,.3F,false,.2F,false,false) == .3F, "overview dim overrides disabled global dim");
    require(previewDim(true,false,.3F,true,.2F,false,false) == 0, "selected preview remains bright");
    require(previewDim(false,true,.3F,true,.2F,false,false) == 0, "noDim rule suppresses inactive dim");
    require(previewDim(false,false,-1.F,true,.2F,false,false) == .2F, "default inherits global dim");
    require(previewDim(false,false,-1.F,false,.2F,false,false) == 0, "default inherits global dim disabled");
    require(previewDim(false,false,0.F,true,.2F,false,false) == 0, "explicit zero overrides global dim");
    require(previewDim(false,false,1.F,false,.2F,false,false) == 1, "full-strength override is supported");
    require(std::abs(previewDim(false,false,.3F,false,.2F,true,true) - .65F) < .0001F, "modal dim composes with inactive dim");
    require(previewDim(true,true,.3F,false,.2F,true,true) == .5F, "preserve independent modal dim semantics");
    float nativeDim = .17F;
    {
        CPreviewDimState state(nativeDim, .3F);
        require(nativeDim == .3F, "preview applies temporary native dim");
        { CPreviewDimState nested(nativeDim, 0.F); }
        require(nativeDim == .3F, "nested preview restores enclosing dim");
    }
    require(nativeDim == .17F, "preview restores native dim after rendering");
    std::cout << "Title divider and preview dim state: passed\n";
}
