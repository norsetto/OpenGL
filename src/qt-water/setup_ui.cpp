#include "openglwindow.h"

#include <QtGui>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

void MyGLWidget::setup_ui(void)
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
}
