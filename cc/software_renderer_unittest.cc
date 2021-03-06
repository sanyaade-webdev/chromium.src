// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"

#include "cc/software_renderer.h"

#include "cc/quad_sink.h"
#include "cc/render_pass.h"
#include "cc/render_pass_draw_quad.h"
#include "cc/settings.h"
#include "cc/single_thread_proxy.h" // For DebugScopedSetImplThread
#include "cc/solid_color_draw_quad.h"
#include "cc/test/animation_test_common.h"
#include "cc/test/fake_web_compositor_output_surface.h"
#include "cc/test/fake_web_compositor_software_output_device.h"
#include "cc/test/geometry_test_utils.h"
#include "cc/test/render_pass_test_common.h"
#include "cc/test/test_common.h"
#include "cc/tile_draw_quad.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using namespace cc;
using namespace WebKit;
using namespace WebKitTests;

namespace {

class SoftwareRendererTest : public testing::Test, public RendererClient {
public:
    void initializeRenderer() {
        m_outputSurface = FakeWebCompositorOutputSurface::createSoftware(scoped_ptr<WebKit::WebCompositorSoftwareOutputDevice>(new FakeWebCompositorSoftwareOutputDevice));
        m_resourceProvider = ResourceProvider::create(m_outputSurface.get());
        m_renderer = SoftwareRenderer::create(this, resourceProvider(), softwareDevice());
    }

    WebCompositorSoftwareOutputDevice* softwareDevice() const { return m_outputSurface->softwareDevice(); }
    FakeWebCompositorOutputSurface* outputSurface() const { return m_outputSurface.get(); }
    ResourceProvider* resourceProvider() const { return m_resourceProvider.get(); }
    SoftwareRenderer* renderer() const { return m_renderer.get(); }
    void setViewportSize(gfx::Size viewportSize) { m_viewportSize = viewportSize; }

    // RendererClient implementation.
    virtual const gfx::Size& deviceViewportSize() const OVERRIDE { return m_viewportSize; }
    virtual const LayerTreeSettings& settings() const OVERRIDE { return m_settings; }
    virtual void didLoseContext() OVERRIDE { }
    virtual void onSwapBuffersComplete() OVERRIDE { }
    virtual void setFullRootLayerDamage() OVERRIDE { }
    virtual void setManagedMemoryPolicy(const ManagedMemoryPolicy& policy) OVERRIDE { };
    virtual void enforceManagedMemoryPolicy(const ManagedMemoryPolicy& policy) OVERRIDE { };

protected:
    DebugScopedSetImplThread m_alwaysImplThread;

    scoped_ptr<FakeWebCompositorOutputSurface> m_outputSurface;
    scoped_ptr<ResourceProvider> m_resourceProvider;
    scoped_ptr<SoftwareRenderer> m_renderer;
    gfx::Size m_viewportSize;
    LayerTreeSettings m_settings;
};

TEST_F(SoftwareRendererTest, solidColorQuad)
{
    gfx::Size outerSize(100, 100);
    int outerPixels = outerSize.width() * outerSize.height();
    gfx::Size innerSize(98, 98);
    gfx::Rect outerRect(gfx::Point(), outerSize);
    gfx::Rect innerRect(gfx::Point(1, 1), innerSize);
    setViewportSize(outerSize);

    initializeRenderer();

    scoped_ptr<SharedQuadState> sharedQuadState = SharedQuadState::create(WebTransformationMatrix(), outerRect, outerRect, 1.0, true);
    RenderPass::Id rootRenderPassId = RenderPass::Id(1, 1);
    scoped_ptr<TestRenderPass> rootRenderPass = TestRenderPass::create(rootRenderPassId, outerRect, WebTransformationMatrix());
    scoped_ptr<DrawQuad> outerQuad = SolidColorDrawQuad::create(sharedQuadState.get(), outerRect, SK_ColorYELLOW).PassAs<DrawQuad>();
    scoped_ptr<DrawQuad> innerQuad = SolidColorDrawQuad::create(sharedQuadState.get(), innerRect, SK_ColorCYAN).PassAs<DrawQuad>();
    rootRenderPass->appendQuad(innerQuad.Pass());
    rootRenderPass->appendQuad(outerQuad.Pass());

    RenderPassList list;
    RenderPassIdHashMap hashmap;
    list.push_back(rootRenderPass.get());
    hashmap.add(rootRenderPassId, rootRenderPass.PassAs<RenderPass>());
    renderer()->drawFrame(list, hashmap);

    scoped_array<SkColor> pixels(new SkColor[deviceViewportSize().width() * deviceViewportSize().height()]);
    renderer()->getFramebufferPixels(pixels.get(), outerRect);

// FIXME: This fails on Android. Endianness maybe?
// Yellow: expects 0xFFFFFF00, was 0xFF00FFFF on android.
// Cyan:   expects 0xFF00FFFF, was 0xFFFFFF00 on android.
// http://crbug.com/154528
#ifndef OS_ANDROID
    EXPECT_EQ(SK_ColorYELLOW, pixels[0]);
    EXPECT_EQ(SK_ColorYELLOW, pixels[outerPixels - 1]);
    EXPECT_EQ(SK_ColorCYAN, pixels[outerSize.width() + 1]);
    EXPECT_EQ(SK_ColorCYAN, pixels[outerPixels - outerSize.width() - 2]);
#endif
}

TEST_F(SoftwareRendererTest, tileQuad)
{
    gfx::Size outerSize(100, 100);
    int outerPixels = outerSize.width() * outerSize.height();
    gfx::Size innerSize(98, 98);
    int innerPixels = innerSize.width() * innerSize.height();
    gfx::Rect outerRect(gfx::Point(), outerSize);
    gfx::Rect innerRect(gfx::Point(1, 1), innerSize);
    setViewportSize(outerSize);
    initializeRenderer();

    ResourceProvider::ResourceId resourceYellow = resourceProvider()->createResource(1, outerSize, GL_RGBA, ResourceProvider::TextureUsageAny);
    ResourceProvider::ResourceId resourceCyan = resourceProvider()->createResource(1, innerSize, GL_RGBA, ResourceProvider::TextureUsageAny);

    SkColor yellow = SK_ColorYELLOW;
    SkColor cyan = SK_ColorCYAN;
    scoped_array<SkColor> yellowPixels(new SkColor[outerPixels]);
    scoped_array<SkColor> cyanPixels(new SkColor[innerPixels]);
    for (int i = 0; i < outerPixels; i++)
      yellowPixels[i] = yellow;
    for (int i = 0; i < innerPixels; i++)
      cyanPixels[i] = cyan;

    resourceProvider()->upload(resourceYellow, reinterpret_cast<uint8_t*>(yellowPixels.get()), gfx::Rect(gfx::Point(), outerSize), gfx::Rect(gfx::Point(), outerSize), gfx::Vector2d());
    resourceProvider()->upload(resourceCyan, reinterpret_cast<uint8_t*>(cyanPixels.get()), gfx::Rect(gfx::Point(), innerSize), gfx::Rect(gfx::Point(), innerSize), gfx::Vector2d());

    gfx::Rect rect = gfx::Rect(gfx::Point(), deviceViewportSize());

    scoped_ptr<SharedQuadState> sharedQuadState = SharedQuadState::create(WebTransformationMatrix(), outerRect, outerRect, 1.0, true);
    RenderPass::Id rootRenderPassId = RenderPass::Id(1, 1);
    scoped_ptr<TestRenderPass> rootRenderPass = TestRenderPass::create(rootRenderPassId, gfx::Rect(gfx::Point(), deviceViewportSize()), WebTransformationMatrix());
    scoped_ptr<DrawQuad> outerQuad = TileDrawQuad::create(sharedQuadState.get(), outerRect, outerRect, resourceYellow, gfx::Vector2d(), outerSize, 0, false, false, false, false, false).PassAs<DrawQuad>();
    scoped_ptr<DrawQuad> innerQuad = TileDrawQuad::create(sharedQuadState.get(), innerRect, innerRect, resourceCyan, gfx::Vector2d(), innerSize, 0, false, false, false, false, false).PassAs<DrawQuad>();
    rootRenderPass->appendQuad(innerQuad.Pass());
    rootRenderPass->appendQuad(outerQuad.Pass());

    RenderPassList list;
    RenderPassIdHashMap hashmap;
    list.push_back(rootRenderPass.get());
    hashmap.add(rootRenderPassId, rootRenderPass.PassAs<RenderPass>());
    renderer()->drawFrame(list, hashmap);

    scoped_array<SkColor> pixels(new SkColor[deviceViewportSize().width() * deviceViewportSize().height()]);
    renderer()->getFramebufferPixels(pixels.get(), outerRect);

    EXPECT_EQ(SK_ColorYELLOW, pixels[0]);
    EXPECT_EQ(SK_ColorYELLOW, pixels[outerPixels - 1]);
    EXPECT_EQ(SK_ColorCYAN, pixels[outerSize.width() + 1]);
    EXPECT_EQ(SK_ColorCYAN, pixels[outerPixels - outerSize.width() - 2]);
}

} // namespace
