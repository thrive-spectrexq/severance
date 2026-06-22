#include "Theme.hpp"
#include <QFont>
#include <QFontDatabase>
#include <QPalette>

namespace severance::gui::theme {

QString GetDarkStylesheet() {
  return QStringLiteral(R"(
    /* === Global === */
    * {
      margin: 0;
      padding: 0;
      font-family: "Segoe UI Variable", "Inter", "Segoe UI", sans-serif;
    }

    QMainWindow {
      background-color: #0B0E14;
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
      background: #2A3441;
      min-height: 40px;
      border-radius: 5px;
    }
    QScrollBar::handle:vertical:hover {
      background: #00E5FF;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
      height: 0;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
      background: transparent;
    }
    QScrollBar:horizontal {
      background: transparent;
      height: 10px;
      margin: 2px;
    }
    QScrollBar::handle:horizontal {
      background: #2A3441;
      min-width: 40px;
      border-radius: 5px;
    }
    QScrollBar::handle:horizontal:hover {
      background: #00E5FF;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
      width: 0;
    }

    /* === Labels === */
    QLabel {
      color: #F1F5F9;
      background: transparent;
    }
    QLabel[cssClass="muted"] {
      color: #94A3B8;
    }
    QLabel[cssClass="heading"] {
      font-size: 18px;
      font-weight: 700;
      color: #F1F5F9;
      letter-spacing: -0.5px;
      text-transform: uppercase;
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
      background-color: #151A23;
      border: 1px solid #2A3441;
      border-radius: 4px;
      padding: 6px 12px;
      color: #F1F5F9;
      font-size: 13px;
      selection-background-color: #1A2E44;
    }
    QLineEdit:focus {
      border: 1px solid #00E5FF;
      background-color: #1C2331;
    }
    QLineEdit::placeholder {
      color: #64748B;
    }

    /* === Push Buttons === */
    QPushButton {
      background-color: #1C2331;
      border: 1px solid #2A3441;
      border-radius: 6px;
      padding: 6px 16px;
      color: #F1F5F9;
      font-size: 13px;
      font-weight: 600;
    }
    QPushButton:hover {
      background-color: #242D3D;
      border-color: #94A3B8;
      color: #FFFFFF;
    }
    QPushButton:pressed {
      background-color: #151A23;
      border-color: #00E5FF;
    }
    QPushButton:disabled {
      color: #64748B;
      background-color: #0B0E14;
      border-color: #1C2331;
    }
    QPushButton[cssClass="primary"] {
      background-color: #00E5FF;
      border: none;
      color: #0B0E14;
      font-weight: 700;
    }
    QPushButton[cssClass="primary"]:hover {
      background-color: #33EFFF;
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
      background-color: #151A23;
      border: 1px solid #2A3441;
      border-radius: 6px;
      outline: none;
      alternate-background-color: #1A202C;
      gridline-color: #2A3441;
    }
    QTreeView::item, QTableView::item {
      padding: 6px 8px;
      border: none;
    }
    QTreeView::item:hover, QTableView::item:hover {
      background-color: #1C2331;
    }
    QTreeView::item:selected, QTableView::item:selected {
      background-color: #1A2E44;
      color: #F1F5F9;
    }
    QTreeView::branch {
      background: transparent;
    }

    /* === Header View (column headers) === */
    QHeaderView::section {
      background-color: #151A23;
      color: #94A3B8;
      border: none;
      border-bottom: 1px solid #2A3441;
      padding: 8px 8px;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QHeaderView::section:hover {
      color: #F1F5F9;
      background-color: #1C2331;
    }

    /* === Tab Widget === */
    QTabWidget::pane {
      border: none;
      background-color: #0B0E14;
    }
    QTabBar::tab {
      background-color: transparent;
      color: #94A3B8;
      padding: 8px 16px;
      border: none;
      border-bottom: 2px solid transparent;
      font-size: 13px;
    }
    QTabBar::tab:hover {
      color: #F1F5F9;
    }
    QTabBar::tab:selected {
      color: #F1F5F9;
      border-bottom-color: #00E5FF;
    }

    /* === Status Bar === */
    QStatusBar {
      background-color: #151A23;
      border-top: 1px solid #2A3441;
      color: #94A3B8;
      font-size: 12px;
      padding: 0 8px;
    }
    QStatusBar::item {
      border: none;
    }

    /* === Menu === */
    QMenu {
      background-color: #151A23;
      border: 1px solid #2A3441;
      border-radius: 6px;
      padding: 4px;
    }
    QMenu::item {
      padding: 6px 24px;
      border-radius: 4px;
      color: #F1F5F9;
    }
    QMenu::item:selected {
      background-color: #1C2331;
      color: #00E5FF;
    }
    QMenu::separator {
      height: 1px;
      background: #2A3441;
      margin: 4px 8px;
    }

    /* === Tooltips === */
    QToolTip {
      background-color: #1C2331;
      border: 1px solid #2A3441;
      border-radius: 4px;
      padding: 6px 10px;
      color: #F1F5F9;
      font-size: 12px;
    }

    /* === Splitter === */
    QSplitter::handle {
      background: #2A3441;
    }
    QSplitter::handle:horizontal {
      width: 1px;
    }
    QSplitter::handle:vertical {
      height: 1px;
    }

    /* === Progress Bar === */
    QProgressBar {
      background-color: #1C2331;
      border: none;
      border-radius: 4px;
      text-align: center;
      color: #F1F5F9;
      font-size: 11px;
      height: 8px;
    }
    QProgressBar::chunk {
      background-color: #00E5FF;
      border-radius: 4px;
    }
  )");
}

QString GetSidebarStylesheet() {
  return QStringLiteral(R"(
    QWidget#sidebar {
      background-color: #0B0E14;
      border-right: 1px solid #2A3441;
    }

    QPushButton[cssClass="sidebarBtn"] {
      background-color: transparent;
      border: none;
      border-radius: 4px;
      padding: 6px 12px;
      margin: 2px 8px;
      color: #94A3B8;
      font-size: 13px;
      font-weight: 500;
      text-align: left;
    }
    QPushButton[cssClass="sidebarBtn"]:hover {
      background-color: #151A23;
      color: #F1F5F9;
    }
    QPushButton[cssClass="sidebarBtn"]:checked {
      background-color: #1A2E44;
      color: #00E5FF;
      font-weight: 600;
    }

    QLabel#sidebarTitle {
      color: #64748B;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      padding: 16px 16px 6px 16px;
      letter-spacing: 1.0px;
    }
    
    QLabel#sidebarLogo {
      color: #F1F5F9;
      font-size: 16px;
      font-weight: 800;
      padding: 16px;
      letter-spacing: 0.5px;
    }
  )");
}

QString GetCardStylesheet() {
  return QStringLiteral(R"(
    QFrame.card {
      background-color: #151A23;
      border: 1px solid #2A3441;
      border-radius: 8px;
    }
    QFrame.card:hover {
      border-color: #3B4B5E;
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
      font-size: 24px;
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
  QFont appFont("Segoe UI", 10);
  appFont.setHintingPreference(QFont::PreferFullHinting);
  app->setFont(appFont);

  // Apply the global stylesheet
  QString fullStylesheet = GetDarkStylesheet() + GetSidebarStylesheet() + GetCardStylesheet();
  app->setStyleSheet(fullStylesheet);

  // Set palette for widgets that don't honor stylesheets fully
  QPalette palette;
  palette.setColor(QPalette::Window, QColor("#0D1117"));
  palette.setColor(QPalette::WindowText, QColor("#E6EDF3"));
  palette.setColor(QPalette::Base, QColor("#0D1117"));
  palette.setColor(QPalette::AlternateBase, QColor("#161B22"));
  palette.setColor(QPalette::Text, QColor("#E6EDF3"));
  palette.setColor(QPalette::Button, QColor("#21262D"));
  palette.setColor(QPalette::ButtonText, QColor("#E6EDF3"));
  palette.setColor(QPalette::Highlight, QColor("#1F3A5F"));
  palette.setColor(QPalette::HighlightedText, QColor("#E6EDF3"));
  palette.setColor(QPalette::ToolTipBase, QColor("#1C2128"));
  palette.setColor(QPalette::ToolTipText, QColor("#E6EDF3"));
  palette.setColor(QPalette::PlaceholderText, QColor("#6E7681"));
  app->setPalette(palette);
}

} // namespace severance::gui::theme
