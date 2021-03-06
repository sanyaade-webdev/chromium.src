// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/automation/automation_util.h"
#include "chrome/browser/extensions/platform_app_browsertest_util.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/ui_test_utils.h"
#include "chrome/test/base/test_launcher_utils.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/test/browser_test_utils.h"
#include "ui/compositor/compositor_setup.h"
#include "ui/gl/gl_switches.h"

class WebViewTest : public extensions::PlatformAppBrowserTest {
 protected:
  virtual void SetUpCommandLine(CommandLine* command_line) {
    extensions::PlatformAppBrowserTest::SetUpCommandLine(command_line);
    // Enable <webview> for running test.
    command_line->AppendSwitch(switches::kEnableWebView);
#if !defined(OS_MACOSX)
    CHECK(test_launcher_utils::OverrideGLImplementation(
          command_line, gfx::kGLImplementationOSMesaName)) <<
          "kUseGL must not be set by test framework code!";
#endif
    ui::DisableTestCompositor();
  }
};

IN_PROC_BROWSER_TEST_F(WebViewTest, Shim) {
  ASSERT_TRUE(RunPlatformAppTest("platform_apps/web_view")) << message_;
}

IN_PROC_BROWSER_TEST_F(WebViewTest, ShimSrcAttribute) {
  ASSERT_TRUE(RunPlatformAppTest("platform_apps/web_view_src_attribute"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(WebViewTest, Isolation) {
  ASSERT_TRUE(StartTestServer());
  const std::wstring kExpire =
      L"var expire = new Date(Date.now() + 24 * 60 * 60 * 1000);";
  std::wstring cookie_script1(kExpire);
  cookie_script1.append(
      L"document.cookie = 'guest1=true; path=/; expires=' + expire + ';';");
  std::wstring cookie_script2(kExpire);
  cookie_script2.append(
      L"document.cookie = 'guest2=true; path=/; expires=' + expire + ';';");

  GURL::Replacements replace_host;
  std::string host_str("localhost");  // Must stay in scope with replace_host.
  replace_host.SetHostStr(host_str);

  GURL set_cookie_url = test_server()->GetURL(
      "files/extensions/platform_apps/isolation/set_cookie.html");
  set_cookie_url = set_cookie_url.ReplaceComponents(replace_host);
  GURL tag_url1 = test_server()->GetURL(
      "files/extensions/platform_apps/web_view_isolation/cookie.html");
  tag_url1 = tag_url1.ReplaceComponents(replace_host);
  GURL tag_url2 = test_server()->GetURL(
      "files/extensions/platform_apps/web_view_isolation/cookie2.html");
  tag_url2 = tag_url2.ReplaceComponents(replace_host);

  // Load a (non-app) page under the "localhost" origin that sets a cookie.
  ui_test_utils::NavigateToURLWithDisposition(
      browser(), set_cookie_url,
      CURRENT_TAB, ui_test_utils::BROWSER_TEST_WAIT_FOR_NAVIGATION);
  // Make sure the cookie is set.
  int cookie_size;
  std::string cookie_value;
  automation_util::GetCookies(set_cookie_url,
                              chrome::GetWebContentsAt(browser(), 0),
                              &cookie_size, &cookie_value);
  EXPECT_EQ("testCookie=1", cookie_value);

  ui_test_utils::UrlLoadObserver observer1(
      tag_url1, content::NotificationService::AllSources());
  ui_test_utils::UrlLoadObserver observer2(
      tag_url2, content::NotificationService::AllSources());
  LoadAndLaunchPlatformApp("web_view_isolation");
  observer1.Wait();
  observer2.Wait();

  content::Source<content::NavigationController> source1 = observer1.source();
  EXPECT_TRUE(source1->GetWebContents()->GetRenderProcessHost()->IsGuest());
  content::Source<content::NavigationController> source2 = observer2.source();
  EXPECT_TRUE(source2->GetWebContents()->GetRenderProcessHost()->IsGuest());
  EXPECT_NE(source1->GetWebContents()->GetRenderProcessHost()->GetID(),
            source2->GetWebContents()->GetRenderProcessHost()->GetID());

  EXPECT_TRUE(content::ExecuteJavaScript(
      source1->GetWebContents()->GetRenderViewHost(), std::wstring(),
      cookie_script1));
  EXPECT_TRUE(content::ExecuteJavaScript(
      source2->GetWebContents()->GetRenderViewHost(), std::wstring(),
      cookie_script2));

  // Test the regular browser context to ensure we still have only one cookie.
  automation_util::GetCookies(GURL("http://localhost"),
                              chrome::GetWebContentsAt(browser(), 0),
                              &cookie_size, &cookie_value);
  EXPECT_EQ("testCookie=1", cookie_value);

  // The default behavior is to combine webview tags with no explicit partition
  // declaration into the same in-memory partition. Test the webview tags to
  // ensure we have properly set the cookies and we have both cookies in both
  // tags.
  automation_util::GetCookies(GURL("http://localhost"),
                              source1->GetWebContents(),
                              &cookie_size, &cookie_value);
  EXPECT_EQ("guest1=true; guest2=true", cookie_value);

  automation_util::GetCookies(GURL("http://localhost"),
                              source2->GetWebContents(),
                              &cookie_size, &cookie_value);
  EXPECT_EQ("guest1=true; guest2=true", cookie_value);
}
