// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/options/network_config_view.h"

#include <algorithm>

#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/chromeos/login/base_login_display_host.h"
#include "chrome/browser/chromeos/options/vpn_config_view.h"
#include "chrome/browser/chromeos/options/wifi_config_view.h"
#include "chrome/browser/chromeos/options/wimax_config_view.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/host_desktop.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "grit/locale_settings.h"
#include "grit/theme_resources.h"
#include "ui/base/accessibility/accessible_view_state.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/layout_constants.h"
#include "ui/views/widget/widget.h"

namespace {

gfx::NativeWindow GetDialogParent() {
  if (chromeos::BaseLoginDisplayHost::default_host()) {
    return chromeos::BaseLoginDisplayHost::default_host()->GetNativeWindow();
  } else {
    Browser* browser = browser::FindTabbedBrowser(
        ProfileManager::GetDefaultProfileOrOffTheRecord(),
        true,
        chrome::HOST_DESKTOP_TYPE_ASH);
    if (browser)
      return browser->window()->GetNativeWindow();
  }
  return NULL;
}

// Avoid global static initializer.
chromeos::NetworkConfigView** GetActiveDialogPointer() {
  static chromeos::NetworkConfigView* active_dialog = NULL;
  return &active_dialog;
}

chromeos::NetworkConfigView* GetActiveDialog() {
  return *(GetActiveDialogPointer());
}

void SetActiveDialog(chromeos::NetworkConfigView* dialog) {
  *(GetActiveDialogPointer()) = dialog;
}

}  // namespace

namespace chromeos {

// static
const int ChildNetworkConfigView::kInputFieldMinWidth = 270;

NetworkConfigView::NetworkConfigView(Network* network)
    : delegate_(NULL),
      advanced_button_(NULL),
      advanced_button_container_(NULL) {
  DCHECK(GetActiveDialog() == NULL);
  SetActiveDialog(this);
  if (network->type() == TYPE_WIFI) {
    child_config_view_ =
        new WifiConfigView(this, static_cast<WifiNetwork*>(network));
  } else if (network->type() == TYPE_WIMAX) {
    child_config_view_ =
        new WimaxConfigView(this, static_cast<WimaxNetwork*>(network));
  } else if (network->type() == TYPE_VPN) {
    child_config_view_ =
        new VPNConfigView(this, static_cast<VirtualNetwork*>(network));
  } else {
    NOTREACHED();
    child_config_view_ = NULL;
  }
}

NetworkConfigView::NetworkConfigView(ConnectionType type)
    : delegate_(NULL),
      advanced_button_(NULL),
      advanced_button_container_(NULL) {
  DCHECK(GetActiveDialog() == NULL);
  SetActiveDialog(this);
  if (type == TYPE_WIFI) {
    child_config_view_ = new WifiConfigView(this, false /* show_8021x */);
    CreateAdvancedButton();
  } else if (type == TYPE_VPN) {
    child_config_view_ = new VPNConfigView(this);
  } else {
    NOTREACHED();
    child_config_view_ = NULL;
  }
}

NetworkConfigView::~NetworkConfigView() {
  DCHECK(GetActiveDialog() == this);
  SetActiveDialog(NULL);
}

// static
bool NetworkConfigView::Show(Network* network, gfx::NativeWindow parent) {
  if (GetActiveDialog() != NULL)
    return false;
  NetworkConfigView* view = new NetworkConfigView(network);
  if (parent == NULL)
    parent = GetDialogParent();
  views::Widget* window = views::Widget::CreateWindowWithParent(view, parent);
  window->SetAlwaysOnTop(true);
  window->Show();
  return true;
}

// static
bool NetworkConfigView::ShowForType(ConnectionType type,
                                    gfx::NativeWindow parent) {
  if (GetActiveDialog() != NULL)
    return false;
  NetworkConfigView* view = new NetworkConfigView(type);
  if (parent == NULL)
    parent = GetDialogParent();
  views::Widget* window = views::Widget::CreateWindowWithParent(view, parent);
  window->SetAlwaysOnTop(true);
  window->Show();
  return true;
}

gfx::NativeWindow NetworkConfigView::GetNativeWindow() const {
  return GetWidget()->GetNativeWindow();
}

string16 NetworkConfigView::GetDialogButtonLabel(
    ui::DialogButton button) const {
  if (button == ui::DIALOG_BUTTON_OK)
    return l10n_util::GetStringUTF16(IDS_OPTIONS_SETTINGS_CONNECT);
  return string16();
}

bool NetworkConfigView::IsDialogButtonEnabled(ui::DialogButton button) const {
  // Disable connect button if cannot login.
  if (button == ui::DIALOG_BUTTON_OK)
    return child_config_view_->CanLogin();
  return true;
}

bool NetworkConfigView::Cancel() {
  if (delegate_)
    delegate_->OnDialogCancelled();
  child_config_view_->Cancel();
  return true;
}

bool NetworkConfigView::Accept() {
  // Do not attempt login if it is guaranteed to fail, keep the dialog open.
  if (!child_config_view_->CanLogin())
    return false;
  bool result = child_config_view_->Login();
  if (result && delegate_)
    delegate_->OnDialogAccepted();
  return result;
}

views::View* NetworkConfigView::GetExtraView() {
  return advanced_button_container_;
}

views::View* NetworkConfigView::GetInitiallyFocusedView() {
  return child_config_view_->GetInitiallyFocusedView();
}

ui::ModalType NetworkConfigView::GetModalType() const {
  return ui::MODAL_TYPE_SYSTEM;
}

views::View* NetworkConfigView::GetContentsView() {
  return this;
}

void NetworkConfigView::GetAccessibleState(ui::AccessibleViewState* state) {
  state->name =
      l10n_util::GetStringUTF16(IDS_OPTIONS_SETTINGS_OTHER_WIFI_NETWORKS);
  state->role = ui::AccessibilityTypes::ROLE_DIALOG;
}

void NetworkConfigView::ButtonPressed(views::Button* sender,
                                      const ui::Event& event) {
  if (advanced_button_ && sender == advanced_button_) {
    advanced_button_->SetVisible(false);
    ShowAdvancedView();
  }
}

void NetworkConfigView::ShowAdvancedView() {
  // Clear out the old widgets and build new ones.
  RemoveChildView(child_config_view_);
  delete child_config_view_;
  // For now, there is only an advanced view for Wi-Fi 802.1X.
  child_config_view_ = new WifiConfigView(this, true /* show_8021x */);
  AddChildView(child_config_view_);
  // Resize the window to be able to hold the new widgets.
  gfx::Size size = views::Widget::GetLocalizedContentsSize(
      IDS_JOIN_WIFI_NETWORK_DIALOG_ADVANCED_WIDTH_CHARS,
      IDS_JOIN_WIFI_NETWORK_DIALOG_ADVANCED_MINIMUM_HEIGHT_LINES);
  // Get the new bounds with desired size at the same center point.
  gfx::Rect bounds = GetWidget()->GetWindowBoundsInScreen();
  int horiz_padding = bounds.width() - size.width();
  int vert_padding = bounds.height() - size.height();
  bounds.Inset(horiz_padding / 2, vert_padding / 2,
               horiz_padding / 2, vert_padding / 2);
  GetWidget()->SetBoundsConstrained(bounds);
  Layout();
  child_config_view_->InitFocus();
}

void NetworkConfigView::Layout() {
  child_config_view_->SetBounds(0, 0, width(), height());
}

gfx::Size NetworkConfigView::GetPreferredSize() {
  gfx::Size result(views::Widget::GetLocalizedContentsSize(
      IDS_JOIN_WIFI_NETWORK_DIALOG_WIDTH_CHARS,
      IDS_JOIN_WIFI_NETWORK_DIALOG_MINIMUM_HEIGHT_LINES));
  gfx::Size size = child_config_view_->GetPreferredSize();
  result.set_height(size.height());
  if (size.width() > result.width())
    result.set_width(size.width());
  return result;
}

void NetworkConfigView::ViewHierarchyChanged(
    bool is_add, views::View* parent, views::View* child) {
  // Can't init before we're inserted into a Container, because we require
  // a HWND to parent native child controls to.
  if (is_add && child == this) {
    AddChildView(child_config_view_);
  }
}

void NetworkConfigView::CreateAdvancedButton() {
  advanced_button_ = new views::NativeTextButton(this,
      l10n_util::GetStringUTF16(
          IDS_OPTIONS_SETTINGS_INTERNET_OPTIONS_ADVANCED_BUTTON));

  // Wrap the advanced button in a grid layout in order to left-align it.
  advanced_button_container_ = new views::View();
  views::GridLayout* layout = new views::GridLayout(advanced_button_container_);
  advanced_button_container_->SetLayoutManager(layout);

  int column_set_id = 0;
  views::ColumnSet* column_set = layout->AddColumnSet(column_set_id);
  column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING,
                        0, views::GridLayout::USE_PREF, 0, 0);
  layout->StartRow(0, column_set_id);
  layout->AddView(advanced_button_);
}

ControlledSettingIndicatorView::ControlledSettingIndicatorView()
    : managed_(false),
      image_view_(NULL) {
  Init();
}

ControlledSettingIndicatorView::ControlledSettingIndicatorView(
    const NetworkPropertyUIData& ui_data)
    : managed_(false),
      image_view_(NULL) {
  Init();
  Update(ui_data);
}

ControlledSettingIndicatorView::~ControlledSettingIndicatorView() {}

void ControlledSettingIndicatorView::Update(
    const NetworkPropertyUIData& ui_data) {
  if (managed_ == ui_data.managed())
    return;

  managed_ = ui_data.managed();
  PreferredSizeChanged();
}

gfx::Size ControlledSettingIndicatorView::GetPreferredSize() {
  return (managed_ && visible()) ? image_view_->GetPreferredSize()
                                 : gfx::Size();
}

void ControlledSettingIndicatorView::Layout() {
  image_view_->SetBounds(0, 0, width(), height());
}

void ControlledSettingIndicatorView::OnMouseEntered(
    const ui::MouseEvent& event) {
  image_view_->SetImage(color_image_);
}

void ControlledSettingIndicatorView::OnMouseExited(
    const ui::MouseEvent& event) {
  image_view_->SetImage(gray_image_);
}

void ControlledSettingIndicatorView::Init() {
  color_image_ = ResourceBundle::GetSharedInstance().GetImageNamed(
      IDR_CONTROLLED_SETTING_MANDATORY).ToImageSkia();
  gray_image_ = ResourceBundle::GetSharedInstance().GetImageNamed(
      IDR_CONTROLLED_SETTING_MANDATORY_GRAY).ToImageSkia();
  image_view_ = new views::ImageView();
  // Disable |image_view_| so mouse events propagate to the parent.
  image_view_->SetEnabled(false);
  image_view_->SetImage(gray_image_);
  image_view_->SetTooltipText(
      l10n_util::GetStringUTF16(IDS_OPTIONS_CONTROLLED_SETTING_POLICY));
  AddChildView(image_view_);
}

}  // namespace chromeos
