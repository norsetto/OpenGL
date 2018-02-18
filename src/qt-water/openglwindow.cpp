#include "openglwindow.h"

#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QTime>
#include <QTimer>
#include <QtGui>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
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
  QLabel *label10 = new QLabel("Amplitude");
  label10->setStyleSheet("QLabel { color : white; }");

  QSlider *slider1 = new QSlider(Qt::Vertical);
  slider1->setRange(0, 100);
  slider1->setSingleStep(1);
  slider1->setPageStep(10);
  slider1->setTickInterval(10);
  slider1->setTickPosition(QSlider::TicksRight);
  slider1->setValue(40);

  QLabel *label1 = new QLabel("40");
  label1->setStyleSheet("QLabel { color : white; }");
    
  connect(slider1, SIGNAL(valueChanged(int)), this, SLOT(setAmplitude0(int)));
  connect(slider1, &QSlider::valueChanged, label1, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox1 = new QVBoxLayout;
  vbox1->addWidget(label10);
  vbox1->addWidget(slider1);
  vbox1->addWidget(label1);

  QLabel *label11 = new QLabel("Period");
  label11->setStyleSheet("QLabel { color : white; }");

  QSlider *slider5 = new QSlider(Qt::Vertical);
  slider5->setRange(0, 100);
  slider5->setSingleStep(1);
  slider5->setPageStep(10);
  slider5->setTickInterval(10);
  slider5->setTickPosition(QSlider::TicksRight);
  slider5->setValue(40);

  QLabel *label5 = new QLabel("40");
  label5->setStyleSheet("QLabel { color : white; }");
    
  connect(slider5, SIGNAL(valueChanged(int)), this, SLOT(setFrequency0(int)));
  connect(slider5, &QSlider::valueChanged, label5, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox2 = new QVBoxLayout;
  vbox2->addWidget(label11);
  vbox2->addWidget(slider5);
  vbox2->addWidget(label5);

  QHBoxLayout *hbox1 = new QHBoxLayout;
  hbox1->addLayout(vbox1);
  hbox1->addLayout(vbox2);
  
  QGroupBox *wave1 = new QGroupBox(tr("Wave 1"));
  wave1->setLayout(hbox1);
  wave1->setStyleSheet("QGroupBox { color : white; }");

  QLabel *label12 = new QLabel("Amplitude");
  label12->setStyleSheet("QLabel { color : white; }");

  QSlider *slider2 = new QSlider(Qt::Vertical, this);
  slider2->setRange(0, 100);
  slider2->setSingleStep(1);
  slider2->setPageStep(10);
  slider2->setTickInterval(10);
  slider2->setTickPosition(QSlider::TicksRight);
  slider2->setValue(20);

  QLabel *label2 = new QLabel("20", this);
  label2->setStyleSheet("QLabel { color : white; }");
    
  connect(slider2, SIGNAL(valueChanged(int)), this, SLOT(setAmplitude1(int)));
  connect(slider2, &QSlider::valueChanged, label2, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox3 = new QVBoxLayout;
  vbox3->addWidget(label12);
  vbox3->addWidget(slider2);
  vbox3->addWidget(label2);

  QLabel *label13 = new QLabel("Period");
  label13->setStyleSheet("QLabel { color : white; }");

  QSlider *slider6 = new QSlider(Qt::Vertical, this);
  slider6->setRange(0, 100);
  slider6->setSingleStep(1);
  slider6->setPageStep(10);
  slider6->setTickInterval(10);
  slider6->setTickPosition(QSlider::TicksRight);
  slider6->setValue(20);

  QLabel *label6 = new QLabel("20", this);
  label6->setStyleSheet("QLabel { color : white; }");
    
  connect(slider6, SIGNAL(valueChanged(int)), this, SLOT(setFrequency1(int)));
  connect(slider6, &QSlider::valueChanged, label6, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox4 = new QVBoxLayout;
  vbox4->addWidget(label13);
  vbox4->addWidget(slider6);
  vbox4->addWidget(label6);

  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addLayout(vbox3);
  hbox2->addLayout(vbox4);
  
  QGroupBox *wave2 = new QGroupBox(tr("Wave 2"));
  wave2->setLayout(hbox2);
  wave2->setStyleSheet("QGroupBox { color : white; }");

  QLabel *label14 = new QLabel("Amplitude");
  label14->setStyleSheet("QLabel { color : white; }");

  QSlider *slider3 = new QSlider(Qt::Vertical, this);
  slider3->setRange(0, 100);
  slider3->setSingleStep(1);
  slider3->setPageStep(10);
  slider3->setTickInterval(10);
  slider3->setTickPosition(QSlider::TicksRight);
  slider3->setValue(10);

  QLabel *label3 = new QLabel("10", this);
  label3->setStyleSheet("QLabel { color : white; }");
    
  connect(slider3, SIGNAL(valueChanged(int)), this, SLOT(setAmplitude2(int)));
  connect(slider3, &QSlider::valueChanged, label3, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox5 = new QVBoxLayout;
  vbox5->addWidget(label14);
  vbox5->addWidget(slider3);
  vbox5->addWidget(label3);

  QLabel *label15 = new QLabel("Period");
  label15->setStyleSheet("QLabel { color : white; }");

  QSlider *slider7 = new QSlider(Qt::Vertical, this);
  slider7->setRange(0, 100);
  slider7->setSingleStep(1);
  slider7->setPageStep(10);
  slider7->setTickInterval(10);
  slider7->setTickPosition(QSlider::TicksRight);
  slider7->setValue(10);

  QLabel *label7 = new QLabel("10", this);
  label7->setStyleSheet("QLabel { color : white; }");
    
  connect(slider7, SIGNAL(valueChanged(int)), this, SLOT(setFrequency2(int)));
  connect(slider7, &QSlider::valueChanged, label7, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox6 = new QVBoxLayout;
  vbox6->addWidget(label15);
  vbox6->addWidget(slider7);
  vbox6->addWidget(label7);

  QHBoxLayout *hbox3 = new QHBoxLayout;
  hbox3->addLayout(vbox5);
  hbox3->addLayout(vbox6);
  
  QGroupBox *wave3 = new QGroupBox(tr("Wave 3"));
  wave3->setLayout(hbox3);
  wave3->setStyleSheet("QGroupBox { color : white; }");

  QLabel *label16 = new QLabel("Amplitude");
  label16->setStyleSheet("QLabel { color : white; }");

  QSlider *slider4 = new QSlider(Qt::Vertical, this);
  slider4->setRange(0, 100);
  slider4->setSingleStep(1);
  slider4->setPageStep(10);
  slider4->setTickInterval(10);
  slider4->setTickPosition(QSlider::TicksRight);
  slider4->setValue(5);

  QLabel *label4 = new QLabel("5", this);
  label4->setStyleSheet("QLabel { color : white; }");
    
  connect(slider4, SIGNAL(valueChanged(int)), this, SLOT(setAmplitude3(int)));
  connect(slider4, &QSlider::valueChanged, label4, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox7 = new QVBoxLayout;
  vbox7->addWidget(label16);
  vbox7->addWidget(slider4);
  vbox7->addWidget(label4);

  QLabel *label17 = new QLabel("Period");
  label17->setStyleSheet("QLabel { color : white; }");

  QSlider *slider8 = new QSlider(Qt::Vertical, this);
  slider8->setRange(0, 100);
  slider8->setSingleStep(1);
  slider8->setPageStep(10);
  slider8->setTickInterval(10);
  slider8->setTickPosition(QSlider::TicksRight);
  slider8->setValue(5);

  QLabel *label8 = new QLabel("5", this);
  label8->setStyleSheet("QLabel { color : white; }");
    
  connect(slider8, SIGNAL(valueChanged(int)), this, SLOT(setFrequency3(int)));
  connect(slider8, &QSlider::valueChanged, label8, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox8 = new QVBoxLayout;
  vbox8->addWidget(label17);
  vbox8->addWidget(slider8);
  vbox8->addWidget(label8);

  QHBoxLayout *hbox4 = new QHBoxLayout;
  hbox4->addLayout(vbox7);
  hbox4->addLayout(vbox8);
  
  QGroupBox *wave4 = new QGroupBox(tr("Wave 4"));
  wave4->setLayout(hbox4);
  wave4->setStyleSheet("QGroupBox { color : white; }");

  QLabel *label18 = new QLabel("Roughness");
  label18->setStyleSheet("QLabel { color : white; }");

  QSlider *slider9 = new QSlider(Qt::Vertical, this);
  slider9->setRange(0, 100);
  slider9->setSingleStep(1);
  slider9->setPageStep(10);
  slider9->setTickInterval(10);
  slider9->setTickPosition(QSlider::TicksRight);
  slider9->setValue(0);

  QLabel *label9 = new QLabel("0", this);
  label9->setStyleSheet("QLabel { color : white; }");
    
  connect(slider9, SIGNAL(valueChanged(int)), this, SLOT(setRoughness(int)));
  connect(slider9, &QSlider::valueChanged, label9, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox9 = new QVBoxLayout;
  vbox9->addWidget(label18);
  vbox9->addWidget(slider9);
  vbox9->addWidget(label9);

  QPushButton *button1 = new QPushButton(tr("&Wireframe"));
  button1->setCheckable(true);
  button1->setChecked(true);
  connect(button1, SIGNAL(toggled(bool)), this, SLOT(setWireframe(bool)));

  QPushButton *button2 = new QPushButton(tr("&Noise"));
  button2->setCheckable(true);
  button2->setChecked(true);
  connect(button2, SIGNAL(toggled(bool)), this, SLOT(setNoise(bool)));

  QPushButton *button3 = new QPushButton(tr("&Animate"));
  button3->setCheckable(true);
  button3->setChecked(true);
  connect(button3, SIGNAL(toggled(bool)), this, SLOT(setAnimate(bool)));

  QVBoxLayout *vbox10 = new QVBoxLayout;
  vbox10->addWidget(button1);
  vbox10->addWidget(button2);
  vbox10->addWidget(button3);

  QHBoxLayout *hbox5 = new QHBoxLayout;
  hbox5->addLayout(vbox9);
  hbox5->addLayout(vbox10);
  
  QGroupBox *wave5 = new QGroupBox;
  wave5->setLayout(hbox5);

  //NOISE SCALE FACTOR 1
  QLabel *label19 = new QLabel("SF");
  label19->setStyleSheet("QLabel { color : white; }");

  QSlider *slider20 = new QSlider(Qt::Vertical, this);
  slider20->setRange(0, 100);
  slider20->setSingleStep(1);
  slider20->setPageStep(10);
  slider20->setTickInterval(10);
  slider20->setTickPosition(QSlider::TicksRight);
  slider20->setValue(5);

  QLabel *label21 = new QLabel("5", this);
  label21->setStyleSheet("QLabel { color : white; }");
    
  connect(slider20, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor1(int)));
  connect(slider20, &QSlider::valueChanged, label21, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox11 = new QVBoxLayout;
  vbox11->addWidget(label19);
  vbox11->addWidget(slider20);
  vbox11->addWidget(label21);

  //NOISE SCALE FACTOR 21
  QLabel *label22 = new QLabel("X");
  label22->setStyleSheet("QLabel { color : white; }");

  QSlider *slider23 = new QSlider(Qt::Vertical, this);
  slider23->setRange(0, 100);
  slider23->setSingleStep(1);
  slider23->setPageStep(10);
  slider23->setTickInterval(10);
  slider23->setTickPosition(QSlider::TicksRight);
  slider23->setValue(5);

  QLabel *label24 = new QLabel("5", this);
  label24->setStyleSheet("QLabel { color : white; }");
    
  connect(slider23, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor21(int)));
  connect(slider23, &QSlider::valueChanged, label24, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox12 = new QVBoxLayout;
  vbox12->addWidget(label22);
  vbox12->addWidget(slider23);
  vbox12->addWidget(label24);

  //NOISE SCALE FACTOR 22
  QLabel *label25 = new QLabel("Y");
  label25->setStyleSheet("QLabel { color : white; }");

  QSlider *slider26 = new QSlider(Qt::Vertical, this);
  slider26->setRange(0, 100);
  slider26->setSingleStep(1);
  slider26->setPageStep(10);
  slider26->setTickInterval(10);
  slider26->setTickPosition(QSlider::TicksRight);
  slider26->setValue(5);

  QLabel *label27 = new QLabel("5", this);
  label27->setStyleSheet("QLabel { color : white; }");
    
  connect(slider26, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor22(int)));
  connect(slider26, &QSlider::valueChanged, label27, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox13 = new QVBoxLayout;
  vbox13->addWidget(label25);
  vbox13->addWidget(slider26);
  vbox13->addWidget(label27);

  //NOISE SCALE FACTOR 23
  QLabel *label28 = new QLabel("Z");
  label28->setStyleSheet("QLabel { color : white; }");

  QSlider *slider29 = new QSlider(Qt::Vertical, this);
  slider29->setRange(0, 100);
  slider29->setSingleStep(1);
  slider29->setPageStep(10);
  slider29->setTickInterval(10);
  slider29->setTickPosition(QSlider::TicksRight);
  slider29->setValue(5);

  QLabel *label30 = new QLabel("5", this);
  label30->setStyleSheet("QLabel { color : white; }");
    
  connect(slider29, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor23(int)));
  connect(slider29, &QSlider::valueChanged, label30, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox14 = new QVBoxLayout;
  vbox14->addWidget(label28);
  vbox14->addWidget(slider29);
  vbox14->addWidget(label30);

  //NOISE TIME SCALE FACTOR
  QLabel *label31 = new QLabel("Time");
  label31->setStyleSheet("QLabel { color : white; }");

  QSlider *slider32 = new QSlider(Qt::Vertical, this);
  slider32->setRange(0, 100);
  slider32->setSingleStep(1);
  slider32->setPageStep(10);
  slider32->setTickInterval(10);
  slider32->setTickPosition(QSlider::TicksRight);
  slider32->setValue(50);

  QLabel *label33 = new QLabel("50", this);
  label33->setStyleSheet("QLabel { color : white; }");
    
  connect(slider32, SIGNAL(valueChanged(int)), this, SLOT(setNoiseTimeScaleFactor(int)));
  connect(slider32, &QSlider::valueChanged, label33, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox15 = new QVBoxLayout;
  vbox15->addWidget(label31);
  vbox15->addWidget(slider32);
  vbox15->addWidget(label33);

  //GROUP 6
  QHBoxLayout *hbox6 = new QHBoxLayout;
  hbox6->addLayout(vbox11);
  hbox6->addLayout(vbox15);
  hbox6->addLayout(vbox12);
  hbox6->addLayout(vbox13);
  hbox6->addLayout(vbox14);

  QGroupBox *wave6 = new QGroupBox(tr("Noise"));;
  wave6->setLayout(hbox6);
  wave6->setStyleSheet("QGroupBox { color : white; }");

  //NOISE SCALE FACTOR 31
  QLabel *label34 = new QLabel("X");
  label34->setStyleSheet("QLabel { color : white; }");

  QSlider *slider35 = new QSlider(Qt::Vertical, this);
  slider35->setRange(0, 100);
  slider35->setSingleStep(1);
  slider35->setPageStep(10);
  slider35->setTickInterval(10);
  slider35->setTickPosition(QSlider::TicksRight);
  slider35->setValue(5);

  QLabel *label36 = new QLabel("5", this);
  label36->setStyleSheet("QLabel { color : white; }");
    
  connect(slider35, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor31(int)));
  connect(slider35, &QSlider::valueChanged, label36, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox16 = new QVBoxLayout;
  vbox16->addWidget(label34);
  vbox16->addWidget(slider35);
  vbox16->addWidget(label36);

  //NOISE SCALE FACTOR 32
  QLabel *label37 = new QLabel("Y");
  label37->setStyleSheet("QLabel { color : white; }");

  QSlider *slider38 = new QSlider(Qt::Vertical, this);
  slider38->setRange(0, 100);
  slider38->setSingleStep(1);
  slider38->setPageStep(10);
  slider38->setTickInterval(10);
  slider38->setTickPosition(QSlider::TicksRight);
  slider38->setValue(5);

  QLabel *label39 = new QLabel("5", this);
  label39->setStyleSheet("QLabel { color : white; }");
    
  connect(slider38, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor32(int)));
  connect(slider38, &QSlider::valueChanged, label39, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox17 = new QVBoxLayout;
  vbox17->addWidget(label37);
  vbox17->addWidget(slider38);
  vbox17->addWidget(label39);

  //NOISE SCALE FACTOR 33
  QLabel *label40 = new QLabel("Z");
  label40->setStyleSheet("QLabel { color : white; }");

  QSlider *slider41 = new QSlider(Qt::Vertical, this);
  slider41->setRange(0, 100);
  slider41->setSingleStep(1);
  slider41->setPageStep(10);
  slider41->setTickInterval(10);
  slider41->setTickPosition(QSlider::TicksRight);
  slider41->setValue(5);

  QLabel *label42 = new QLabel("5", this);
  label42->setStyleSheet("QLabel { color : white; }");
    
  connect(slider41, SIGNAL(valueChanged(int)), this, SLOT(setNoiseScaleFactor33(int)));
  connect(slider41, &QSlider::valueChanged, label42, 
	  static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QVBoxLayout *vbox18 = new QVBoxLayout;
  vbox18->addWidget(label40);
  vbox18->addWidget(slider41);
  vbox18->addWidget(label42);

  //GROUP 7
  QHBoxLayout *hbox7 = new QHBoxLayout;
  hbox7->addLayout(vbox16);
  hbox7->addLayout(vbox17);
  hbox7->addLayout(vbox18);

  QGroupBox *wave7 = new QGroupBox(tr("Normal"));;
  wave7->setLayout(hbox7);
  wave7->setStyleSheet("QGroupBox { color : white; }");

  //MAIN GRID
  QGridLayout *mainLayout = new QGridLayout(this);
  mainLayout->addWidget(wave1, 0, 0);
  mainLayout->addWidget(wave2, 0, 1);
  mainLayout->addWidget(wave3, 1, 0);
  mainLayout->addWidget(wave4, 1, 1);
  mainLayout->addWidget(wave5, 0, 2);
  mainLayout->addWidget(wave6, 1, 2);
  mainLayout->addWidget(wave7, 0, 3);
  setLayout(mainLayout);
    
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
  setRoughness(10);

  for (int index = 0; index < NUM_WAVES; index++)
    {
      m_wave_data.D[index] = glm::vec2(0.7071f, 0.7071f);
    }

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
  case Qt::Key_Escape:
    close();
    break;
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
  case Qt::Key_P:
    qDebug() << glm::to_string(m_camera->get_position()).c_str() << glm::to_string(m_camera->get_direction()).c_str();
    break;
  case Qt::Key_G:
    wireframe = !wireframe;
    break;
  }
}
