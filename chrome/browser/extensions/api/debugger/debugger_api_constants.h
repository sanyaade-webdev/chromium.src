// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Constants used for the Cookies API.

#ifndef CHROME_BROWSER_EXTENSIONS_API_DEBUGGER_DEBUGGER_API_CONSTANTS_H_
#define CHROME_BROWSER_EXTENSIONS_API_DEBUGGER_DEBUGGER_API_CONSTANTS_H_

namespace debugger_api_constants {

// Events.
extern const char kOnEvent[];
extern const char kOnDetach[];

// Errors.
extern const char kAlreadyAttachedError[];
extern const char kAttachToWebUIError[];
extern const char kDebuggerExecuteFailedError[];
extern const char kNoTabError[];
extern const char kNotAttachedError[];
extern const char kPermissionError[];
extern const char kProtocolVersionNotSupportedError[];

}  // namespace debugger_api_constants

#endif  // CHROME_BROWSER_EXTENSIONS_API_DEBUGGER_DEBUGGER_API_CONSTANTS_H_
