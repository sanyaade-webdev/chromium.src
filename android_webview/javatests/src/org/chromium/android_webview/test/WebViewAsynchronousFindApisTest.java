// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;

/**
 * Tests the asynchronous find-in-page APIs in WebView.
 */
public class WebViewAsynchronousFindApisTest extends WebViewFindApisTestBase {

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindAllFinds() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindAllDouble() throws Throwable {
        findAllAsyncOnUiThread("wood");
        assertEquals(4, findAllAsyncOnUiThread("chuck"));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindAllDoubleNext() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(2, findNextOnUiThread(true));
    }

    @SmallTest
    @Feature({"AndroidWebView", "FindInPage"})
    public void testFindAllDoesNotFind() throws Throwable {
        assertEquals(0, findAllAsyncOnUiThread("foo"));
    }

    @SmallTest
    @Feature({"AndroidWebView", "FindInPage"})
    public void testFindAllEmptyPage() throws Throwable {
        assertEquals(0, findAllAsyncOnUiThread("foo"));
    }

    @SmallTest
    @Feature({"AndroidWebView", "FindInPage"})
    public void testFindAllEmptyString() throws Throwable {
        assertEquals(0, findAllAsyncOnUiThread(""));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindNextForward() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));

        for (int i = 2; i <= 4; i++) {
            assertEquals(i - 1, findNextOnUiThread(true));
        }
        assertEquals(0, findNextOnUiThread(true));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindNextBackward() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));

        for (int i = 4; i >= 1; i--) {
            assertEquals(i - 1, findNextOnUiThread(false));
        }
        assertEquals(3, findNextOnUiThread(false));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindNextBig() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));

        assertEquals(1, findNextOnUiThread(true));
        assertEquals(0, findNextOnUiThread(false));
        assertEquals(3, findNextOnUiThread(false));
        for (int i = 1; i <= 4; i++) {
            assertEquals(i - 1, findNextOnUiThread(true));
        }
        assertEquals(0, findNextOnUiThread(true));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindAllEmptyNext() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
        assertEquals(0, findAllAsyncOnUiThread(""));
        assertEquals(0, findNextOnUiThread(true));
        assertEquals(0, findAllAsyncOnUiThread(""));
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testClearMatches() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        clearMatchesOnUiThread();
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testClearFindNext() throws Throwable {
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        clearMatchesOnUiThread();
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(2, findNextOnUiThread(true));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindEmptyNext() throws Throwable {
        assertEquals(0, findAllAsyncOnUiThread(""));
        assertEquals(0, findNextOnUiThread(true));
        assertEquals(4, findAllAsyncOnUiThread("wood"));
    }

    /*
     * @SmallTest
     * @Feature({"AndroidWebView", "FindInPage"})
     * BUG=158284
     */
    @DisabledTest
    public void testFindNextFirst() throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                contents().findNext(true);
            }
        });
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
        assertEquals(0, findNextOnUiThread(false));
        assertEquals(3, findNextOnUiThread(false));
    }
}
