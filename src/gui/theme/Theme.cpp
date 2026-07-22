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
    /* === LUMON INDUSTRIES — Institutional CRT Theme === */
    * {
      margin: 0;
      padding: 0;
      font-family: "Courier New", "Consolas", "Segoe UI Variable", sans-serif;
    }

    QMainWindow {
      background-color: #050B09;
      color: #D0F5E8;
    }

    QWidget {
      background-color: transparent;
      color: #D0F5E8;
      font-size: 13px;
    }

    /* === Scrollbars — Minimal Phosphor === */
    QScrollBar:vertical {
      background: transparent;
      width: 6px;
      margin: 0;
    }
    QScrollBar::handle:vertical {
      background: #143832;
      min-height: 40px;
      border-radius: 3px;
    }
    QScrollBar::handle:vertical:hover {
      background: #208A7C;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
    
    QScrollBar:horizontal {
      background: transparent;
      height: 6px;
      margin: 0;
    }
    QScrollBar::handle:horizontal {
      background: #143832;
      min-width: 40px;
      border-radius: 3px;
    }
    QScrollBar::handle:horizontal:hover {
      background: #208A7C;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

    /* === Labels === */
    QLabel {
      color: #D0F5E8;
      background: transparent;
    }
    QLabel[cssClass="muted"] { color: #3AA394; }
    QLabel[cssClass="heading"] {
      font-size: 18px;
      font-weight: 700;
      color: #20F8D5;
      letter-spacing: 1px;
    }
    QLabel[cssClass="subheading"] {
      font-size: 11px;
      font-weight: 700;
      color: #3AA394;
      letter-spacing: 1.5px;
      text-transform: uppercase;
    }

    /* === Line Edit / Search — Dark with phosphor cyan focus border === */
    QLineEdit {
      background-color: #08120F;
      border: 1px solid #143832;
      border-radius: 3px;
      padding: 8px 12px;
      color: #20F8D5;
      font-size: 13px;
      selection-background-color: rgba(32, 248, 213, 0.35);
    }
    QLineEdit:focus {
      border: 1px solid #20F8D5;
      background-color: #08120F;
    }
    QLineEdit::placeholder { color: #1D524A; }

    /* === Push Buttons — Flat, uppercase, phosphor glow === */
    QPushButton {
      background-color: #08120F;
      border: 1px solid #143832;
      border-radius: 3px;
      padding: 8px 16px;
      color: #D0F5E8;
      font-size: 12px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QPushButton:hover {
      background-color: rgba(32, 248, 213, 0.2);
      border-color: #20F8D5;
      color: #20F8D5;
    }
    QPushButton:pressed {
      background-color: #208A7C;
      border-color: #20F8D5;
      color: #050B09;
    }
    QPushButton:disabled {
      color: #1D524A;
      background-color: #050B09;
      border-color: #143832;
    }
    QPushButton[cssClass="primary"] {
      background-color: #208A7C;
      border: none;
      color: #050B09;
      font-weight: 700;
    }
    QPushButton[cssClass="primary"]:hover {
      background-color: #20F8D5;
      color: #050B09;
    }

    /* === Table & Tree View === */
    QTreeView, QTableWidget, QTableView {
      background-color: transparent;
      border: none;
      outline: none;
      alternate-background-color: #070E0C;
      gridline-color: transparent;
    }
    QTreeView::item, QTableView::item {
      padding: 8px 12px;
      border: none;
      border-bottom: 1px solid #143832;
    }
    QTreeView::item:hover, QTableView::item:hover {
      background-color: rgba(32, 248, 213, 0.15);
    }
    QTreeView::item:selected, QTableView::item:selected {
      background-color: rgba(32, 248, 213, 0.25);
      color: #20F8D5;
    }

    /* === Header View === */
    QHeaderView::section {
      background-color: transparent;
      color: #3AA394;
      border: none;
      border-bottom: 1px solid #143832;
      padding: 10px 12px;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1.5px;
    }
    QHeaderView::section:hover {
      color: #20F8D5;
      background-color: rgba(32, 248, 213, 0.1);
    }

    /* === Tab Widget === */
    QTabWidget::pane {
      border: none;
      background-color: #050B09;
    }
    QTabBar::tab {
      background-color: transparent;
      color: #3AA394;
      padding: 10px 20px;
      border: none;
      border-bottom: 2px solid transparent;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    QTabBar::tab:hover {
      color: #20F8D5;
    }
    QTabBar::tab:selected {
      color: #20F8D5;
      border-bottom-color: #20F8D5;
    }

    /* === Status Bar === */
    QStatusBar {
      background-color: #050B09;
      border-top: 1px solid #143832;
      color: #3AA394;
      font-size: 11px;
      font-weight: 500;
      padding: 0 12px;
      letter-spacing: 0.5px;
    }
    QStatusBar::item { border: none; }

    /* === Splitter === */
    QSplitter::handle { background: #143832; }
    QSplitter::handle:horizontal { width: 1px; }
    QSplitter::handle:vertical { height: 1px; }

    /* === Progress Bar === */
    QProgressBar {
      background-color: #08120F;
      border: 1px solid #143832;
      border-radius: 3px;
      text-align: center;
      color: transparent;
      height: 6px;
    }
    QProgressBar::chunk {
      background-color: #20F8D5;
      border-radius: 3px;
    }

    /* === Tooltips === */
    QToolTip {
      background-color: #08120F;
      color: #D0F5E8;
      border: 1px solid #143832;
      padding: 6px 10px;
      font-size: 12px;
    }
  )");
}

QString GetSidebarStylesheet() {
  return QStringLiteral(R"(
    QWidget#sidebar {
      background-color: #040807;
      border-right: 1px solid #143832;
    }

    QPushButton[cssClass="sidebarBtn"] {
      background-color: transparent;
      border: none;
      border-radius: 3px;
      padding: 10px 16px;
      margin: 2px 12px;
      color: #3AA394;
      font-size: 12px;
      font-weight: 600;
      text-align: left;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QPushButton[cssClass="sidebarBtn"]:hover {
      background-color: rgba(32, 248, 213, 0.15);
      color: #20F8D5;
    }
    QPushButton[cssClass="sidebarBtn"]:checked {
      background-color: rgba(32, 248, 213, 0.12);
      color: #20F8D5;
      font-weight: 700;
      border-left: 3px solid #20F8D5;
      border-top-left-radius: 0;
      border-bottom-left-radius: 0;
    }

    QLabel#sidebarTitle {
      color: #1D524A;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      padding: 20px 16px 8px 24px;
      letter-spacing: 2.0px;
    }
    
    QLabel#sidebarLogo {
      color: #20F8D5;
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
      background-color: #08120F;
      border: 1px solid #143832;
      border-radius: 4px;
    }

    QLabel.cardTitle {
      color: #3AA394;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 2px;
    }
    QLabel.cardValue {
      color: #20F8D5;
      font-size: 28px;
      font-weight: 800;
      letter-spacing: -0.5px;
    }
    QLabel.cardUnit {
      color: #1D524A;
      font-size: 13px;
      font-weight: 600;
    }
  )");
}

void ApplyDarkTheme(QApplication* app) {
  if (!app) return;

  QFont appFont("Courier New", 10);
  appFont.setHintingPreference(QFont::PreferFullHinting);
  app->setFont(appFont);

  QString fullStylesheet = GetDarkStylesheet() + GetSidebarStylesheet() + GetCardStylesheet();
  app->setStyleSheet(fullStylesheet);

  QPalette palette;
  palette.setColor(QPalette::Window, QColor("#050B09"));
  palette.setColor(QPalette::WindowText, QColor("#D0F5E8"));
  palette.setColor(QPalette::Base, QColor("#08120F"));
  palette.setColor(QPalette::AlternateBase, QColor("#070E0C"));
  palette.setColor(QPalette::Text, QColor("#20F8D5"));
  palette.setColor(QPalette::Button, QColor("#08120F"));
  palette.setColor(QPalette::ButtonText, QColor("#D0F5E8"));
  palette.setColor(QPalette::Highlight, QColor("#208A7C"));
  palette.setColor(QPalette::HighlightedText, QColor("#050B09"));
  palette.setColor(QPalette::ToolTipBase, QColor("#08120F"));
  palette.setColor(QPalette::ToolTipText, QColor("#D0F5E8"));
  palette.setColor(QPalette::PlaceholderText, QColor("#1D524A"));
  palette.setColor(QPalette::Link, QColor("#20F8D5"));
  palette.setColor(QPalette::LinkVisited, QColor("#208A7C"));
  app->setPalette(palette);
}

} // namespace severance::gui::theme
