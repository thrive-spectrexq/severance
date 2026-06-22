#pragma once

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QString>

namespace severance::gui::theme {

// ============================================================================
// Severance Design Tokens
// ============================================================================
// Color palette inspired by modern IDE themes — deep, high-contrast, with
// teal accent. Designed for all-day use without eye strain.

struct Colors {
  // Backgrounds
  static constexpr const char* BgPrimary    = "#0B0E14";   // Main background
  static constexpr const char* BgSecondary  = "#151A23";   // Sidebar, cards, panels
  static constexpr const char* BgTertiary   = "#1C2331";   // Elevated surfaces / input bg
  static constexpr const char* BgHover      = "#242D3D";   // Hover state
  static constexpr const char* BgSelected   = "#1A2E44";   // Selected/active

  // Borders
  static constexpr const char* Border       = "#2A3441";   // Default border
  static constexpr const char* BorderLight  = "#1C2331";   // Subtle dividers

  // Text
  static constexpr const char* TextPrimary  = "#F1F5F9";   // Main text
  static constexpr const char* TextSecondary= "#94A3B8";   // Muted/secondary
  static constexpr const char* TextTertiary = "#64748B";   // Disabled/hint
  static constexpr const char* TextLink     = "#00E5FF";   // Links / primary accent

  // Accent — Cyan/Teal
  static constexpr const char* Accent       = "#00E5FF";   // Primary accent
  static constexpr const char* AccentHover  = "#33EFFF";   // Accent hover
  static constexpr const char* AccentMuted  = "#003D4D";   // Accent background

  // Semantic
  static constexpr const char* Success      = "#00E676";   // Green — good/200 OK
  static constexpr const char* Warning      = "#FFAB00";   // Orange — caution
  static constexpr const char* Error        = "#FF1744";   // Red — critical
  static constexpr const char* Info         = "#2979FF";   // Blue — info/300

  // Charts
  static constexpr const char* ChartPurple  = "#D500F9";
  static constexpr const char* ChartBlue    = "#2979FF";
  static constexpr const char* ChartGreen   = "#00E676";
  static constexpr const char* ChartYellow  = "#FFEA00";
  static constexpr const char* ChartOrange  = "#FF9100";
};

struct Dimensions {
  static constexpr int SidebarWidth        = 60;
  static constexpr int SidebarExpandedWidth= 260;
  static constexpr int StatusBarHeight     = 28;
  static constexpr int CardRadius          = 8;
  static constexpr int CardPadding         = 16;
  static constexpr int CardSpacing         = 16;
  static constexpr int SearchBarHeight     = 36;
  static constexpr int SearchOverlayWidth  = 600;
  static constexpr int ScrollbarWidth      = 8;
};

// Apply the complete dark theme to the application
void ApplyDarkTheme(QApplication* app);

// Get the main application stylesheet
QString GetDarkStylesheet();

// Get the sidebar-specific stylesheet
QString GetSidebarStylesheet();

// Get card/panel stylesheet
QString GetCardStylesheet();

} // namespace severance::gui::theme
