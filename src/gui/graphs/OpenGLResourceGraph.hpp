#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QTimer>
#include <vector>

namespace severance::gui::graphs {

class OpenGLResourceGraph : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit OpenGLResourceGraph(QWidget *parent = nullptr);
  ~OpenGLResourceGraph() override;

  void addDataPoint(double value);
  void clear();

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

private slots:
  void updateData();

private:
  void updateBuffers();

  std::vector<double> m_Data;
  size_t m_MaxPoints{100};
  QTimer* m_Timer{nullptr};

  QOpenGLShaderProgram* m_Program{nullptr};
  QOpenGLBuffer m_Vbo;
  QOpenGLVertexArrayObject m_Vao;
  
  int m_VertexAttr{-1};
  int m_ColorUniform{-1};

  bool m_NeedsBufferUpdate{true};
};

} // namespace severance::gui::graphs
