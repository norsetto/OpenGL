#include "openglwindow.h"

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#ifdef GL_DEBUG
#include <QDebug>
#include <QOpenGLDebugLogger>
#endif

#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
#include <QVector3D>

#include "image.hpp"
#include "camera.hpp"

MyGLWidget::MyGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
  this->setFocusPolicy(Qt::ClickFocus);
  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
  timer.setInterval(0);
  timer.start();
}

#ifdef GL_DEBUG
void MyGLWidget::onMessageLogged(const QOpenGLDebugMessage &msg)
{
  QString error;
 
  // Format based on severity
  switch (msg.severity())
    {
    case QOpenGLDebugMessage::AnySeverity:
      error += "<A>";
      break;
    case QOpenGLDebugMessage::InvalidSeverity:
      error += "<I>";
      break;
    case QOpenGLDebugMessage::NotificationSeverity:
      error += "<N>";
      break;
    case QOpenGLDebugMessage::HighSeverity:
      error += "<!!!>";
      break;
    case QOpenGLDebugMessage::MediumSeverity:
      error += "<!!>";
      break;
    case QOpenGLDebugMessage::LowSeverity:
      error += "<!>";
      break;
    }
 
  error += " (";
 
  // Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
  switch (msg.source())
    {
      CASE(AnySource);
      CASE(APISource);
      CASE(WindowSystemSource);
      CASE(ShaderCompilerSource);
      CASE(ThirdPartySource);
      CASE(ApplicationSource);
      CASE(OtherSource);
      CASE(InvalidSource);
    }
#undef CASE
 
  error += " : ";
 
  // Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
  switch (msg.type())
    {
      CASE(InvalidType);
      CASE(AnyType);
      CASE(ErrorType);
      CASE(DeprecatedBehaviorType);
      CASE(UndefinedBehaviorType);
      CASE(PortabilityType);
      CASE(PerformanceType);
      CASE(OtherType);
      CASE(MarkerType);
      CASE(GroupPushType);
      CASE(GroupPopType);
    }
#undef CASE
 
  error += ")";
  qDebug() << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
}
#endif

void  MyGLWidget::initializeGL()
{
  initializeOpenGLFunctions();
#ifdef GL_DEBUG
  QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
  if(logger->initialize()) {
    qDebug() << "GL_DEBUG Debug Logger" << logger << "\n";
    connect(logger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(onMessageLogged(const QOpenGLDebugMessage&)), Qt::DirectConnection);
    logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    logger->enableMessages();
  }
#endif    
  m_camera = new Camera(glm::vec3(42.0f, 17.0f, 42.0f));
  m_camera->set_direction(glm::vec2(-0.475f, -0.7668f));
  m_camera->set_speed(0.01f);

  glClearColor(0.7f, 1.0f, 1.0f, 1.0f);

  setNoiseTimeScaleFactor(50);
  setNoiseScaleFactor1(5);
  setNoiseScaleFactor21(5);
  setNoiseScaleFactor22(5);
  setNoiseScaleFactor23(5);
  setNoiseScaleFactor31(5);
  setNoiseScaleFactor32(5);
  setNoiseScaleFactor33(5);

  m_program = new QOpenGLShaderProgram(this);
  m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "water.vs.glsl");
  m_program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, "water.tcs.glsl");
  m_program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, "water.tes.glsl");
  m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "water.fs.glsl");
  m_program->link();
  m_program->bind();
  m_uniform.grid                = m_program->uniformLocation("grid");
  m_uniform.max_distance        = m_program->uniformLocation("max_distance");
  m_uniform.water_size          = m_program->uniformLocation("water_size");
  m_uniform.viewport            = m_program->uniformLocation("viewport");
  m_uniform.patch_size          = m_program->uniformLocation("patch_size");
  m_uniform.noise_time_scale_factor = m_program->uniformLocation("noise_time_scale_factor");
  m_uniform.noise_scale_factor1     = m_program->uniformLocation("noise_scale_factor1");
  m_uniform.noise_scale_factor2     = m_program->uniformLocation("noise_scale_factor2");
  m_uniform.noise_scale_factor3     = m_program->uniformLocation("noise_scale_factor3");
  m_program->setUniformValue(m_uniform.grid,           4);
  m_program->setUniformValue(m_uniform.max_distance, 160.0f);
  m_program->setUniformValue(m_uniform.water_size,    64.0f);
  m_program->setUniformValue(m_uniform.patch_size,   128);
  m_program->setUniformValue(m_uniform.noise_time_scale_factor, noise_time_scale_factor);
  m_program->setUniformValue(m_uniform.noise_scale_factor1, noise_scale_factor1);
  m_program->setUniformValue(m_uniform.noise_scale_factor2, noise_scale_factor2);
  m_program->setUniformValue(m_uniform.noise_scale_factor3, noise_scale_factor3);
  m_program->setPatchVertexCount(4);
  m_program->release();

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &m_texture_id);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  Image water_texture("water.png");

  glTexImage2D(GL_TEXTURE_2D, 0,
	       water_texture.internalformat(),
	       water_texture.width(),
	       water_texture.height(),
	       0,
	       water_texture.format(),
	       GL_UNSIGNED_BYTE,
	       water_texture.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  setAmplitude0(40);
  setAmplitude1(20);
  setAmplitude2(10);
  setAmplitude3(5);
  setFrequency0(40);
  setFrequency1(20);
  setFrequency2(10);
  setFrequency3(5);
  setRoughness(20);
  setDirection0(45);
  setDirection1(225);
  setDirection2(135);
  setDirection3(315);

  m_quad_vao = new QOpenGLVertexArrayObject;
  m_quad_vao->create();

  glGenBuffers(1, &m_matrices_buffer);
  glBindBuffer(GL_UNIFORM_BUFFER, m_matrices_buffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(MATRICES_BLOCK), NULL, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_matrices_buffer);
  m_block = (MATRICES_BLOCK *)glMapBufferRange(GL_UNIFORM_BUFFER,
					       0,
					       sizeof(MATRICES_BLOCK),
					       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);

  sinE = sinf(eLit);
  cosE = cosf(eLit);
  sinA = sinf(aLit);
  cosA = cosf(aLit);

  t.start();
}

void  MyGLWidget::resizeGL(int w, int h)
{
  m_camera->set_proj_matrix(w / float(h), 0.001f, 320.0f);
  m_program->bind();
  m_program->setUniformValue(m_uniform.viewport, float(w), float(h));
  m_program->release();
}

void  MyGLWidget::paintGL()
{
  if (animate)
    delta_t = float(t.elapsed())/1000.0f;
    
  glClear(GL_COLOR_BUFFER_BIT |
	  GL_DEPTH_BUFFER_BIT |
	  GL_STENCIL_BUFFER_BIT);

  m_light_pos = glm::vec4(lightRadius * glm::vec3(cosE * sinA, sinE, cosE * cosA), delta_t);
  m_camera->update();

  m_block->view_matrix  = m_camera->get_view_matrix();
  m_block->proj_matrix  = m_camera->get_proj_matrix();
  m_block->vp_matrix    = m_camera->get_vp_matrix();
  m_block->light_pos    = m_light_pos;
  std::memcpy(&(m_block->wave_data), &m_wave_data, sizeof(WAVE_DATA));

  m_program->bind();
  m_quad_vao->bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);
  
  if (wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  if (noise)
    {
      m_program->setUniformValue(m_uniform.noise_time_scale_factor, noise_time_scale_factor);
      m_program->setUniformValue(m_uniform.noise_scale_factor1, noise_scale_factor1);
      m_program->setUniformValue(m_uniform.noise_scale_factor2, noise_scale_factor2);
      m_program->setUniformValue(m_uniform.noise_scale_factor3, noise_scale_factor3);
    } else
    {
      m_program->setUniformValue(m_uniform.noise_scale_factor1, 0.0f);
      m_program->setUniformValue(m_uniform.noise_scale_factor2, QVector3D(0.0f, 0.0f, 0.0f));
      m_program->setUniformValue(m_uniform.noise_scale_factor3, QVector3D(0.0f, 0.0f, 0.0f));
    }
    
  glDrawArraysInstanced(GL_PATCHES, 0, 4, 16384);
  m_quad_vao->release();
  m_program->release();
}

void MyGLWidget::setQ(const int &index, const float &value)
{
  if (value > 0.0f)
    m_wave_data.Q[index] = m_roughness / value;
}

void  MyGLWidget::setAmplitude0(int value)
{
  m_wave_data.amplitude[0] = float(value) / 100.0f;
  setQ(0, m_wave_data.frequency[0] * m_wave_data.amplitude[0]);
}

void  MyGLWidget::setAmplitude1(int value)
{
  m_wave_data.amplitude[1] = float(value) / 100.0f;
  setQ(1, m_wave_data.frequency[1] * m_wave_data.amplitude[1]);
}

void  MyGLWidget::setAmplitude2(int value)
{
  m_wave_data.amplitude[2] = float(value) / 100.0f;
  setQ(2, m_wave_data.frequency[2] * m_wave_data.amplitude[2]);
}

void  MyGLWidget::setAmplitude3(int value)
{
  m_wave_data.amplitude[3] = float(value) / 100.0f;
  setQ(3, m_wave_data.frequency[3] * m_wave_data.amplitude[3]);
}

void  MyGLWidget::setFrequency0(int value)
{
  m_wave_data.frequency[0] = 6.28318530718f / (10.0f + float(value) / 10.0f);
  m_wave_data.phase[0]     = std::sqrt(10.0f * m_wave_data.frequency[0]);
  setQ(0, m_wave_data.frequency[0] * m_wave_data.amplitude[0]);
}

void  MyGLWidget::setFrequency1(int value)
{
  m_wave_data.frequency[1] = 6.28318530718f / (10.0f + float(value) / 10.0f);
  m_wave_data.phase[1]     = std::sqrt(10.0f * m_wave_data.frequency[1]);
  setQ(1, m_wave_data.frequency[1] * m_wave_data.amplitude[1]);
}

void  MyGLWidget::setFrequency2(int value)
{
  m_wave_data.frequency[2] = 6.28318530718f / (10.0f + float(value) / 10.0f);
  m_wave_data.phase[2]     = std::sqrt(10.0f * m_wave_data.frequency[2]);
  setQ(2, m_wave_data.frequency[2] * m_wave_data.amplitude[2]);
}

void  MyGLWidget::setFrequency3(int value)
{
  m_wave_data.frequency[3] = 6.28318530718f / (10.0f + float(value) / 10.0f);
  m_wave_data.phase[3]     = std::sqrt(10.0f * m_wave_data.frequency[3]);
  setQ(3, m_wave_data.frequency[3] * m_wave_data.amplitude[3]);
}

void MyGLWidget::setRoughness(int value)
{
  m_roughness = float(value) / 100.0f;
  for (int index = 0; index < NUM_WAVES; index++)
    {
      setQ(index, m_wave_data.frequency[index] * m_wave_data.amplitude[index]);
    }
}

void  MyGLWidget::setDirection0(int value)
{
  float wave_angle = float(value) * deg2rad;
  m_wave_data.D[0] = glm::vec2(std::cos(wave_angle), std::sin(wave_angle));
}

void  MyGLWidget::setDirection1(int value)
{
  float wave_angle = float(value) * deg2rad;
  m_wave_data.D[1] = glm::vec2(std::cos(wave_angle), std::sin(wave_angle));
}

void  MyGLWidget::setDirection2(int value)
{
  float wave_angle = float(value) * deg2rad;
  m_wave_data.D[2] = glm::vec2(std::cos(wave_angle), std::sin(wave_angle));
}

void  MyGLWidget::setDirection3(int value)
{
  float wave_angle = float(value) * deg2rad;
  m_wave_data.D[3] = glm::vec2(std::cos(wave_angle), std::sin(wave_angle));
}

void MyGLWidget::setNoiseTimeScaleFactor(int value)
{
  noise_time_scale_factor = float(value) / 50.0f;
}

void MyGLWidget::setNoiseScaleFactor1(int value)
{
  noise_scale_factor1 = float(value) / 100.0f;
}

void MyGLWidget::setNoiseScaleFactor21(int value)
{
  noise_scale_factor2.setX(float(value) / 1000.0f);
}

void MyGLWidget::setNoiseScaleFactor22(int value)
{
  noise_scale_factor2.setY(float(value) / 1000.0f);
}

void MyGLWidget::setNoiseScaleFactor23(int value)
{
  noise_scale_factor2.setZ(float(value) / 1000.0f);
}

void MyGLWidget::setNoiseScaleFactor31(int value)
{
  noise_scale_factor3.setX(float(value) / 1000.0f);
}

void MyGLWidget::setNoiseScaleFactor32(int value)
{
  noise_scale_factor3.setY(float(value) / 1000.0f);
}

void MyGLWidget::setNoiseScaleFactor33(int value)
{
  noise_scale_factor3.setZ(float(value) / 1000.0f);
}

void MyGLWidget::setWireframe(bool value)
{
  wireframe = value;
}

void MyGLWidget::setNoise(bool value)
{
  noise = value;
}

void MyGLWidget::setAnimate(bool value)
{
  animate = value;
}

void  MyGLWidget::mousePressEvent(QMouseEvent *event)
{
  lastPos = event->pos();
}

void  MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
  int dx = event->x() - lastPos.x();
  int dy = event->y() - lastPos.y();

  if (event->buttons() & Qt::RightButton) {
    m_camera->rotate(float(dx), float(dy));
  }

  lastPos = event->pos();
}

void  MyGLWidget::keyPressEvent(QKeyEvent *e)
{
  switch(e->key()) {
  case Qt::Key_W:
    m_camera->move_forward(delta_t);
    break;
  case Qt::Key_S:
    m_camera->move_backward(delta_t);
    break;
  case Qt::Key_D:
    m_camera->move_right(delta_t);
    break;
  case Qt::Key_A:
    m_camera->move_left(delta_t);
    break;
  case Qt::Key_Q:
    m_camera->move_up(delta_t);
    break;
  case Qt::Key_Z:
    m_camera->move_down(delta_t);
    break;
#ifdef GL_DEBUG
  case Qt::Key_P:
    qDebug() << glm::to_string(m_camera->get_position()).c_str() << glm::to_string(m_camera->get_direction()).c_str();
    break;
#endif
  default:
    QWidget::keyPressEvent(e);
  }
}
