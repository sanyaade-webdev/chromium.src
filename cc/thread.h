// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCThread_h
#define CCThread_h

#include "base/callback.h"
#include "base/basictypes.h"

namespace cc {

// Thread provides basic infrastructure for messaging with the compositor in a
// platform-neutral way.
class Thread {
public:
    virtual ~Thread() { }

    // Executes the callback on context's thread asynchronously.
    virtual void postTask(base::Closure cb) = 0;

    // Executes the task after the specified delay.
    virtual void postDelayedTask(base::Closure cb, long long delayMs) = 0;

    virtual bool belongsToCurrentThread() const = 0;
};

}

#endif
