#include "config.h"
#include "main.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "object.hpp"

#include <image.hpp>

#include <vector>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_inverse.hpp> //glm::inverse

class test_app : public Application {

public:
  test_app() {}
  
protected:
  void onKey(int key, int action, int modifier) {
    
    if (action) {
      switch (key) {
      case GLFW_KEY_ESCAPE:
	glfwSetWindowShouldClose(window, GL_TRUE);
	break;
      case 'W':
	camera->move_forward((float)deltaTime);
	break;
      case 'S':
	camera->move_backward((float)deltaTime);
	break;
      case 'D':
	camera->move_right((float)deltaTime);
	break;
      case 'A':
	camera->move_left((float)deltaTime);
	break;
      case 'Q':
	camera->move_up((float)deltaTime);
	break;
      case 'Z': 
	camera->move_down((float)deltaTime);
	break;
      case 'F':
	calcFps = !calcFps;
	frame = 0;
	lastFPSTime = 0.0;
	break;
      case 'G': wireframe = !wireframe;
	if (wireframe)
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;
      case GLFW_KEY_UP:	(eLit < 3.0f ? eLit += sLit: eLit);
	sinE = sinf(eLit);
	cosE = cosf(eLit);
	break;
      case GLFW_KEY_DOWN: (eLit > 0.0f ? eLit -= sLit: eLit);
	sinE = sinf(eLit);
	cosE = cosf(eLit);
	break;
      case GLFW_KEY_LEFT: aLit += sLit;
	sinA = sinf(aLit);
	cosA = cosf(aLit);
	break;
      case GLFW_KEY_RIGHT: aLit -= sLit;
	sinA = sinf(aLit);
	cosA = cosf(aLit);
	break;
      case GLFW_KEY_RIGHT_BRACKET: camera->change_speed(1.1f);
	break;
      case GLFW_KEY_LEFT_BRACKET: camera->change_speed(1.0f / 1.1f);
	break;
      }
    }
  }

    void resize(int width, int height)
  {
    if (height > 0) {
      info.windowWidth = width;
      info.windowHeight = height;
      info.aspect = width / float(height);
    }
    camera->set_proj_matrix(info.aspect, 0.1f, 100.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
  }

  void onMouseWheel(double pos) {
    camera->change_fov (0.02f * (float)pos);
  }

  void onMouseButton(int button, int action) {
    glfwGetCursorPos(info.window, &mouse.xpos, &mouse.ypos);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
      {
	mouse.pressed_left = !mouse.pressed_left;
      } else if (button == GLFW_MOUSE_BUTTON_RIGHT)
      {
	mouse.pressed_right = !mouse.pressed_right;
      }
  }

  virtual void init() {
    Application::init();
    info.samples = 8;
    info.windowWidth = 800;
    info.windowHeight = 600;
    info.show_cursor = true;
  }
  
  virtual void startup() {
    fprintf(stdout,"%s Version %s - %s build\n",
            info.argv[0],
            VERSION,
	    BUILD_TYPE);
      
    GLuint shader[2];
    bool check_errors = false;
#ifdef GL_DEBUG
    check_errors = true;
#endif

    shader[0] = shader::load(SHADERS_LOCATION "model/model.vs.glsl", GL_VERTEX_SHADER, check_errors);
    shader[1] = shader::load(SHADERS_LOCATION "model/model.fs.glsl", GL_FRAGMENT_SHADER, check_errors);

    objProgram = program::link_from_shaders(shader, 2, true, check_errors);

    glGenBuffers(1, &matrices_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices_block), NULL, GL_DYNAMIC_DRAW);
    
    object.load(MODELS_LOCATION "kila.dae", true, true);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_CULL_FACE);
  
    camera = new Camera(glm::vec3(0.0f, 0.5f, object.max().z*object.scale() + 2.0f));
    camera->set_speed(2.f);
    camera->set_proj_matrix(info.aspect, 0.1f, 100.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);

    sinE = sinf(eLit);
    cosE = cosf(eLit);
    sinA = sinf(aLit);
    cosA = cosf(aLit);
  }

virtual void render(double currentTime) {
      
  static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
  static const GLfloat one  = 1.0f;

  if (mouse.pressed_right)
    {
      double xpos, ypos;
      glfwGetCursorPos(info.window, &xpos, &ypos);
      camera->rotate((float)(xpos - mouse.xpos), (float)(ypos - mouse.ypos));
      mouse.xpos = xpos;
      mouse.ypos = ypos;
    } else if (mouse.pressed_left)
    {
      double xpos, ypos;
      glfwGetCursorPos(info.window, &xpos, &ypos);
      Model.yaw += (float)(xpos - mouse.xpos)*Model.rate;
      Model.pitch += (float)(ypos - mouse.ypos)*Model.rate;
      if (Model.pitch >= 1.57) Model.pitch = 1.57;
      if (Model.pitch <= -1.57) Model.pitch = -1.57;
      mouse.xpos = xpos;
      mouse.ypos = ypos;
    }

  camera->update();

  glm::mat4 model_matrix = glm::rotate(glm::mat4(1.0), Model.yaw, glm::vec3(0.0f, -1.0f, 0.0f));
  model_matrix = glm::rotate(model_matrix, Model.pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
  model_matrix = glm::scale(model_matrix, glm::vec3(object.scale()));

  glClearBufferfv(GL_COLOR, 0, green);
  glClearBufferfv(GL_DEPTH, 0, &one);

  light_pos = lightRadius * glm::vec3(cosE * sinA, sinE, cosE * cosA);
	    
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_buffer);
  block = (matrices_block *)glMapBufferRange(GL_UNIFORM_BUFFER,
					     0,
					     sizeof(matrices_block),
					     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  block->model_matrix = model_matrix;
  block->view_matrix  = camera->get_view_matrix();
  block->proj_matrix  = camera->get_proj_matrix();
  block->light_pos    = light_pos;
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  glUseProgram(objProgram);
  object.render();

  if (calcFps) {
    frame++;
    double deltaFPSTime = currentTime-lastFPSTime;
    if (deltaFPSTime > 0.25) {
      double fps = ((double)frame)/deltaFPSTime;
      printf("\r%7.2f FPS", fps);
      fflush(stdout);
      lastFPSTime = currentTime;
      frame = 0;
    }
   }

  deltaTime = currentTime-lastTime;
  lastTime = currentTime;
}

virtual void shutdown() {
  glDeleteProgram(objProgram);
  object.free();
}

private:
  double lastFPSTime;
  double lastTime = 0.0;
  double deltaTime;
  unsigned long frame;
  bool calcFps = false;
  bool wireframe = false;

  glm::vec3 light_pos;
  float lightRadius = 100.0f, eLit = 0.0f, aLit = 0.7854f, sLit = 0.01f;
  float sinE, cosE, sinA, cosA;
  
  struct
  {
    bool pressed_left = false;
    bool pressed_right = false;
    double xpos;
    double ypos;
  } mouse;

  GLuint objProgram;
  struct matrices_block
  {
    glm::mat4 model_matrix;
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::vec3 light_pos;
  };
  matrices_block * block;
  GLuint matrices_buffer;

  Object object;
  Camera *camera;

  struct {
    GLfloat pitch = 0.0f;
    GLfloat yaw   = 0.0f;
    GLfloat rate  = 0.005f;
  } Model;

};

Application * Application::app = 0;

DECLARE_MAIN(test_app)
