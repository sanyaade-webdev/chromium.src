// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/screen_ash.h"

#include "ash/display/display_controller.h"
#include "ash/display/multi_display_manager.h"
#include "ash/root_window_controller.h"
#include "ash/shell.h"
#include "ash/wm/property_util.h"
#include "ash/wm/coordinate_conversion.h"
#include "ash/wm/shelf_layout_manager.h"
#include "base/logging.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/env.h"
#include "ui/aura/display_manager.h"
#include "ui/aura/root_window.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

namespace ash {

namespace {
internal::MultiDisplayManager* GetDisplayManager() {
  return static_cast<internal::MultiDisplayManager*>(
      aura::Env::GetInstance()->display_manager());
}
}  // namespace

ScreenAsh::ScreenAsh() {
}

ScreenAsh::~ScreenAsh() {
}

// static
gfx::Display ScreenAsh::FindDisplayContainingPoint(const gfx::Point& point) {
  return GetDisplayManager()->FindDisplayContainingPoint(point);
}

// static
gfx::Rect ScreenAsh::GetMaximizedWindowBoundsInParent(aura::Window* window) {
  if (GetRootWindowController(window->GetRootWindow())->launcher())
    return GetDisplayWorkAreaBoundsInParent(window);
  else
    return GetDisplayBoundsInParent(window);
}

// static
gfx::Rect ScreenAsh::GetDisplayBoundsInParent(aura::Window* window) {
  return ConvertRectFromScreen(
      window->parent(),
      Shell::GetScreen()->GetDisplayNearestWindow(window).bounds());
}

// static
gfx::Rect ScreenAsh::GetDisplayWorkAreaBoundsInParent(aura::Window* window) {
  return ConvertRectFromScreen(
      window->parent(),
      Shell::GetScreen()->GetDisplayNearestWindow(window).work_area());
}

// static
gfx::Rect ScreenAsh::ConvertRectToScreen(aura::Window* window,
                                         const gfx::Rect& rect) {
  gfx::Point point = rect.origin();
  aura::client::GetScreenPositionClient(window->GetRootWindow())->
      ConvertPointToScreen(window, &point);
  return gfx::Rect(point, rect.size());
}

// static
gfx::Rect ScreenAsh::ConvertRectFromScreen(aura::Window* window,
                                           const gfx::Rect& rect) {
  gfx::Point point = rect.origin();
  aura::client::GetScreenPositionClient(window->GetRootWindow())->
      ConvertPointFromScreen(window, &point);
  return gfx::Rect(point, rect.size());
}

// static
const gfx::Display& ScreenAsh::GetSecondaryDisplay() {
  return *(Shell::GetInstance()->display_controller()->GetSecondaryDisplay());
}

// static
const gfx::Display& ScreenAsh::GetDisplayForId(int64 display_id) {
  return GetDisplayManager()->GetDisplayForId(display_id);
}

bool ScreenAsh::IsDIPEnabled() {
  return true;
}

gfx::Point ScreenAsh::GetCursorScreenPoint() {
  return aura::Env::GetInstance()->last_mouse_location();
}

gfx::NativeWindow ScreenAsh::GetWindowAtCursorScreenPoint() {
  const gfx::Point point = Shell::GetScreen()->GetCursorScreenPoint();
  return wm::GetRootWindowAt(point)->GetTopWindowContainingPoint(point);
}

int ScreenAsh::GetNumDisplays() {
  return GetDisplayManager()->GetNumDisplays();
}

gfx::Display ScreenAsh::GetDisplayNearestWindow(gfx::NativeView window) const {
  return GetDisplayManager()->GetDisplayNearestWindow(window);
}

gfx::Display ScreenAsh::GetDisplayNearestPoint(const gfx::Point& point) const {
  return GetDisplayManager()->GetDisplayNearestPoint(point);
}

gfx::Display ScreenAsh::GetDisplayMatching(const gfx::Rect& match_rect) const {
  return GetDisplayManager()->GetDisplayMatching(match_rect);
}

gfx::Display ScreenAsh::GetPrimaryDisplay() const {
  return DisplayController::GetPrimaryDisplay();
}

}  // namespace ash
