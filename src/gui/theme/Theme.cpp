#include "Theme.hpp"
#include <QFont>
#include <QFontDatabase>
#include <QPalette>
#include <QGraphicsDropShadowEffect>
#include <QWidget>

namespace severance::gui::theme {

void ApplyDropShadow(QWidget* widget) {
  if (!widget) return;
  auto* shadow = new QGraphicsDropShadowEffect(widget);
  shadow->setBlurRadius(24);
  shadow->setColor(QColor(10, 14, 20, 120));
  shadow->setOffset(0, 2);
  widget->setGraphicsEffect(shadow);
}

QString GetDarkStylesheet() {
  return QStringLiteral(R"(
    /* === LUMON INDUSTRIES — Global === */
    * {
      margin: 0;
      padding: 0;
      font-family: "Segoe UI Variable", "Helvetica Neue", "Inter", "Segoe UI", sans-serif;
    }

    QMainWindow {
      background-color: #0A0E14;
      color: #E8ECEF;
    }

    QWidget {
      background-color: transparent;
      color: #E8ECEF;
      font-size: 13px;
    }

    /* === Scrollbars — Thin, minimal, institutional === */
    QScrollBar:vertical {
      background: transparent;
      width: 6px;
      margin: 0;
    }
    QScrollBar::handle:vertical {
      background: #1C2E38;
      min-height: 40px;
      border-radius: 3px;
    }
    QScrollBar::handle:vertical:hover {
      background: #1A7A5C;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
    
    QScrollBar:horizontal {
      background: transparent;
      height: 6px;
      margin: 0;
    }
    QScrollBar::handle:horizontal {
      background: #1C2E38;
      min-width: 40px;
      border-radius: 3px;
    }
    QScrollBar::handle:horizontal:hover {
      background: #1A7A5C;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

    /* === Labels === */
    QLabel {
      color: #E8ECEF;
      background: transparent;
    }
    QLabel[cssClass="muted"] { color: #8B9DAF; }
    QLabel[cssClass="heading"] {
      font-size: 18px;
      font-weight: 700;
      color: #E8ECEF;
      letter-spacing: 0.5px;
    }
    QLabel[cssClass="subheading"] {
      font-size: 11px;
      font-weight: 700;
      color: #8B9DAF;
      letter-spacing: 1.5px;
      text-transform: uppercase;
    }

    /* === Line Edit / Search — Dark with teal focus border === */
    QLineEdit {
      background-color: #0F1A1F;
      border: 1px solid #1C2E38;
      border-radius: 3px;
      padding: 8px 12px;
      color: #E8ECEF;
      font-size: 13px;
      selection-background-color: rgba(26, 122, 92, 0.35);
    }
    QLineEdit:focus {
      border: 1px solid #1A7A5C;
      background-color: #0F1A1F;
    }
    QLineEdit::placeholder { color: #3D4F5F; }

    /* === Push Buttons — Flat, uppercase, teal hover glow === */
    QPushButton {
      background-color: #0F1A1F;
      border: 1px solid #1C2E38;
      border-radius: 3px;
      padding: 8px 16px;
      color: #E8ECEF;
      font-size: 12px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QPushButton:hover {
      background-color: rgba(26, 122, 92, 0.25);
      border-color: #1A7A5C;
      color: #7FDBCA;
    }
    QPushButton:pressed {
      background-color: #1A7A5C;
      border-color: #2ECC71;
      color: #E8ECEF;
    }
    QPushButton:disabled {
      color: #3D4F5F;
      background-color: #0A0E14;
      border-color: #1C2E38;
    }
    QPushButton[cssClass="primary"] {
      background-color: #1A7A5C;
      border: none;
      color: #E8ECEF;
      font-weight: 700;
    }
    QPushButton[cssClass="primary"]:hover {
      background-color: #2ECC71;
      color: #0A0E14;
    }
    QPushButton[cssClass="danger"] {
      background-color: transparent;
      border: 1px solid #C0392B;
      color: #C0392B;
    }
    QPushButton[cssClass="danger"]:hover {
      background-color: #C0392B;
      color: #E8ECEF;
    }

    /* === Tree View & Table View — Institutional data grids === */
    QTreeView, QTableWidget, QTableView {
      background-color: transparent;
      border: none;
      outline: none;
      alternate-background-color: #0D1117;
      gridline-color: transparent;
    }
    QTreeView::item, QTableView::item {
      padding: 8px 12px;
      border: none;
      border-bottom: 1px solid #1C2E38;
    }
    QTreeView::item:hover, QTableView::item:hover {
      background-color: rgba(26, 122, 92, 0.25);
    }
    QTreeView::item:selected, QTableView::item:selected {
      background-color: rgba(26, 122, 92, 0.35);
      color: #E8ECEF;
    }
    QTreeView::branch { background: transparent; }
    QTreeView::branch:has-children:!has-siblings:closed,
    QTreeView::branch:closed:has-children:has-siblings {
      border-image: none;
    }

    /* === Header View — Uppercase, institutional === */
    QHeaderView::section {
      background-color: transparent;
      color: #8B9DAF;
      border: none;
      border-bottom: 1px solid #1C2E38;
      padding: 10px 12px;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1.5px;
    }
    QHeaderView::section:hover {
      color: #7FDBCA;
      background-color: rgba(26, 122, 92, 0.15);
    }

    /* === Tab Widget — Geometric, teal active indicator === */
    QTabWidget::pane {
      border: none;
      background-color: #0A0E14;
    }
    QTabBar::tab {
      background-color: transparent;
      color: #8B9DAF;
      padding: 10px 20px;
      border: none;
      border-bottom: 2px solid transparent;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    QTabBar::tab:hover {
      color: #7FDBCA;
    }
    QTabBar::tab:selected {
      color: #2ECC71;
      border-bottom-color: #1A7A5C;
    }

    /* === Status Bar === */
    QStatusBar {
      background-color: #0A0E14;
      border-top: 1px solid #1C2E38;
      color: #8B9DAF;
      font-size: 11px;
      font-weight: 500;
      padding: 0 12px;
      letter-spacing: 0.5px;
    }
    QStatusBar::item { border: none; }

    /* === Splitter === */
    QSplitter::handle { background: #1C2E38; }
    QSplitter::handle:horizontal { width: 1px; }
    QSplitter::handle:vertical { height: 1px; }

    /* === Progress Bar — Lumon teal fill === */
    QProgressBar {
      background-color: #132A2E;
      border: none;
      border-radius: 3px;
      text-align: center;
      color: transparent;
      height: 6px;
    }
    QProgressBar::chunk {
      background-color: #1A7A5C;
      border-radius: 3px;
    }

    /* === Tooltips — Clinical dark panels === */
    QToolTip {
      background-color: #132A2E;
      color: #E8ECEF;
      border: 1px solid #1C2E38;
      padding: 6px 10px;
      font-size: 12px;
    }

    /* === Combo Boxes === */
    QComboBox {
      background-color: #0F1A1F;
      border: 1px solid #1C2E38;
      border-radius: 3px;
      padding: 6px 12px;
      color: #E8ECEF;
    }
    QComboBox:hover {
      border-color: #1A7A5C;
    }
    QComboBox::drop-down {
      border: none;
    }
    QComboBox QAbstractItemView {
      background-color: #0F1A1F;
      border: 1px solid #1C2E38;
      selection-background-color: rgba(26, 122, 92, 0.35);
      color: #E8ECEF;
    }

    /* === Menu === */
    QMenu {
      background-color: #0F1A1F;
      border: 1px solid #1C2E38;
      padding: 4px 0;
    }
    QMenu::item {
      padding: 8px 24px;
      color: #E8ECEF;
    }
    QMenu::item:selected {
      background-color: rgba(26, 122, 92, 0.25);
      color: #7FDBCA;
    }
    QMenu::separator {
      height: 1px;
      background: #1C2E38;
      margin: 4px 8px;
    }
  )");
}

QString GetSidebarStylesheet() {
  return QStringLiteral(R"(
    QWidget#sidebar {
      background-color: #0A0E14;
      border-right: 1px solid #1C2E38;
    }

    QPushButton[cssClass="sidebarBtn"] {
      background-color: transparent;
      border: none;
      border-radius: 3px;
      padding: 10px 16px;
      margin: 2px 12px;
      color: #8B9DAF;
      font-size: 12px;
      font-weight: 600;
      text-align: left;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QPushButton[cssClass="sidebarBtn"]:hover {
      background-color: rgba(26, 122, 92, 0.25);
      color: #7FDBCA;
    }
    QPushButton[cssClass="sidebarBtn"]:checked {
      background-color: rgba(26, 122, 92, 0.15);
      color: #2ECC71;
      font-weight: 700;
      border-left: 3px solid #1A7A5C;
      border-top-left-radius: 0;
      border-bottom-left-radius: 0;
    }

    QLabel#sidebarTitle {
      color: #3D4F5F;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      padding: 20px 16px 8px 24px;
      letter-spacing: 2.0px;
    }
    
    QLabel#sidebarLogo {
      color: #7FDBCA;
      font-size: 16px;
      font-weight: 800;
      padding: 20px 16px;
      letter-spacing: 3px;
      text-transform: uppercase;
    }
  )");
}

QString GetCardStylesheet() {
  return QStringLiteral(R"(
    QFrame.card {
      background-color: #0F1A1F;
      border: 1px solid #1C2E38;
      border-radius: 4px;
    }

    QLabel.cardTitle {
      color: #8B9DAF;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 2px;
    }
    QLabel.cardValue {
      color: #E8ECEF;
      font-size: 28px;
      font-weight: 800;
      letter-spacing: -0.5px;
    }
    QLabel.cardUnit {
      color: #3D4F5F;
      font-size: 13px;
      font-weight: 600;
    }
  )");
}

void ApplyDarkTheme(QApplication* app) {
  if (!app) return;

  // Set application-wide font — clean, institutional sans-serif
  QFont appFont("Segoe UI Variable", 10);
  appFont.setHintingPreference(QFont::PreferFullHinting);
  app->setFont(appFont);

  // Apply the global stylesheet
  QString fullStylesheet = GetDarkStylesheet() + GetSidebarStylesheet() + GetCardStylesheet();
  app->setStyleSheet(fullStylesheet);

  // Set palette for widgets that don't honor stylesheets fully
  // Lumon Industries color mapping
  QPalette palette;
  palette.setColor(QPalette::Window, QColor("#0A0E14"));
  palette.setColor(QPalette::WindowText, QColor("#E8ECEF"));
  palette.setColor(QPalette::Base, QColor("#0F1A1F"));
  palette.setColor(QPalette::AlternateBase, QColor("#132A2E"));
  palette.setColor(QPalette::Text, QColor("#E8ECEF"));
  palette.setColor(QPalette::Button, QColor("#0F1A1F"));
  palette.setColor(QPalette::ButtonText, QColor("#E8ECEF"));
  palette.setColor(QPalette::Highlight, QColor("#1A7A5C"));
  palette.setColor(QPalette::HighlightedText, QColor("#E8ECEF"));
  palette.setColor(QPalette::ToolTipBase, QColor("#132A2E"));
  palette.setColor(QPalette::ToolTipText, QColor("#E8ECEF"));
  palette.setColor(QPalette::PlaceholderText, QColor("#3D4F5F"));
  palette.setColor(QPalette::Link, QColor("#7FDBCA"));
  palette.setColor(QPalette::LinkVisited, QColor("#1A7A5C"));
  app->setPalette(palette);
}

} // namespace severance::gui::theme
