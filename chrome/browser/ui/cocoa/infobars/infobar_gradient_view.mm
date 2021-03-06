// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/cocoa/infobars/infobar_gradient_view.h"

#include "base/memory/scoped_nsobject.h"
#include "chrome/browser/infobars/infobar.h"
#import "chrome/browser/themes/theme_service.h"
#import "chrome/browser/ui/cocoa/browser_window_controller.h"
#import "chrome/browser/ui/cocoa/infobars/infobar_container_controller.h"
#import "chrome/browser/ui/cocoa/location_bar/location_bar_view_mac.h"
#import "chrome/browser/ui/cocoa/themed_window.h"
#include "skia/ext/skia_utils_mac.h"

namespace {

const CGFloat kTipWidth = 23;

}  // namespace

@implementation InfoBarGradientView

@synthesize tipApex = tipApex_;

- (void)setInfobarType:(InfoBarDelegate::Type)infobarType {
  SkColor topColor = GetInfoBarTopColor(infobarType);
  SkColor bottomColor = GetInfoBarBottomColor(infobarType);
  scoped_nsobject<NSGradient> gradient([[NSGradient alloc]
      initWithStartingColor:gfx::SkColorToCalibratedNSColor(topColor)
                endingColor:gfx::SkColorToCalibratedNSColor(bottomColor)]);
  [self setGradient:gradient];
}

- (NSColor*)strokeColor {
  ui::ThemeProvider* themeProvider = [[self window] themeProvider];
  if (!themeProvider)
    return [NSColor blackColor];

  BOOL active = [[self window] isMainWindow];
  return themeProvider->GetNSColor(
      active ? ThemeService::COLOR_TOOLBAR_STROKE :
               ThemeService::COLOR_TOOLBAR_STROKE_INACTIVE,
      true);
}

- (void)drawRect:(NSRect)rect {
  NSRect bounds = [self bounds];
  bounds.size.height = infobars::kBaseHeight;

  const CGFloat kHalfWidth = kTipWidth / 2.0;
  NSPoint localApex = [self convertPoint:self.tipApex fromView:nil];
  CGFloat tipXOffset = localApex.x - kHalfWidth;

  // Around the bounds of the infobar, continue drawing the path into which the
  // gradient will be drawn.
  NSBezierPath* infoBarPath = [NSBezierPath bezierPath];
  [infoBarPath moveToPoint:NSMakePoint(NSMinX(bounds), NSMaxY(bounds))];

  // Draw the tip.
  [infoBarPath lineToPoint:NSMakePoint(tipXOffset, NSMaxY(bounds))];
  [infoBarPath relativeLineToPoint:NSMakePoint(kHalfWidth,
                                               infobars::kTipHeight)];
  [infoBarPath relativeLineToPoint:NSMakePoint(kHalfWidth,
                                               -infobars::kTipHeight)];
  [infoBarPath lineToPoint:NSMakePoint(NSMaxX(bounds), NSMaxY(bounds))];

  // Save off the top path of the infobar.
  scoped_nsobject<NSBezierPath> topPath([infoBarPath copy]);

  [infoBarPath lineToPoint:NSMakePoint(NSMaxX(bounds), NSMinY(bounds))];
  [infoBarPath lineToPoint:NSMakePoint(NSMinX(bounds), NSMinY(bounds))];
  [infoBarPath closePath];

  // Draw the gradient.
  [[self gradient] drawInBezierPath:infoBarPath angle:270];

  NSColor* strokeColor = [self strokeColor];
  if (strokeColor) {
    [strokeColor set];

    // Stroke the bottom of the infobar.
    NSRect borderRect, contentRect;
    NSDivideRect(bounds, &borderRect, &contentRect, 1, NSMinYEdge);
    NSRectFillUsingOperation(borderRect, NSCompositeSourceOver);

    // Re-stroke the top because the tip will have no stroke. This will draw
    // over the divider drawn by the bottom of the tabstrip.
    [topPath stroke];
  }

  // Add an inner stroke.
  const CGFloat kHighlightTipHeight = infobars::kTipHeight - 1;
  NSBezierPath* highlightPath = [NSBezierPath bezierPath];
  [highlightPath moveToPoint:NSMakePoint(NSMinX(bounds), NSMaxY(bounds) - 1)];
  [highlightPath relativeLineToPoint:NSMakePoint(tipXOffset + 1, 0)];
  [highlightPath relativeLineToPoint:NSMakePoint(kHalfWidth - 1,
                                                 kHighlightTipHeight)];
  [highlightPath relativeLineToPoint:NSMakePoint(kHalfWidth - 1,
                                                 -kHighlightTipHeight)];
  [highlightPath lineToPoint:NSMakePoint(NSMaxX(bounds), NSMaxY(bounds) - 1)];

  [[NSColor colorWithDeviceWhite:1.0 alpha:1.0] setStroke];
  [highlightPath stroke];
}

- (BOOL)mouseDownCanMoveWindow {
  return NO;
}

// This view is intentionally not opaque because it overlaps with the findbar.

- (BOOL)accessibilityIsIgnored {
  return NO;
}

- (id)accessibilityAttributeValue:(NSString*)attribute {
  if ([attribute isEqual:NSAccessibilityRoleAttribute])
    return NSAccessibilityGroupRole;

  return [super accessibilityAttributeValue:attribute];
}

@end
