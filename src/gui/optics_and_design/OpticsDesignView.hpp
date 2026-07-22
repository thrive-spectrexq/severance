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

protected:
  void resizeEvent(QResizeEvent* event) override;

private slots:
  void onCreateArtwork();
  void onArtworkClicked(const QPixmap& pixmap, const QString& title);

private:
  void setupUI();
  void loadArtworks();
  
  QWidget* createArtworkWidget(const QString& title, const QString& type, const QString& colorHex, const QPixmap& customPixmap = QPixmap());
  QPixmap generateProceduralArtwork();

  QScrollArea* m_ScrollArea{nullptr};
  QGridLayout* m_GalleryLayout{nullptr};
  int m_GalleryItemCount{0};
  
  QWidget* m_Overlay{nullptr};
  QLabel* m_OverlayImage{nullptr};
  QLabel* m_OverlayTitle{nullptr};
};

} // namespace severance::gui::optics_and_design
