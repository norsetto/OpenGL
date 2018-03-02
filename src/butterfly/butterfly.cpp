/*
 *
 */


#include <glm/glm.hpp>
#include <cstdlib>
#include <vector>
#include <time.h>

#include "config.h"
#include "main.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"

class test_app : public Application {

public:
  test_app() :	calcFps(false)
  {}
  
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
      case 'B':
	butterfly_end = !butterfly_end;
	break;
      case 'F':
	calcFps = !calcFps;
	frame = 0;
	lastFPSTime = 0.0;
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
    camera->set_proj_matrix(info.aspect, 0.001f, 1000.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
  }

  void onMouseWheel(double pos) {
    camera->change_fov (0.02f * (float)pos);
  }

  void onMouseButton(int button, int action) {
      if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
	  glfwGetCursorPos(info.window, &mouse.xpos, &mouse.ypos);
	  mouse.pressed_left = !mouse.pressed_left;
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

    GLuint shader[3];

    shader[0] = shader::load(SHADERS_LOCATION "butterfly/butterfly.vs.glsl", GL_VERTEX_SHADER);
    shader[1] = shader::load(SHADERS_LOCATION "butterfly/butterfly.gs.glsl", GL_GEOMETRY_SHADER);
    shader[2] = shader::load(SHADERS_LOCATION "butterfly/butterfly.fs.glsl", GL_FRAGMENT_SHADER);

    butterflyProgram = program::link_from_shaders(shader, 3, true);

    Image butterfly_image(butterfly_texture, DATA_LOCATION "butterfly.png");

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    srand (static_cast <unsigned> (time(0)));

    num_butterflies = 156;
    butterfly = new BUTTERFLY[num_butterflies];
    timer     = new float[num_butterflies];
    final     = new glm::vec3[num_butterflies];
    next      = new glm::vec3[num_butterflies];
    
    for (int i = 0; i < num_butterflies; i++) {
      butterfly[i].position = glm::vec3(-5.f + (float)rand() * 10.f / (float)RAND_MAX, -5.f + (float)rand() * 10.f / (float)RAND_MAX, -5.f + (float)rand() * 10.f / (float)RAND_MAX);
      butterfly[i].properties = glm::vec3(1.25f + (float)rand() * 0.5f / (float)RAND_MAX,
					  (float)rand() * 0.5f / (float)RAND_MAX,
					  0.f);
      timer[i] = 0.f;
      next[i] = glm::vec3(0.f, 0.f, 0.f);
    }

    // T
    final[  0] = glm::vec3(-6.0f, -4.0f, 0.f);
    final[  1] = glm::vec3(-6.0f, -3.5f, 0.f);
    final[  2] = glm::vec3(-6.0f, -3.0f, 0.f);
    final[  3] = glm::vec3(-6.0f, -2.5f, 0.f);
    final[  4] = glm::vec3(-6.0f, -2.0f, 0.f);
    final[  5] = glm::vec3(-6.0f, -1.5f, 0.f);
    final[  6] = glm::vec3(-6.0f, -1.0f, 0.f);
    final[  7] = glm::vec3(-6.0f, -0.5f, 0.f);
    final[  8] = glm::vec3(-6.0f,  0.0f, 0.f);
    final[  9] = glm::vec3(-6.0f,  0.5f, 0.f);
    final[ 10] = glm::vec3(-6.0f,  1.0f, 0.f);
    final[ 11] = glm::vec3(-6.0f,  1.5f, 0.f);
    final[ 12] = glm::vec3(-6.0f,  2.0f, 0.f);
    final[ 13] = glm::vec3(-6.0f,  2.5f, 0.f);
    final[ 14] = glm::vec3(-6.0f,  3.0f, 0.f);
    final[ 15] = glm::vec3(-8.5f,  3.5f, 0.f);
    final[ 16] = glm::vec3(-8.0f,  3.5f, 0.f);
    final[ 17] = glm::vec3(-7.5f,  3.5f, 0.f);
    final[ 18] = glm::vec3(-7.0f,  3.5f, 0.f);
    final[ 19] = glm::vec3(-6.5f,  3.5f, 0.f);
    final[ 20] = glm::vec3(-6.0f,  3.5f, 0.f);
    final[ 21] = glm::vec3(-5.5f,  3.5f, 0.f);
    final[ 22] = glm::vec3(-5.0f,  3.5f, 0.f);
    final[ 23] = glm::vec3(-4.5f,  3.5f, 0.f);
    final[ 24] = glm::vec3(-4.0f,  3.5f, 0.f);
    final[ 25] = glm::vec3(-3.5f,  3.5f, 0.f);

    // I
    final[ 26] = glm::vec3(-1.5f,  3.5f, 0.f);
    final[ 27] = glm::vec3(-1.5f,  3.0f, 0.f);
    final[ 28] = glm::vec3(-1.5f,  2.5f, 0.f);
    final[ 29] = glm::vec3(-1.5f,  2.0f, 0.f);
    final[ 30] = glm::vec3(-1.5f,  1.5f, 0.f);
    final[ 31] = glm::vec3(-1.5f,  1.0f, 0.f);
    final[ 32] = glm::vec3(-1.5f,  0.5f, 0.f);
    final[ 33] = glm::vec3(-1.5f,  0.0f, 0.f);
    final[ 34] = glm::vec3(-1.5f, -0.5f, 0.f);
    final[ 35] = glm::vec3(-1.5f, -1.0f, 0.f);
    final[ 36] = glm::vec3(-1.5f, -1.5f, 0.f);
    final[ 37] = glm::vec3(-1.5f, -2.0f, 0.f);
    final[ 38] = glm::vec3(-1.5f, -2.5f, 0.f);
    final[ 39] = glm::vec3(-1.5f, -3.0f, 0.f);
    final[ 40] = glm::vec3(-1.5f, -3.5f, 0.f);
    final[ 41] = glm::vec3(-1.5f, -4.0f, 0.f);

    // A
    final[ 42] = glm::vec3( 0.5f, -4.0f, 0.f);
    final[ 43] = glm::vec3( 0.5f, -3.5f, 0.f);
    final[ 44] = glm::vec3( 0.5f, -3.0f, 0.f);
    final[ 45] = glm::vec3( 0.5f, -2.5f, 0.f);
    final[ 46] = glm::vec3( 0.5f, -2.0f, 0.f);
    final[ 47] = glm::vec3( 0.5f, -1.5f, 0.f);
    final[ 48] = glm::vec3( 0.5f, -1.0f, 0.f);
    final[ 49] = glm::vec3( 0.5f, -0.5f, 0.f);
    final[ 50] = glm::vec3( 0.5f,  0.0f, 0.f);
    final[ 51] = glm::vec3( 0.5f,  0.5f, 0.f);
    final[ 52] = glm::vec3( 0.5f,  1.0f, 0.f);
    final[ 53] = glm::vec3( 0.5f,  1.5f, 0.f);
    final[ 54] = glm::vec3( 0.5f,  2.0f, 0.f);
    final[ 55] = glm::vec3( 0.5f,  2.5f, 0.f);
    final[ 56] = glm::vec3( 0.5f,  3.0f, 0.f);
    final[ 57] = glm::vec3( 1.0f,  3.5f, 0.f);
    final[ 58] = glm::vec3( 1.0f, -1.0f, 0.f);
    final[ 59] = glm::vec3( 1.0f, -1.5f, 0.f);
    final[ 60] = glm::vec3( 1.5f,  3.5f, 0.f);
    final[ 61] = glm::vec3( 1.5f, -1.0f, 0.f);
    final[ 62] = glm::vec3( 1.5f, -1.5f, 0.f);
    final[ 63] = glm::vec3( 2.0f,  3.5f, 0.f);
    final[ 64] = glm::vec3( 2.0f, -1.0f, 0.f);
    final[ 65] = glm::vec3( 2.0f, -1.5f, 0.f);
    final[ 66] = glm::vec3( 2.5f, -4.0f, 0.f);
    final[ 67] = glm::vec3( 2.5f, -3.5f, 0.f);
    final[ 68] = glm::vec3( 2.5f, -3.0f, 0.f);
    final[ 69] = glm::vec3( 2.5f, -2.5f, 0.f);
    final[ 70] = glm::vec3( 2.5f, -2.0f, 0.f);
    final[ 71] = glm::vec3( 2.5f, -1.5f, 0.f);
    final[ 72] = glm::vec3( 2.5f, -1.0f, 0.f);
    final[ 73] = glm::vec3( 2.5f, -0.5f, 0.f);
    final[ 74] = glm::vec3( 2.5f,  0.0f, 0.f);
    final[ 75] = glm::vec3( 2.5f,  0.5f, 0.f);
    final[ 76] = glm::vec3( 2.5f,  1.0f, 0.f);
    final[ 77] = glm::vec3( 2.5f,  1.5f, 0.f);
    final[ 78] = glm::vec3( 2.5f,  2.0f, 0.f);
    final[ 79] = glm::vec3( 2.5f,  2.5f, 0.f);
    final[ 80] = glm::vec3( 2.5f,  3.0f, 0.f);

    // M
    final[ 81] = glm::vec3( 4.5f,  3.5f, 0.f);
    final[ 82] = glm::vec3( 4.5f,  3.0f, 0.f);
    final[ 83] = glm::vec3( 4.5f,  2.5f, 0.f);
    final[ 84] = glm::vec3( 4.5f,  2.0f, 0.f);
    final[ 85] = glm::vec3( 4.5f,  1.5f, 0.f);
    final[ 86] = glm::vec3( 4.5f,  1.0f, 0.f);
    final[ 87] = glm::vec3( 4.5f,  0.5f, 0.f);
    final[ 88] = glm::vec3( 4.5f,  0.0f, 0.f);
    final[ 89] = glm::vec3( 4.5f, -0.5f, 0.f);
    final[ 90] = glm::vec3( 4.5f, -1.0f, 0.f);
    final[ 91] = glm::vec3( 4.5f, -1.5f, 0.f);
    final[ 92] = glm::vec3( 4.5f, -2.0f, 0.f);
    final[ 93] = glm::vec3( 4.5f, -2.5f, 0.f);
    final[ 94] = glm::vec3( 4.5f, -3.0f, 0.f);
    final[ 95] = glm::vec3( 4.5f, -3.5f, 0.f);
    final[ 96] = glm::vec3( 4.5f, -4.0f, 0.f);
    final[ 97] = glm::vec3( 5.0f,  3.0f, 0.f);
    final[ 98] = glm::vec3( 5.5f,  2.5f, 0.f);
    final[ 99] = glm::vec3( 6.0f,  2.0f, 0.f);
    final[100] = glm::vec3( 6.5f,  2.5f, 0.f);
    final[101] = glm::vec3( 7.0f,  3.0f, 0.f);
    final[102] = glm::vec3( 7.5f,  3.5f, 0.f);
    final[103] = glm::vec3( 7.5f,  3.0f, 0.f);
    final[104] = glm::vec3( 7.5f,  2.5f, 0.f);
    final[105] = glm::vec3( 7.5f,  2.0f, 0.f);
    final[106] = glm::vec3( 7.5f,  1.5f, 0.f);
    final[107] = glm::vec3( 7.5f,  1.0f, 0.f);
    final[108] = glm::vec3( 7.5f,  0.5f, 0.f);
    final[109] = glm::vec3( 7.5f,  0.0f, 0.f);
    final[100] = glm::vec3( 7.5f, -0.5f, 0.f);
    final[111] = glm::vec3( 7.5f, -1.0f, 0.f);
    final[112] = glm::vec3( 7.5f, -1.5f, 0.f);
    final[113] = glm::vec3( 7.5f, -2.0f, 0.f);
    final[114] = glm::vec3( 7.5f, -2.5f, 0.f);
    final[115] = glm::vec3( 7.5f, -3.0f, 0.f);
    final[116] = glm::vec3( 7.5f, -3.5f, 0.f);
    final[117] = glm::vec3( 7.5f, -4.0f, 0.f);

    // O
    final[118] = glm::vec3( 9.5f,  3.5f, 0.f);
    final[119] = glm::vec3( 9.5f,  3.0f, 0.f);
    final[120] = glm::vec3( 9.5f,  2.5f, 0.f);
    final[121] = glm::vec3( 9.5f,  2.0f, 0.f);
    final[122] = glm::vec3( 9.5f,  1.5f, 0.f);
    final[123] = glm::vec3( 9.5f,  1.0f, 0.f);
    final[124] = glm::vec3( 9.5f,  0.5f, 0.f);
    final[125] = glm::vec3( 9.5f,  0.0f, 0.f);
    final[126] = glm::vec3( 9.5f, -0.5f, 0.f);
    final[127] = glm::vec3( 9.5f, -1.0f, 0.f);
    final[128] = glm::vec3( 9.5f, -1.5f, 0.f);
    final[129] = glm::vec3( 9.5f, -2.0f, 0.f);
    final[130] = glm::vec3( 9.5f, -2.5f, 0.f);
    final[131] = glm::vec3( 9.5f, -3.0f, 0.f);
    final[132] = glm::vec3( 9.5f, -3.5f, 0.f);
    final[133] = glm::vec3( 9.5f, -4.0f, 0.f);
    final[134] = glm::vec3(10.0f, -4.0f, 0.f);
    final[135] = glm::vec3(10.0f,  3.5f, 0.f);
    final[136] = glm::vec3(10.5f, -4.0f, 0.f);
    final[137] = glm::vec3(10.5f,  3.5f, 0.f);
    final[138] = glm::vec3(11.0f, -4.0f, 0.f);
    final[139] = glm::vec3(11.0f,  3.5f, 0.f);
    final[140] = glm::vec3(11.5f,  3.5f, 0.f);
    final[141] = glm::vec3(11.5f,  3.0f, 0.f);
    final[142] = glm::vec3(11.5f,  2.5f, 0.f);
    final[143] = glm::vec3(11.5f,  2.0f, 0.f);
    final[144] = glm::vec3(11.5f,  1.5f, 0.f);
    final[145] = glm::vec3(11.5f,  1.0f, 0.f);
    final[146] = glm::vec3(11.5f,  0.5f, 0.f);
    final[147] = glm::vec3(11.5f,  0.0f, 0.f);
    final[148] = glm::vec3(11.5f, -0.5f, 0.f);
    final[149] = glm::vec3(11.5f, -1.0f, 0.f);
    final[150] = glm::vec3(11.5f, -1.5f, 0.f);
    final[151] = glm::vec3(11.5f, -2.0f, 0.f);
    final[152] = glm::vec3(11.5f, -2.5f, 0.f);
    final[153] = glm::vec3(11.5f, -3.0f, 0.f);
    final[154] = glm::vec3(11.5f, -3.5f, 0.f);
    final[155] = glm::vec3(11.5f, -4.0f, 0.f);

    glGenBuffers(1, &quad_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BUTTERFLY)*num_butterflies, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BUTTERFLY), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BUTTERFLY), (const GLvoid *)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenBuffers(1, &matrices_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices_block), NULL, GL_DYNAMIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    camera = new Camera(glm::vec3(0.0f, 0.5f, 25.0f));
    camera->set_proj_matrix(info.aspect, 0.001f, 1000.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);

    sinE = sinf(eLit);
    cosE = cosf(eLit);
    sinA = sinf(aLit);
    cosA = cosf(aLit);
  }

virtual void render(double currentTime) {
      
  static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
  static const GLfloat one  = 1.0f;

  if (mouse.pressed_left)
    {
      double xpos, ypos;
      glfwGetCursorPos(info.window, &xpos, &ypos);
      camera->rotate((float)(xpos - mouse.xpos), (float)(ypos - mouse.ypos));
      mouse.xpos = xpos;
      mouse.ypos = ypos;
    }

  camera->update();

  glClearBufferfv(GL_COLOR, 0, green);
  glClearBufferfv(GL_DEPTH, 0, &one);

  for (int i = 0; i < num_butterflies; i++) {
    timer[i] -= (float)deltaTime;
    if (timer[i] <= 0.0f)
      {
	if (butterfly_end)
	  next[i] = final[i];
	else
	  {
	    next[i] = butterfly[i].position + glm::vec3(-80.f + (float)rand() * 160.f / (float)RAND_MAX,
							-20.f + (float)rand() *  40.f / (float)RAND_MAX,
							-80.f + (float)rand() * 160.f / (float)RAND_MAX);
	  }
	timer[i] = 0.5f + (float)rand() * 1.5f / (float)RAND_MAX;
      }
    butterfly[i].position += glm::normalize(next[i] - butterfly[i].position) *
      time_fade * (float)deltaTime;
  }

  light_pos = glm::vec4(lightRadius * glm::vec3(cosE * sinA, sinE, cosE * cosA), (float)currentTime);
    
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_buffer);
  matrices_block * block = (matrices_block *)glMapBufferRange(GL_UNIFORM_BUFFER,
							      0,
							      sizeof(matrices_block),
							      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  block->view_matrix  = camera->get_view_matrix();
  block->proj_matrix  = camera->get_proj_matrix();
  block->vp_matrix    = camera->get_vp_matrix();
  block->light_pos    = light_pos;

  glUnmapBuffer(GL_UNIFORM_BUFFER);

  glUseProgram(butterflyProgram);

  glBindTexture(GL_TEXTURE_2D, butterfly_texture);
  glBindVertexArray(quad_vao);
  glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(BUTTERFLY)*num_butterflies, &butterfly[0].position.x);
  glDrawArrays(GL_POINTS, 0, num_butterflies);
  glBindVertexArray(0);

  if (calcFps) {
    ImGui_ImplGlfwGL3_NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::Begin("Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
  }
  
  deltaTime = currentTime-lastTime;
  lastTime = currentTime;
}

virtual void shutdown() {
  glDeleteProgram(butterflyProgram);
  glDeleteVertexArrays(1, &quad_vao);
  glDeleteBuffers(1, &quad_buffer);
  glDeleteTextures(1, &butterfly_texture);
}

private:
  double lastFPSTime;
  double lastTime = 0.0;
  double deltaTime;
  unsigned long frame;
  bool calcFps;

  bool wireframe;
  
  glm::vec4 light_pos;
  float lightRadius = 100.0f, eLit = 0.0f, aLit = 0.0f, sLit = 0.01f;
  float sinE, cosE, sinA, cosA;
  
  struct
  {
    bool pressed_left = false;
    bool pressed_right = false;
    double xpos;
    double ypos;
  } mouse;

  GLuint butterflyProgram;
  struct matrices_block
  {
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::mat4 vp_matrix;
    glm::vec4 light_pos;
  };
  GLuint  matrices_buffer;
  GLuint  quad_buffer;
  GLuint  quad_vao;
  GLuint  butterfly_texture;
  GLsizei num_butterflies;
  struct BUTTERFLY
  {
    glm::vec3 position;
    glm::vec3 properties;
  };
  BUTTERFLY* butterfly;
  float *timer, time_fade = 10.0f;
  glm::vec3 *final, *next;
  bool butterfly_end = false;
  Camera *camera;

};

Application * Application::app = 0;

DECLARE_MAIN(test_app)
