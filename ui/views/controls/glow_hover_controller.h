// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_GLOW_HOVER_CONTROLLER_H_
#define UI_VIEWS_CONTROLS_GLOW_HOVER_CONTROLLER_H_

#include "ui/base/animation/animation_delegate.h"
#include "ui/base/animation/slide_animation.h"
#include "ui/views/views_export.h"

namespace gfx {
class Canvas;
class ImageSkia;
class Point;
}

namespace views {

class View;

// GlowHoverController is responsible for drawing a hover effect as is used by
// the tabstrip. Typical usage:
//   OnMouseEntered() -> invoke Show().
//   OnMouseMoved()   -> invoke SetLocation().
//   OnMouseExited()  -> invoke Hide().
//   OnPaint()        -> if ShouldDraw() returns true invoke Draw().
// Internally GlowHoverController uses an animation to animate the glow and
// invokes SchedulePaint() back on the View as necessary.
class VIEWS_EXPORT GlowHoverController : public ui::AnimationDelegate {
 public:
  explicit GlowHoverController(views::View* view);
  virtual ~GlowHoverController();

  // Sets the AnimationContainer used by the animation.
  void SetAnimationContainer(ui::AnimationContainer* container);

  // Sets the location of the hover, relative to the View passed to the
  // constructor.
  void SetLocation(const gfx::Point& location);

  // Initiates showing the hover.
  void Show();

  // Hides the hover.
  void Hide();

  // Hides the hover immediately.
  void HideImmediately();

  // Returns the value of the animation.
  double GetAnimationValue() const;

  // Returns true if there is something to be drawn. Use this instead of
  // invoking Draw() if creating |mask_image| is expensive.
  bool ShouldDraw() const;

  // If the hover is currently visible it is drawn to the supplied canvas.
  // |mask_image| is used to determine what parts of the canvas to draw on.
  void Draw(gfx::Canvas* canvas, const gfx::ImageSkia& mask_image) const;

  // ui::AnimationDelegate overrides:
  virtual void AnimationEnded(const ui::Animation* animation) OVERRIDE;
  virtual void AnimationProgressed(const ui::Animation* animation) OVERRIDE;

 private:
  // View we're drawing to.
  views::View* view_;

  // Opacity of the glow ramps up over time.
  ui::SlideAnimation animation_;

  // Location of the glow, relative to view.
  gfx::Point location_;

  DISALLOW_COPY_AND_ASSIGN(GlowHoverController);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_GLOW_HOVER_CONTROLLER_H_
