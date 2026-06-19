#include "OpenGLResourceGraph.hpp"
#include <QPainter>

namespace severance::gui::graphs {

// Basic shaders
static const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 position;
void main() {
    // Map from [0, 1] to NDC [-1, 1]
    gl_Position = vec4(position.x * 2.0 - 1.0, position.y * 2.0 - 1.0, 0.0, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
}
)";

OpenGLResourceGraph::OpenGLResourceGraph(QWidget *parent)
  : QOpenGLWidget(parent), m_Vbo(QOpenGLBuffer::VertexBuffer) {
  setMinimumHeight(60);
  m_Data.assign(m_MaxPoints, 0.0);

  m_Timer = new QTimer(this);
  connect(m_Timer, &QTimer::timeout, this, &OpenGLResourceGraph::updateData);
  m_Timer->start(100); // 10 FPS updates
}

OpenGLResourceGraph::~OpenGLResourceGraph() {
  makeCurrent();
  delete m_Program;
  m_Vbo.destroy();
  m_Vao.destroy();
  doneCurrent();
}

void OpenGLResourceGraph::addDataPoint(double value) {
  m_Data.erase(m_Data.begin());
  m_Data.push_back(std::clamp(value, 0.0, 100.0));
  m_NeedsBufferUpdate = true;
  update();
}

void OpenGLResourceGraph::clear() {
  m_Data.assign(m_MaxPoints, 0.0);
  m_NeedsBufferUpdate = true;
  update();
}

void OpenGLResourceGraph::updateData() {
  // If we wanted to animate/slide automatically, we could do it here
  // For now, this is driven externally by DashboardView calling addDataPoint
}

void OpenGLResourceGraph::initializeGL() {
  initializeOpenGLFunctions();

  m_Program = new QOpenGLShaderProgram(this);
  m_Program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
  m_Program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
  m_Program->link();

  m_VertexAttr = m_Program->attributeLocation("position");
  m_ColorUniform = m_Program->uniformLocation("color");

  m_Vao.create();
  m_Vao.bind();

  m_Vbo.create();
  m_Vbo.bind();
  m_Vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);

  // We need 2 vertices per data point for a line strip (or triangles for filled area)
  // Let's do a simple line strip for now
  m_Vbo.allocate(m_MaxPoints * 2 * sizeof(float));

  m_Program->enableAttributeArray(m_VertexAttr);
  m_Program->setAttributeBuffer(m_VertexAttr, GL_FLOAT, 0, 2);

  m_Vao.release();
  m_Vbo.release();
}

void OpenGLResourceGraph::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);
}

void OpenGLResourceGraph::updateBuffers() {
  if (!m_NeedsBufferUpdate || m_Data.empty()) return;

  std::vector<float> vertices;
  vertices.reserve(m_MaxPoints * 2);

  // X maps from 0.0 to 1.0
  // Y maps from 0.0 to 1.0 (assuming max 100.0)
  for (size_t i = 0; i < m_MaxPoints; ++i) {
    float x = static_cast<float>(i) / static_cast<float>(m_MaxPoints - 1);
    float y = static_cast<float>(m_Data[i]) / 100.0f;
    vertices.push_back(x);
    vertices.push_back(y);
  }

  m_Vbo.bind();
  m_Vbo.write(0, vertices.data(), vertices.size() * sizeof(float));
  m_Vbo.release();

  m_NeedsBufferUpdate = false;
}

void OpenGLResourceGraph::paintGL() {
  glClearColor(0.051f, 0.067f, 0.090f, 1.0f); // #0D1117 roughly
  glClear(GL_COLOR_BUFFER_BIT);

  updateBuffers();

  m_Program->bind();
  m_Vao.bind();

  // Draw grid lines or background? Skipping for brevity

  // Draw the line graph
  m_Program->setUniformValue(m_ColorUniform, QVector4D(0.345f, 0.651f, 1.0f, 1.0f)); // #58A6FF
  
  // Line width (might be ignored by modern core profile, but we try)
  glLineWidth(2.0f);
  glDrawArrays(GL_LINE_STRIP, 0, m_MaxPoints);

  m_Vao.release();
  m_Program->release();
}

} // namespace severance::gui::graphs
