#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>

namespace severance::gui::optics_and_design {

class OpticsDesignView : public QWidget {
  Q_OBJECT

public:
  explicit OpticsDesignView(QWidget* parent = nullptr);
  ~OpticsDesignView() override;

private:
  void setupUI();
  void loadArtworks();
  
  QWidget* createArtworkWidget(const QString& title, const QString& type, const QString& colorHex);

  QScrollArea* m_ScrollArea{nullptr};
  QGridLayout* m_GalleryLayout{nullptr};
};

} // namespace severance::gui::optics_and_design
