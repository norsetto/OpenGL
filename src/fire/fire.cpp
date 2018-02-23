#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <random>
#include <vector>

#include "main.hpp"
#include "particles.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"

#define NUM_PARTICLES 512

std::random_device rd;
std::default_random_engine rndEngine(rd());

float rnd(float range)
{
  std::uniform_real_distribution<float> rndDist(0.0f, range);
  return rndDist(rndEngine);
}

float fire_size = 0.035f;
float prob_to_smoke = 2.0f;
float smoke_size_factor = 0.01f;

Particle::PARTICLE generate(void)
{
  float azimuth = rnd(TAU);
  float elevation = rnd(float(M_PI)) - float(M_PI) / 2.0f;
  float r = rnd(fire_size);
  return Particle::PARTICLE(glm::vec3(r * cos(azimuth) * cos(elevation),
				      r * sin(elevation),
				      r * sin(azimuth) * cos(elevation)),
			    rnd(TAU),
			    glm::vec4(1.0f, 1.0f, 1.0f, 0.6f + rnd(0.4f)),
			    glm::vec3(0.0f, rnd(0.01f), 0.0f),
			    rnd(0.04f) - 0.02f,
			    0.075f + rnd(0.05f),
			    0
			    );
}

class Fire: public Particle
{
public:
  void update(float delta_time)
  {
    for (auto &particle : m_particles)
      {
	if (particle.type == 0)
	  if (rnd(100.0f) < prob_to_smoke)
	    {
	      particle.velocity = glm::vec3((rnd(0.02f) - 0.01f),
					    rnd(0.1f),
					    (rnd(0.02f) - 0.01f));
	      particle.color =  glm::vec4(0.8f, 0.8f, 1.0f, 0.6f + rnd(0.4f));
	      particle.rotationRate = rnd(0.16f) - 0.8f;
	      particle.type = 1;
	    }
	if (particle.color.w < 0.0f)
	  {
	    particle = m_generate_particle();
	  } else
	  {
	    particle.position += particle.velocity * delta_time;
	    particle.rotation += particle.rotationRate * delta_time;
	    particle.size -= m_size_rate * delta_time;
	    particle.color.w -= m_alpha_rate * delta_time;
	  }
      }
  }
};


class test_app : public Application {

public:
  test_app() :	showUi(false)
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
      case 'U':
	showUi = !showUi;
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
    camera->set_proj_matrix(info.aspect, 0.001f, 100.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
  }

  void onMouseWheel(double pos) {
    camera->change_fov (0.02f * (float)pos);
  }

  void onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
      {
	glfwGetCursorPos(info.window, &mouse.xpos, &mouse.ypos);
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

    GLuint shader[3];

    shader[0] = shader::load(SHADERS_LOCATION "fire/fire.vs.glsl", GL_VERTEX_SHADER);
    shader[1] = shader::load(SHADERS_LOCATION "fire/fire.gs.glsl", GL_GEOMETRY_SHADER);
    shader[2] = shader::load(SHADERS_LOCATION "fire/fire.fs.glsl", GL_FRAGMENT_SHADER);

    fireProgram = program::link_from_shaders(shader, 3, true);

    camera = new Camera(glm::vec3(0.0f, 0.0f, 2.0f));
    camera->set_proj_matrix(info.aspect, 0.001f, 100.0f);
    camera->set_speed(2.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
    
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo_data), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    block = (ubo_data *)glMapBufferRange(GL_UNIFORM_BUFFER,
					 0,
					 sizeof(ubo_data),
					 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    fire.startup(NUM_PARTICLES, generate);
    fire.setProgram(fireProgram);
    Image smoke_texture(texture_smoke, DATA_LOCATION "smoke.png");
    Image fire_texture (texture_fire, DATA_LOCATION "fire.png");
    fire.setTexture1(texture_fire);
    fire.setTexture2(texture_smoke);
    fire.setSizeRate(smoke_size_factor);
  }

virtual void render(double currentTime) {
      
  if (mouse.pressed_right)
    {
      double xpos, ypos;
      glfwGetCursorPos(info.window, &xpos, &ypos);
      camera->rotate((float)(xpos - mouse.xpos), (float)(ypos - mouse.ypos));
      mouse.xpos = xpos;
      mouse.ypos = ypos;
    }

  deltaTime = currentTime - lastTime;
  lastTime = currentTime;

  glClear(GL_COLOR_BUFFER_BIT);
  camera->update();
  
  block->view_matrix  = camera->get_view_matrix();
  block->proj_matrix  = camera->get_proj_matrix();
  block->vp_matrix    = camera->get_vp_matrix();

  fire.update(float(deltaTime));
  fire.render();

  if (showUi) {
    ImGui_ImplGlfwGL3_NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(0,0));
    ImGui::Begin("Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SliderFloat("Smoke probability", &prob_to_smoke, 0.0f, 10.0f);
    ImGui::SliderFloat("Fire size", &fire_size, 0.0f, 0.05f);
    ImGui::SliderFloat("Smoke size", &smoke_size_factor, 0.0f, 0.02f);
    fire.setSizeRate(smoke_size_factor);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
  }
}

virtual void shutdown() {
  glUnmapBuffer(GL_UNIFORM_BUFFER);
  fire.destroy();
  glDeleteTextures(1, &texture_fire);
  glDeleteTextures(1, &texture_smoke);
}

private:
  bool showUi;

  Fire fire;
  GLuint fireProgram;
  GLuint texture_smoke, texture_fire;
  struct ubo_data
  {
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::mat4 vp_matrix;
  };
  GLuint ubo;
  ubo_data * block;

  double lastTime = 0.0, currentTime, deltaTime;

  struct
  {
    bool pressed_left = false;
    bool pressed_right = false;
    double xpos;
    double ypos;
  } mouse;

  Camera *camera;
};

Application * Application::app = 0;

DECLARE_MAIN(test_app)
