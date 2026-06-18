#include "Theme.hpp"
#include <QFont>
#include <QFontDatabase>

namespace severance::gui::theme {

QString GetDarkStylesheet() {
  return QStringLiteral(R"(
    /* === Global === */
    * {
      margin: 0;
      padding: 0;
    }

    QMainWindow {
      background-color: #0D1117;
      color: #E6EDF3;
    }

    QWidget {
      background-color: transparent;
      color: #E6EDF3;
      font-size: 13px;
    }

    /* === Scrollbars === */
    QScrollBar:vertical {
      background: transparent;
      width: 8px;
      margin: 0;
    }
    QScrollBar::handle:vertical {
      background: #30363D;
      min-height: 30px;
      border-radius: 4px;
    }
    QScrollBar::handle:vertical:hover {
      background: #484F58;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
      height: 0;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
      background: transparent;
    }
    QScrollBar:horizontal {
      background: transparent;
      height: 8px;
      margin: 0;
    }
    QScrollBar::handle:horizontal {
      background: #30363D;
      min-width: 30px;
      border-radius: 4px;
    }
    QScrollBar::handle:horizontal:hover {
      background: #484F58;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
      width: 0;
    }

    /* === Labels === */
    QLabel {
      color: #E6EDF3;
      background: transparent;
    }
    QLabel[cssClass="muted"] {
      color: #8B949E;
    }
    QLabel[cssClass="heading"] {
      font-size: 18px;
      font-weight: 600;
      color: #E6EDF3;
    }
    QLabel[cssClass="subheading"] {
      font-size: 14px;
      font-weight: 500;
      color: #8B949E;
    }

    /* === Line Edit / Search === */
    QLineEdit {
      background-color: #161B22;
      border: 1px solid #30363D;
      border-radius: 6px;
      padding: 8px 12px;
      color: #E6EDF3;
      font-size: 14px;
      selection-background-color: #1F3A5F;
    }
    QLineEdit:focus {
      border-color: #58A6FF;
    }
    QLineEdit::placeholder {
      color: #6E7681;
    }

    /* === Push Buttons === */
    QPushButton {
      background-color: #21262D;
      border: 1px solid #30363D;
      border-radius: 6px;
      padding: 6px 16px;
      color: #E6EDF3;
      font-size: 13px;
      font-weight: 500;
    }
    QPushButton:hover {
      background-color: #30363D;
      border-color: #484F58;
    }
    QPushButton:pressed {
      background-color: #1C2128;
    }
    QPushButton:disabled {
      color: #6E7681;
      background-color: #161B22;
      border-color: #21262D;
    }
    QPushButton[cssClass="primary"] {
      background-color: #1F6FEB;
      border-color: #1F6FEB;
      color: #FFFFFF;
    }
    QPushButton[cssClass="primary"]:hover {
      background-color: #388BFD;
    }
    QPushButton[cssClass="danger"] {
      background-color: #DA3633;
      border-color: #DA3633;
      color: #FFFFFF;
    }
    QPushButton[cssClass="danger"]:hover {
      background-color: #F85149;
    }

    /* === Tree View === */
    QTreeView {
      background-color: #0D1117;
      border: none;
      outline: none;
      alternate-background-color: #161B22;
    }
    QTreeView::item {
      padding: 4px 8px;
      border: none;
    }
    QTreeView::item:hover {
      background-color: #21262D;
    }
    QTreeView::item:selected {
      background-color: #1F3A5F;
      color: #E6EDF3;
    }
    QTreeView::branch {
      background: transparent;
    }

    /* === Header View (column headers) === */
    QHeaderView::section {
      background-color: #161B22;
      color: #8B949E;
      border: none;
      border-bottom: 1px solid #30363D;
      border-right: 1px solid #21262D;
      padding: 6px 8px;
      font-size: 12px;
      font-weight: 600;
      text-transform: uppercase;
    }
    QHeaderView::section:hover {
      color: #E6EDF3;
      background-color: #1C2128;
    }

    /* === Tab Widget === */
    QTabWidget::pane {
      border: none;
      background-color: #0D1117;
    }
    QTabBar::tab {
      background-color: transparent;
      color: #8B949E;
      padding: 8px 16px;
      border: none;
      border-bottom: 2px solid transparent;
      font-size: 13px;
    }
    QTabBar::tab:hover {
      color: #E6EDF3;
    }
    QTabBar::tab:selected {
      color: #E6EDF3;
      border-bottom-color: #58A6FF;
    }

    /* === Status Bar === */
    QStatusBar {
      background-color: #161B22;
      border-top: 1px solid #30363D;
      color: #8B949E;
      font-size: 12px;
      padding: 0 8px;
    }
    QStatusBar::item {
      border: none;
    }

    /* === Menu === */
    QMenu {
      background-color: #161B22;
      border: 1px solid #30363D;
      border-radius: 8px;
      padding: 4px;
    }
    QMenu::item {
      padding: 6px 24px;
      border-radius: 4px;
      color: #E6EDF3;
    }
    QMenu::item:selected {
      background-color: #1F3A5F;
    }
    QMenu::separator {
      height: 1px;
      background: #30363D;
      margin: 4px 8px;
    }

    /* === Tooltips === */
    QToolTip {
      background-color: #1C2128;
      border: 1px solid #30363D;
      border-radius: 4px;
      padding: 4px 8px;
      color: #E6EDF3;
      font-size: 12px;
    }

    /* === Splitter === */
    QSplitter::handle {
      background: #30363D;
    }
    QSplitter::handle:horizontal {
      width: 1px;
    }
    QSplitter::handle:vertical {
      height: 1px;
    }

    /* === Progress Bar === */
    QProgressBar {
      background-color: #21262D;
      border: none;
      border-radius: 4px;
      text-align: center;
      color: #E6EDF3;
      font-size: 11px;
      height: 8px;
    }
    QProgressBar::chunk {
      background-color: #58A6FF;
      border-radius: 4px;
    }
  )");
}

QString GetSidebarStylesheet() {
  return QStringLiteral(R"(
    QWidget#sidebar {
      background-color: #161B22;
      border-right: 1px solid #30363D;
    }

    QPushButton.sidebarBtn {
      background-color: transparent;
      border: none;
      border-radius: 8px;
      padding: 12px;
      margin: 2px 6px;
      color: #8B949E;
      font-size: 18px;
      text-align: center;
    }
    QPushButton.sidebarBtn:hover {
      background-color: #21262D;
      color: #E6EDF3;
    }
    QPushButton.sidebarBtn:checked {
      background-color: #1F3A5F;
      color: #58A6FF;
    }

    QLabel#sidebarTitle {
      color: #6E7681;
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      padding: 8px 12px 4px 12px;
      letter-spacing: 1px;
    }
  )");
}

QString GetCardStylesheet() {
  return QStringLiteral(R"(
    QFrame.card {
      background-color: #161B22;
      border: 1px solid #30363D;
      border-radius: 8px;
    }
    QFrame.card:hover {
      border-color: #484F58;
    }

    QLabel.cardTitle {
      color: #8B949E;
      font-size: 12px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    QLabel.cardValue {
      color: #E6EDF3;
      font-size: 24px;
      font-weight: 700;
    }
    QLabel.cardUnit {
      color: #6E7681;
      font-size: 13px;
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
