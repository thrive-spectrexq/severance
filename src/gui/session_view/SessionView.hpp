#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>

namespace severance::gui::session_view {

class BiometricGraph;

class SessionView : public QWidget {
  Q_OBJECT

public:
  explicit SessionView(QWidget *parent = nullptr);
  ~SessionView() override = default;

private slots:
  void onToggleRecording();
  void onAddAnnotation();
  void onExportMarkdown();
  void onExportJson();
  void updateStatus();
  void updateBiometrics();

private:
  void setupUI();

  QPushButton* m_RecordBtn{nullptr};
  QLabel* m_StatusLabel{nullptr};
  QLabel* m_EventCountLabel{nullptr};

  QLineEdit* m_AnnotationInput{nullptr};
  QPushButton* m_AddAnnotationBtn{nullptr};
  QListWidget* m_AnnotationList{nullptr};

  QPushButton* m_ExportMdBtn{nullptr};
  QPushButton* m_ExportJsonBtn{nullptr};

  BiometricGraph* m_BrainwaveGraph{nullptr};
  BiometricGraph* m_StressGraph{nullptr};
  BiometricGraph* m_ChipRejectionGraph{nullptr};
  class QTimer* m_BiometricTimer{nullptr};
  
  bool m_isRecording{false};
};

} // namespace severance::gui::session_view
