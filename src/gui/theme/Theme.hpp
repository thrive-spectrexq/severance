#pragma once

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QString>

namespace severance::gui::theme {

// ============================================================================
// LUMON INDUSTRIES — Design Tokens
// ============================================================================
// Color palette inspired by Lumon Industries (Apple TV+ "Severance").
// Clinical, institutional teal-green on near-black backgrounds.
// Sterile precision. Corporate control. The work is mysterious and important.

struct Colors {
  // Backgrounds — cold, liminal corridor tones
  static constexpr const char* BgPrimary    = "#0A0E14";   // Near-black, cold blue undertone
  static constexpr const char* BgSecondary  = "#0F1A1F";   // Very dark teal-green
  static constexpr const char* BgTertiary   = "#132A2E";   // Dark institutional green (panels)
  static constexpr const char* BgHover      = "#1A2332";   // Dark slate, blue-green cast
  static constexpr const char* BgSelected   = "#1A2332";   // Selected/active surface

  // Borders — dark blue-green separators
  static constexpr const char* Border       = "#1C2E38";   // Default border
  static constexpr const char* BorderLight  = "#243447";   // Subtle dividers

  // Text — cool institutional whites and grays
  static constexpr const char* TextPrimary  = "#E8ECEF";   // Cool white
  static constexpr const char* TextSecondary= "#8B9DAF";   // Muted cool gray
  static constexpr const char* TextTertiary = "#3D4F5F";   // Disabled/hint (faint gray)
  static constexpr const char* TextLink     = "#7FDBCA";   // Pale fluorescent teal

  // Accent — Lumon Teal
  static constexpr const char* Accent       = "#1A7A5C";   // Primary Lumon teal
  static constexpr const char* AccentHover  = "#2ECC71";   // Bright green for active states
  static constexpr const char* AccentMuted  = "#0F1A1F";   // Accent background (dark teal)
  static constexpr const char* AccentGlow   = "#7FDBCA";   // Pale fluorescent teal glow

  // Institutional blue
  static constexpr const char* Blue         = "#1B4F72";   // Cold institutional blue
  static constexpr const char* BlueBright   = "#2980B9";   // Brighter institutional blue

  // Semantic — clinical indicators
  static constexpr const char* Success      = "#2ECC71";   // Lumon green — operational
  static constexpr const char* Warning      = "#D4A017";   // Institutional amber
  static constexpr const char* Error        = "#C0392B";   // Clinical red
  static constexpr const char* Info         = "#1B4F72";   // Institutional blue

  // Charts — Lumon data visualization palette
  static constexpr const char* ChartTeal    = "#1A7A5C";
  static constexpr const char* ChartGlow    = "#7FDBCA";
  static constexpr const char* ChartGreen   = "#2ECC71";
  static constexpr const char* ChartAmber   = "#D4A017";
  static constexpr const char* ChartBlue    = "#2980B9";
  static constexpr const char* ChartPurple  = "#8E44AD";
  static constexpr const char* ChartYellow  = "#F1C40F";
  static constexpr const char* ChartOrange  = "#E67E22";
};

struct Dimensions {
  static constexpr int SidebarWidth        = 60;
  static constexpr int SidebarExpandedWidth= 260;
  static constexpr int StatusBarHeight     = 28;
  static constexpr int CardRadius          = 4;     // Sharp, clinical precision
  static constexpr int CardPadding         = 16;
  static constexpr int CardSpacing         = 16;
  static constexpr int SearchBarHeight     = 36;
  static constexpr int SearchOverlayWidth  = 600;
  static constexpr int ScrollbarWidth      = 6;     // Thin, minimal scrollbars
};

// Applies a soft modern drop shadow to a widget
void ApplyDropShadow(QWidget* widget);

// Apply the complete dark theme to the application
void ApplyDarkTheme(QApplication* app);

// Get the main application stylesheet
QString GetDarkStylesheet();

// Get the sidebar-specific stylesheet
QString GetSidebarStylesheet();

// Get card/panel stylesheet
QString GetCardStylesheet();

} // namespace severance::gui::theme
