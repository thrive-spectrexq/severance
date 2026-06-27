#include "Theme.hpp"
#include <QFont>
#include <QFontDatabase>
#include <QPalette>
#include <QGraphicsDropShadowEffect>

namespace severance::gui::theme {

void ApplyDropShadow(QWidget* widget) {
  if (!widget) return;
  auto* shadow = new QGraphicsDropShadowEffect(widget);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 80));
  shadow->setOffset(0, 4);
  widget->setGraphicsEffect(shadow);
}

QString GetDarkStylesheet() {
  return QStringLiteral(R"(
    /* === Global === */
    * {
      margin: 0;
      padding: 0;
      font-family: "Segoe UI Variable", "Inter", "Segoe UI", sans-serif;
    }

    QMainWindow {
      background-color: #09090b;
      color: #F1F5F9;
    }

    QWidget {
      background-color: transparent;
      color: #F1F5F9;
      font-size: 13px;
    }

    /* === Scrollbars === */
    QScrollBar:vertical {
      background: transparent;
      width: 10px;
      margin: 2px;
    }
    QScrollBar::handle:vertical {
      background: #3f3f46;
      min-height: 40px;
      border-radius: 5px;
    }
    QScrollBar::handle:vertical:hover {
      background: #3b82f6;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
    
    QScrollBar:horizontal {
      background: transparent;
      height: 10px;
      margin: 2px;
    }
    QScrollBar::handle:horizontal {
      background: #3f3f46;
      min-width: 40px;
      border-radius: 5px;
    }
    QScrollBar::handle:horizontal:hover {
      background: #3b82f6;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

    /* === Labels === */
    QLabel {
      color: #F1F5F9;
      background: transparent;
    }
    QLabel[cssClass="muted"] { color: #94A3B8; }
    QLabel[cssClass="heading"] {
      font-size: 18px;
      font-weight: 700;
      color: #F1F5F9;
      letter-spacing: -0.5px;
    }
    QLabel[cssClass="subheading"] {
      font-size: 13px;
      font-weight: 600;
      color: #94A3B8;
      letter-spacing: 0.5px;
      text-transform: uppercase;
    }

    /* === Line Edit / Search === */
    QLineEdit {
      background-color: #18181b;
      border: 1px solid #27272a;
      border-radius: 6px;
      padding: 8px 12px;
      color: #F1F5F9;
      font-size: 13px;
      selection-background-color: #1e3a8a;
    }
    QLineEdit:focus {
      border: 1px solid #3b82f6;
      background-color: #27272a;
    }
    QLineEdit::placeholder { color: #64748B; }

    /* === Push Buttons === */
    QPushButton {
      background-color: #18181b;
      border: 1px solid #27272a;
      border-radius: 6px;
      padding: 8px 16px;
      color: #F1F5F9;
      font-size: 13px;
      font-weight: 600;
    }
    QPushButton:hover {
      background-color: #27272a;
      border-color: #3f3f46;
    }
    QPushButton:pressed {
      background-color: #09090b;
      border-color: #3b82f6;
    }
    QPushButton:disabled {
      color: #64748B;
      background-color: #09090b;
      border-color: #18181b;
    }
    QPushButton[cssClass="primary"] {
      background-color: #3b82f6;
      border: none;
      color: #ffffff;
      font-weight: 700;
    }
    QPushButton[cssClass="primary"]:hover {
      background-color: #60a5fa;
    }
    QPushButton[cssClass="danger"] {
      background-color: transparent;
      border: 1px solid #FF1744;
      color: #FF1744;
    }
    QPushButton[cssClass="danger"]:hover {
      background-color: #FF1744;
      color: #FFFFFF;
    }

    /* === Tree View & Table View === */
    QTreeView, QTableWidget, QTableView {
      background-color: transparent;
      border: none;
      outline: none;
      alternate-background-color: #0c0c0f;
      gridline-color: transparent;
    }
    QTreeView::item, QTableView::item {
      padding: 8px 12px;
      border: none;
      border-bottom: 1px solid #18181b;
    }
    QTreeView::item:hover, QTableView::item:hover {
      background-color: #18181b;
    }
    QTreeView::item:selected, QTableView::item:selected {
      background-color: #1f2937;
      color: #F1F5F9;
    }
    QTreeView::branch { background: transparent; }

    /* === Header View === */
    QHeaderView::section {
      background-color: transparent;
      color: #94A3B8;
      border: none;
      border-bottom: 1px solid #27272a;
      padding: 10px 12px;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QHeaderView::section:hover {
      color: #F1F5F9;
      background-color: #18181b;
    }

    /* === Tab Widget === */
    QTabWidget::pane {
      border: none;
      background-color: #09090b;
    }
    QTabBar::tab {
      background-color: transparent;
      color: #94A3B8;
      padding: 10px 20px;
      border: none;
      border-bottom: 2px solid transparent;
      font-size: 13px;
      font-weight: 500;
    }
    QTabBar::tab:hover { color: #F1F5F9; }
    QTabBar::tab:selected {
      color: #3b82f6;
      border-bottom-color: #3b82f6;
    }

    /* === Status Bar === */
    QStatusBar {
      background-color: #09090b;
      border-top: 1px solid #18181b;
      color: #94A3B8;
      font-size: 12px;
      padding: 0 12px;
    }
    QStatusBar::item { border: none; }

    /* === Splitter === */
    QSplitter::handle { background: #18181b; }
    QSplitter::handle:horizontal { width: 1px; }
    QSplitter::handle:vertical { height: 1px; }

    /* === Progress Bar === */
    QProgressBar {
      background-color: #27272a;
      border: none;
      border-radius: 4px;
      text-align: center;
      color: transparent;
      height: 6px;
    }
    QProgressBar::chunk {
      background-color: #3b82f6;
      border-radius: 4px;
    }
  )");
}

QString GetSidebarStylesheet() {
  return QStringLiteral(R"(
    QWidget#sidebar {
      background-color: #09090b;
      border-right: 1px solid #18181b;
    }

    QPushButton[cssClass="sidebarBtn"] {
      background-color: transparent;
      border: none;
      border-radius: 6px;
      padding: 10px 16px;
      margin: 2px 12px;
      color: #94A3B8;
      font-size: 13px;
      font-weight: 500;
      text-align: left;
    }
    QPushButton[cssClass="sidebarBtn"]:hover {
      background-color: #18181b;
      color: #F1F5F9;
    }
    QPushButton[cssClass="sidebarBtn"]:checked {
      background-color: #1f2937;
      color: #3b82f6;
      font-weight: 600;
      border-left: 3px solid #3b82f6;
      border-top-left-radius: 0;
      border-bottom-left-radius: 0;
    }

    QLabel#sidebarTitle {
      color: #64748B;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      padding: 20px 16px 8px 24px;
      letter-spacing: 1.0px;
    }
    
    QLabel#sidebarLogo {
      color: #F1F5F9;
      font-size: 16px;
      font-weight: 800;
      padding: 20px 16px;
      letter-spacing: 1px;
    }
  )");
}

QString GetCardStylesheet() {
  return QStringLiteral(R"(
    QFrame.card {
      background-color: #18181b;
      border: 1px solid #27272a;
      border-radius: 12px;
    }

    QLabel.cardTitle {
      color: #94A3B8;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    QLabel.cardValue {
      color: #F1F5F9;
      font-size: 28px;
      font-weight: 800;
      letter-spacing: -0.5px;
    }
    QLabel.cardUnit {
      color: #64748B;
      font-size: 13px;
      font-weight: 600;
    }
  )");
}

void ApplyDarkTheme(QApplication* app) {
  if (!app) return;

  // Set application-wide font
  QFont appFont("Segoe UI Variable", 10);
  appFont.setHintingPreference(QFont::PreferFullHinting);
  app->setFont(appFont);

  // Apply the global stylesheet
  QString fullStylesheet = GetDarkStylesheet() + GetSidebarStylesheet() + GetCardStylesheet();
  app->setStyleSheet(fullStylesheet);

  // Set palette for widgets that don't honor stylesheets fully
  QPalette palette;
  palette.setColor(QPalette::Window, QColor("#09090b"));
  palette.setColor(QPalette::WindowText, QColor("#F1F5F9"));
  palette.setColor(QPalette::Base, QColor("#18181b"));
  palette.setColor(QPalette::AlternateBase, QColor("#27272a"));
  palette.setColor(QPalette::Text, QColor("#F1F5F9"));
  palette.setColor(QPalette::Button, QColor("#18181b"));
  palette.setColor(QPalette::ButtonText, QColor("#F1F5F9"));
  palette.setColor(QPalette::Highlight, QColor("#3b82f6"));
  palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
  palette.setColor(QPalette::ToolTipBase, QColor("#1f2937"));
  palette.setColor(QPalette::ToolTipText, QColor("#F1F5F9"));
  palette.setColor(QPalette::PlaceholderText, QColor("#64748B"));
  app->setPalette(palette);
}

} // namespace severance::gui::theme
