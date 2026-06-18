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
  static constexpr const char* BgPrimary    = "#0D1117";   // Main background
  static constexpr const char* BgSecondary  = "#161B22";   // Sidebar, cards
  static constexpr const char* BgTertiary   = "#1C2128";   // Elevated surfaces
  static constexpr const char* BgHover      = "#21262D";   // Hover state
  static constexpr const char* BgSelected   = "#1F3A5F";   // Selected/active

  // Borders
  static constexpr const char* Border       = "#30363D";   // Default border
  static constexpr const char* BorderLight  = "#21262D";   // Subtle dividers

  // Text
  static constexpr const char* TextPrimary  = "#E6EDF3";   // Main text
  static constexpr const char* TextSecondary= "#8B949E";   // Muted/secondary
  static constexpr const char* TextTertiary = "#6E7681";   // Disabled/hint
  static constexpr const char* TextLink     = "#58A6FF";   // Links

  // Accent — Teal
  static constexpr const char* Accent       = "#58A6FF";   // Primary accent
  static constexpr const char* AccentHover  = "#79C0FF";   // Accent hover
  static constexpr const char* AccentMuted  = "#1F3A5F";   // Accent background

  // Semantic
  static constexpr const char* Success      = "#3FB950";   // Green — good
  static constexpr const char* Warning      = "#D29922";   // Yellow — caution
  static constexpr const char* Error        = "#F85149";   // Red — critical
  static constexpr const char* Info         = "#58A6FF";   // Blue — info

  // Charts / heatmap
  static constexpr const char* ChartLine    = "#58A6FF";
  static constexpr const char* ChartFill    = "#1F3A5F";
  static constexpr const char* CpuHigh      = "#F85149";
  static constexpr const char* CpuMid       = "#D29922";
  static constexpr const char* CpuLow       = "#3FB950";
  static constexpr const char* MemUsed      = "#58A6FF";
  static constexpr const char* MemCached    = "#3FB950";
  static constexpr const char* MemFree      = "#21262D";
};

struct Dimensions {
  static constexpr int SidebarWidth        = 56;
  static constexpr int SidebarExpandedWidth= 200;
  static constexpr int StatusBarHeight     = 28;
  static constexpr int CardRadius          = 8;
  static constexpr int CardPadding         = 16;
  static constexpr int CardSpacing         = 12;
  static constexpr int SearchBarHeight     = 48;
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
