#include "../OverviewTitle.hpp"

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
    require(OverviewWindow::titleBannerHeight(textHeight + padding * 2.F + 1.F, scale, padding, textHeight) == textHeight + 1.F,
            "banner must clamp to available preview space");
    require(OverviewWindow::titleBannerHeight(textHeight + padding * 2.F, scale, padding, textHeight) == textHeight,
            "an exact text fit must remain visible");
    require(OverviewWindow::titleBannerHeight(textHeight + padding * 2.F - 1.F, scale, padding, textHeight) == 0.F,
            "text taller than available preview space must be omitted");
    require(OverviewWindow::titleBannerHeight(1.F, scale, padding, textHeight) == 0.F,
            "tiny previews must omit titles");
    for (const float pixelScale : {0.5F, 1.F, 1.5F, 2.F}) {
        const float inset = std::round(6.F * pixelScale);
        const int measured = measureTitleHeight(std::lround(24.F * pixelScale));
        const float banner = OverviewWindow::titleBannerHeight(400.F, pixelScale, inset, measured);
        require(banner >= measured + inset * 2.F, "larger preview/monitor scales must also fit requested text");
        require(banner <= 400.F - inset * 2.F, "banner must remain within preview bounds");
    }
    std::cout << "Overview title geometry: passed\n";
}
