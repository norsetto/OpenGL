#include "config.h"
#include "main.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "water.hpp"

#include <image.hpp>

#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <tinyxml2.h>
#include <string>

#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp> // glm::to_string
#undef GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp> //glm::value_ptr

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
      case 'G': wireframe = !wireframe;
	if (wireframe)
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;
      case 'U': update_waves = !update_waves;
	break;
      case 'E': lake.generate();
	break;
      case 'O':
	if (modifier & GLFW_MOD_SHIFT)
	  min_amplitude /= 1.01f;
	else
	  min_amplitude *= 1.01f;
	printf("Min. wave amplitude %f\n", min_amplitude);
	lake.amplitude(min_amplitude, max_amplitude);
	break;
      case 'M':
	if (modifier & GLFW_MOD_SHIFT)
	  max_amplitude /= 1.01f;
	else
	  max_amplitude *= 1.01f;
	printf("Max. wave amplitude %f\n", max_amplitude);
	lake.amplitude(min_amplitude, max_amplitude);
	break;
      case 'R':
	if (modifier & GLFW_MOD_SHIFT)
	  roughness /= 1.01f;
	else
	  roughness *= 1.01f;
	printf("Wave roughness %f\n", roughness);
	lake.roughness(roughness);
	break;
      case 'T':
	if (modifier & GLFW_MOD_SHIFT)
	  wavelength /= 1.01f;
	else
	  wavelength *= 1.01f;
	printf("Wave length %f\n", wavelength);
	lake.wavelength(wavelength);
	break;
      case 'L':
	if (modifier & GLFW_MOD_SHIFT)
	  {
	    lod--;
	    if (lod < 0) lod = 0;
	  } else
	  {
	    lod++;
	    if (lod > max_lod) lod = max_lod;
	  }
	num_patches = static_cast<GLsizei>(powf(4.0f, static_cast<float>(lod)));
	glUniform1i(uniform.patch_size, static_cast<int>(powf(2.0f, static_cast<float>(lod))));
	std::cout<<"lod = "<<lod<<std::endl;
	break;
      case 'N':
	if (modifier & GLFW_MOD_SHIFT)
	  {
	    grid--;
	    if (grid < 0) grid = 0;
	  } else
	  {
	    grid++;
	    if (grid > max_grid) grid = max_grid;
	  }
	glUniform1i(uniform.grid, grid);
	std::cout<<"grid = "<<grid<<std::endl;
	break;
      case 'V':
	if (modifier & GLFW_MOD_SHIFT)
	  {
	    max_distance /= 1.05f;
	  } else
	  {
	    max_distance *= 1.05f;
	  }
	glUniform1f(uniform.max_distance, max_distance);
	std::cout<<"max distance = "<<max_distance<<std::endl;
	break;
      case 'P':
	std::cout<<glm::to_string(camera->get_position())<<" "<<glm::to_string(camera->get_direction())<<std::endl;
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
      case GLFW_KEY_RIGHT_BRACKET: camera->change_speed(1.1f);
	break;
      case GLFW_KEY_LEFT_BRACKET: camera->change_speed(1.0f / 1.1f);
	break;
      }
    }
  }

  void onMouseWheel(double pos) {
    camera->change_fov (0.02f * (float)pos);
  }

  void onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      mouse.pressed = !mouse.pressed;
      glfwGetCursorPos(info.window, &mouse.xpos, &mouse.ypos);
    }
  }

  virtual void startup() {

    fprintf(stdout,"%s Version %s - %s build\n",
            info.argv[0],
            VERSION,
	    BUILD_TYPE);

    GLuint shader[4];
    bool check_errors = false;
    
#ifdef GL_DEBUG
    check_errors = true;
#endif
    
    shader[0] = shader::load(SHADERS_LOCATION "water/water.vs.glsl", GL_VERTEX_SHADER, check_errors);
    shader[1] = shader::load(SHADERS_LOCATION "water/water.tcs.glsl", GL_TESS_CONTROL_SHADER, check_errors);
    shader[2] = shader::load(SHADERS_LOCATION "water/water.tes.glsl", GL_TESS_EVALUATION_SHADER, check_errors);
    shader[3] = shader::load(SHADERS_LOCATION "water/water.fs.glsl", GL_FRAGMENT_SHADER, check_errors);

    waterProgram = program::link_from_shaders(shader, 4, true, check_errors);

    //INPUT
    glm::vec3 camera_position;
    glm::vec2 camera_direction;
    float camera_speed;
    float water_size;

    //Load alternative input data from xml file
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLError Result = xmlDoc.LoadFile(INPUT_LOCATION "water.xml");
    assert(Result == tinyxml2::XML_SUCCESS);
    _unused( Result );
    tinyxml2::XMLElement *xmlRoot = xmlDoc.FirstChildElement("data");
    assert(xmlRoot!=nullptr);

    xmlRoot->FirstChildElement("position")->FirstChildElement("x")->QueryFloatText(&camera_position.x);
    xmlRoot->FirstChildElement("position")->FirstChildElement("y")->QueryFloatText(&camera_position.y);
    xmlRoot->FirstChildElement("position")->FirstChildElement("z")->QueryFloatText(&camera_position.z);
    camera = new Camera(camera_position);
    
    xmlRoot->FirstChildElement("pitch")->QueryFloatText(&camera_direction.x);
    xmlRoot->FirstChildElement("yaw")->QueryFloatText(&camera_direction.y);
    camera->set_direction(camera_direction);
    
    xmlRoot->FirstChildElement("speed")->QueryFloatText(&camera_speed);
    camera->set_speed(camera_speed);

    xmlRoot->FirstChildElement("max_lod")->QueryIntText(&max_lod);
    lod = max_lod;
    xmlRoot->FirstChildElement("max_grid")->QueryIntText(&max_grid);
    grid = max_grid;
    
    xmlRoot->FirstChildElement("max_distance")->QueryFloatText(&max_distance);
    xmlRoot->FirstChildElement("water_size")->QueryFloatText(&water_size);

    const char * texture_file = xmlRoot->FirstChildElement("texture_file")->GetText();

    char texture_location[MAX_STRING_LENGTH];
    strcpy(texture_location, DATA_LOCATION);
    strcat(texture_location, texture_file);
    
    glGenBuffers(1, &matrices_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices_block), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_buffer);
    block = (matrices_block *)glMapBufferRange(GL_UNIFORM_BUFFER,
					       0,
					       sizeof(matrices_block),
					       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glUseProgram(waterProgram);
    num_patches = static_cast<GLsizei>(powf(4.0f, static_cast<float>(lod)));

    //Set uniforms
    uniform.grid = glGetUniformLocation(waterProgram, "grid");
    uniform.max_distance = glGetUniformLocation(waterProgram, "max_distance");
    uniform.water_size = glGetUniformLocation(waterProgram, "water_size");
    uniform.viewport = glGetUniformLocation(waterProgram, "viewport");
    uniform.patch_size = glGetUniformLocation(waterProgram, "patch_size");
    glUniform1i(uniform.grid, grid);
    glUniform1i(uniform.patch_size, static_cast<int>(powf(2.0f, static_cast<float>(lod))));
    glUniform1f(uniform.max_distance, max_distance);
    glUniform1f(uniform.water_size, water_size);
    
    camera->set_proj_matrix(info.aspect, 0.001f, max_distance * 2.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glUniform2f(uniform.viewport, static_cast<float>(info.windowWidth), static_cast<float>(info.windowHeight));

    sinE = sinf(eLit);
    cosE = cosf(eLit);
    sinA = sinf(aLit);
    cosA = cosf(aLit);

    glActiveTexture(GL_TEXTURE0);
    Image water_texture(texture_id, texture_location);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    lake.generate();
    
    lastTime = 0.0;
  }

  virtual void render(double currentTime) {

    if (mouse.pressed) {
      double xpos, ypos;
      glfwGetCursorPos(info.window, &xpos, &ypos);
      camera->rotate((float)(xpos - mouse.xpos), (float)(ypos - mouse.ypos));
      mouse.xpos = xpos;
      mouse.ypos = ypos;
    }

    camera->update();

    if (update_waves)
      lake.update(deltaTime);

    light_pos = glm::vec4(lightRadius * glm::vec3(cosE * sinA, sinE, cosE * cosA), (float)currentTime);

    static const GLfloat sky_blue[] = { 0.7f, 1.0f, 1.0f, 0.0f };
    static const GLfloat one  = 1.0f;
    glClearBufferfv(GL_COLOR, 0, sky_blue);
    glClearBufferfv(GL_DEPTH, 0, &one);

    block->view_matrix  = camera->get_view_matrix();
    block->proj_matrix  = camera->get_proj_matrix();
    block->vp_matrix    = camera->get_vp_matrix();
    block->light_pos    = light_pos;
    std::memcpy(&(block->wave_data), lake.data(), sizeof(water::WAVE_DATA));

    lake.render(num_patches);

    if (calcFps) {
      frame++;
      double deltaFPSTime = currentTime - lastFPSTime;
      if (deltaFPSTime > 0.25) {
	double fps = ((double)frame) / deltaFPSTime;
	printf("\r%7.2f FPS", fps);
	fflush(stdout);
	lastFPSTime = currentTime;
	frame = 0;
      }
    }

    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
  }

  virtual void shutdown() {
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glDeleteProgram(waterProgram);
    glDeleteTextures(1, &texture_id);
  }

private:
  double lastFPSTime;
  double lastTime = 0.0;
  double deltaTime;
  unsigned long frame;
  bool calcFps = false;
  bool wireframe = true;
  bool update_waves = false;
  struct MOUSE {
    bool pressed = false;
    double xpos;
    double ypos;
  } mouse;
  glm::vec4 light_pos;
  float lightRadius = 100.0f, eLit = 0.7854f, aLit = 0.7854f, sLit = 0.01f;
  float sinE, cosE, sinA, cosA;
  Camera* camera;
  GLuint waterProgram;
  water::Water lake;
  GLuint texture_id;
  float min_amplitude = 0.1f, max_amplitude = 0.5f, wavelength = 10.5f, roughness = 0.11f;
  struct matrices_block
  {
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::mat4 vp_matrix;
    glm::vec4 light_pos;
    water::WAVE_DATA wave_data;
  };
  GLuint matrices_buffer;
  matrices_block * block;
  struct UNIFORM {
    GLint grid;
    GLint max_distance;
    GLint water_size;
    GLint viewport;
    GLint patch_size;
  } uniform;
  int max_lod, lod;
  int max_grid, grid;
  float max_distance;
  GLsizei num_patches;
};

Application * Application::app = 0;

DECLARE_MAIN(test_app)
