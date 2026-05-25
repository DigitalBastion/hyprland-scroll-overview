#include "OverviewRender.hpp"
#include <chrono>
#include <cmath>
#include <sstream>
#define private public
#define protected public
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/helpers/math/Math.hpp>
#undef protected
#undef private

using namespace Render;
using Render::GL::CHyprOpenGLImpl;
using Render::GL::g_pHyprOpenGL;

namespace OverviewRender {

void flushPass(PHLMONITOR monitor) {
    if (!monitor || g_pHyprRenderer->m_renderPass.empty())
        return;

    g_pHyprRenderer->m_renderPass.render(CRegion{CBox{{}, monitor->m_transformedSize}});
    g_pHyprRenderer->m_renderPass.clear();
}

void renderBlur(PHLMONITOR monitor, const CBox& windowBox, int rounding, float roundingPower, float alpha, bool usePrecomputedBlur) {
    if (!monitor || alpha <= 0.F)
        return;

    CRegion blurDamage{windowBox};
    if (blurDamage.empty())
        return;

    CRegion drawDamage{CBox{{}, monitor->m_transformedSize}};

    const auto SAVEDFB        = g_pHyprRenderer->m_renderData.currentFB;
    const auto BLURREDTEXTURE = usePrecomputedBlur ? g_pHyprRenderer->getBlurTexture(monitor) : g_pHyprRenderer->blurMainFramebuffer(alpha, &blurDamage);

    if (SAVEDFB)
        SAVEDFB->bind();

    if (!BLURREDTEXTURE)
        return;

    CBox transformedBox = windowBox;
    transformedBox.transform(Math::wlTransformToHyprutils(Math::invertTransform(monitor->m_transform)), monitor->m_transformedSize.x, monitor->m_transformedSize.y);

    const CBox monitorSpaceBox = {transformedBox.pos().x / monitor->m_pixelSize.x * monitor->m_transformedSize.x,
                                  transformedBox.pos().y / monitor->m_pixelSize.y * monitor->m_transformedSize.y,
                                  transformedBox.width / monitor->m_pixelSize.x * monitor->m_transformedSize.x,
                                  transformedBox.height / monitor->m_pixelSize.y * monitor->m_transformedSize.y};

    CHyprOpenGLImpl::STextureRenderData renderData;
    renderData.damage                               = &drawDamage;
    renderData.a                                    = alpha;
    renderData.round                                = rounding;
    renderData.roundingPower                        = roundingPower;
    renderData.allowCustomUV                        = true;
    renderData.allowDim                             = false;

    g_pHyprRenderer->pushMonitorTransformEnabled(true);
    const auto SAVEDRENDERMODIF                     = g_pHyprRenderer->m_renderData.renderModif;
    const auto SAVEDUVTOPLEFT                       = g_pHyprRenderer->m_renderData.primarySurfaceUVTopLeft;
    const auto SAVEDUVBOTTOMRIGHT                   = g_pHyprRenderer->m_renderData.primarySurfaceUVBottomRight;
    g_pHyprRenderer->m_renderData.renderModif       = {};
    g_pHyprRenderer->m_renderData.primarySurfaceUVTopLeft     = monitorSpaceBox.pos() / monitor->m_transformedSize;
    g_pHyprRenderer->m_renderData.primarySurfaceUVBottomRight = (monitorSpaceBox.pos() + monitorSpaceBox.size()) / monitor->m_transformedSize;
    g_pHyprOpenGL->renderTexture(BLURREDTEXTURE, windowBox, renderData);
    g_pHyprRenderer->m_renderData.primarySurfaceUVTopLeft     = SAVEDUVTOPLEFT;
    g_pHyprRenderer->m_renderData.primarySurfaceUVBottomRight = SAVEDUVBOTTOMRIGHT;
    g_pHyprRenderer->m_renderData.renderModif                 = SAVEDRENDERMODIF;
    g_pHyprRenderer->popMonitorTransformEnabled();
}

}
