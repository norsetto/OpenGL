#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#ifdef GL_DEBUG
#include <QDebug>
#include <QOpenGLDebugLogger>
#endif
#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QTime>
#include <QTimer>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "camera.hpp"

#define NUM_WAVES 4

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
  Q_OBJECT
  
 public:
  
  MyGLWidget(QWidget *parent = 0);
#ifdef GL_DEBUG
 protected slots:
  
   void onMessageLogged(const QOpenGLDebugMessage &msg);
#endif
 protected:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    
 protected slots:

   void setQ(const int &index, const float &value);
   void setAmplitude0(int value);
   void setAmplitude1(int value);
   void setAmplitude2(int value);
   void setAmplitude3(int value);
   void setFrequency0(int value);
   void setFrequency1(int value);
   void setFrequency2(int value);
   void setFrequency3(int value);
   void setRoughness (int value);
   void setDirection0(int value);
   void setDirection1(int value);
   void setDirection2(int value);
   void setDirection3(int value);

   void setNoiseTimeScaleFactor(int value);
   void setNoiseScaleFactor1(int value);
   void setNoiseScaleFactor21(int value);
   void setNoiseScaleFactor22(int value);
   void setNoiseScaleFactor23(int value);
   void setNoiseScaleFactor31(int value);
   void setNoiseScaleFactor32(int value);
   void setNoiseScaleFactor33(int value);
   
   void setWireframe (bool value);
   void setNoise (bool value);
   void setAnimate (bool value);

 protected:

      void mousePressEvent(QMouseEvent *event);
      void mouseMoveEvent(QMouseEvent *event);
      void keyPressEvent(QKeyEvent *e);

 private:
      
      Camera *m_camera = nullptr;
      QOpenGLShaderProgram *m_program = nullptr;
      struct UNIFORM {
	GLint grid;
	GLint max_distance;
	GLint water_size;
	GLint viewport;
	GLint patch_size;
	GLint noise_time_scale_factor;
	GLint noise_scale_factor1;
	GLint noise_scale_factor2;
	GLint noise_scale_factor3;
      } m_uniform;
      QOpenGLVertexArrayObject *m_quad_vao;
      struct WAVE_DATA
      {
	float amplitude[NUM_WAVES];
	float frequency[NUM_WAVES];
	float phase[NUM_WAVES];
	float Q[NUM_WAVES];
	glm::vec2 D[NUM_WAVES];
      } m_wave_data;
      float m_roughness;
      float noise_time_scale_factor = 1.0f;
      float noise_scale_factor1 = 0.05f;
      QVector3D noise_scale_factor2 = QVector3D(0.1f, 0.1f, 0.1f);
      QVector3D noise_scale_factor3 = QVector3D(0.075f, 0.075f, 0.075f);
      struct MATRICES_BLOCK
      {
	glm::mat4 view_matrix;
	glm::mat4 proj_matrix;
	glm::mat4 vp_matrix;
	glm::vec4 light_pos;
	WAVE_DATA wave_data;
      };
      GLuint m_matrices_buffer;
      MATRICES_BLOCK *m_block;
      glm::vec4 m_light_pos;
      float lightRadius = 100.0f, eLit = 0.1f, aLit = 0.7854f, sLit = 0.01f;
      float sinE, cosE, sinA, cosA;
      QTime t;
      QTimer timer;
      float delta_t;
      QPoint lastPos;
      bool wireframe = true;
      bool noise = true;
      bool animate = true;
      GLuint m_texture_id;
};
