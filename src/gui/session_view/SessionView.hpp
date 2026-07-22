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
  
  void onBeginWellness();
  void onEnjoyEqually();
  void onExpressFavoritism();
  void showNextFact();

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

  QPushButton* m_BeginWellnessBtn{nullptr};
  QLabel* m_WellnessDialogue{nullptr};
  QPushButton* m_EnjoyEquallyBtn{nullptr};
  QPushButton* m_ExpressFavoritismBtn{nullptr};
  QLabel* m_WellnessScoreLabel{nullptr};
  QStringList m_WellnessFacts;
  int m_CurrentFactIndex{0};
  int m_WellnessScore{100};

  BiometricGraph* m_BrainwaveGraph{nullptr};
  BiometricGraph* m_StressGraph{nullptr};
  BiometricGraph* m_ChipRejectionGraph{nullptr};
  class QTimer* m_BiometricTimer{nullptr};
  
  bool m_isRecording{false};
};

} // namespace severance::gui::session_view
