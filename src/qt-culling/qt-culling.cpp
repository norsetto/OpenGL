#include <vector>
#include <iostream>
#include <algorithm>

#include "main.hpp"
#include "config.h"
#include "camera.hpp"
#include "shader.hpp"
#include "object.hpp"
#include "poisson.hpp"
#include "quadtree.hpp"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

class test_app : public Application {

public:
  test_app() {}

protected:

  struct OBJECT
  {
    glm::mat4 model_matrix;
    unsigned int identifier;
    float distance;
  };
  enum {ROCK, TREE, TREE_OLD, SPRUCE };

  inline float distance2(OBJECT a, glm::vec3 b)
  {
    glm::vec3 temp = glm::vec3(a.model_matrix[3]) - b;
    return glm::dot(temp, temp);
  }

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
      case GLFW_KEY_LEFT_BRACKET: sampling_area /= 1.1f;
	break;
      case GLFW_KEY_RIGHT_BRACKET: sampling_area *= 1.1f;
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
    camera->set_proj_matrix(info.aspect, 0.001f, 10.0f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
  }

  void onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		mouse.pressed = !mouse.pressed;
		glfwGetCursorPos(info.window, &mouse.xpos, &mouse.ypos);
    }
  }

  virtual void startup() {

    camera = new Camera(glm::vec3(0.5f, 0.001f, 0.5f));
    camera->set_direction(glm::vec2(0.0f, -1.0f));
    camera->set_speed(0.05f);
    
    GLuint shader[2];
    bool check_errors = false;
    
#ifdef GL_DEBUG
    check_errors = true;
#endif

    
    shader[0] = shader::load(SHADERS_LOCATION "qt-culling/object.vs.glsl", GL_VERTEX_SHADER, check_errors);
    shader[1] = shader::load(SHADERS_LOCATION "qt-culling/object.fs.glsl", GL_FRAGMENT_SHADER, check_errors);

    program_obj = program::link_from_shaders(shader, 2, true, check_errors);

    uniforms.obj.projection_matrix = glGetUniformLocation(program_obj, "proj_matrix");
    uniforms.obj.view_matrix       = glGetUniformLocation(program_obj, "view_matrix");
    uniforms.obj.model_matrix      = glGetUniformLocation(program_obj, "model_matrix");

    tree.load(MODELS_LOCATION "pine_tree.obj");
    tree_old.load(MODELS_LOCATION "pine_tree_old.obj");
    spruce.load(MODELS_LOCATION "spruce.obj");
    rock.load(MODELS_LOCATION "rock.obj");
	
    Poisson Points;
    Points.generate(40000, 200);
    std::vector<glm::vec3> obj_positions;
    for (const auto &p : Points.points) {
      glm::vec3 position = glm::vec3(p.x, 0.0f, p.y);
      obj_positions.push_back(position);
    }
    num_instances = obj_positions.size();
    object.reserve(num_instances);

    static const GLfloat global_scale = 0.00025f;
    
    for (GLsizei i = 0; i < num_instances / 10; i++) {
      OBJECT temp_object;
      int k = rand() * (obj_positions.size() - 1) / RAND_MAX;
      glm::vec3 temp_position = obj_positions[k];
      obj_positions.erase(obj_positions.begin() + k);
      glm::mat4 temp_mat = glm::mat4(1.0f);
      temp_mat = glm::translate(temp_mat, temp_position);
      float rot_angle = (float)rand() * 6.2831853f / (float)RAND_MAX;
      temp_mat = glm::rotate(temp_mat, rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
      rot_angle = (float)rand() * 6.2831853f / (float)RAND_MAX;
      temp_mat = glm::rotate(temp_mat, rot_angle, glm::vec3(1.0f, 0.0f, 0.0f));
      rot_angle = (float)rand() * 6.2831853f / (float)RAND_MAX;
      temp_mat = glm::rotate(temp_mat, rot_angle, glm::vec3(0.0f, 0.0f, 1.0f));
      float scale1 = 6.0f + (float)rand() * 6.0f / (float)RAND_MAX;
      float scale2 = 6.0f + (float)rand() * 6.0f / (float)RAND_MAX;
      float scale3 = 6.0f + (float)rand() * 6.0f / (float)RAND_MAX;
      temp_object.model_matrix = glm::scale(temp_mat, glm::vec3(scale1, scale2, scale3) * global_scale);
      temp_object.distance = distance2(temp_object, camera->get_position());
      temp_object.identifier = ROCK;
      object.push_back(temp_object);
    }

    for (int i = num_instances / 10; i < num_instances / 5; i++) {
      OBJECT temp_object;
      int k = rand() * (obj_positions.size() - 1) / RAND_MAX;
      glm::vec3 temp_position = obj_positions[k];
      obj_positions.erase(obj_positions.begin() + k);
      glm::mat4 temp_mat = glm::mat4(1.0f);
      temp_mat = glm::translate(temp_mat, temp_position);
      float rot_angle = (float)rand() * 6.2831853f / (float)RAND_MAX;
      temp_mat = glm::rotate(temp_mat, rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
      float scale1 = 1.5f + (float)rand() * 0.5f / (float)RAND_MAX;
      float scale2 = 1.0f + (float)rand() * 1.0f / (float)RAND_MAX;
      float scale3 = 1.5f + (float)rand() * 0.5f / (float)RAND_MAX;
      temp_object.model_matrix = glm::scale(temp_mat, glm::vec3(scale1, scale2, scale3) * global_scale);
      temp_object.distance = distance2(temp_object, camera->get_position());
      temp_object.identifier = TREE_OLD;
      object.push_back(temp_object);
    }

    for (int i = num_instances / 5; i < (num_instances * 3) / 10; i++) {
      OBJECT temp_object;
      int k = rand() * (obj_positions.size() - 1) / RAND_MAX;
      glm::vec3 temp_position = obj_positions[k];
      obj_positions.erase(obj_positions.begin() + k);
      glm::mat4 temp_mat = glm::mat4(1.0f);
      temp_mat = glm::translate(temp_mat, temp_position);
      float rot_angle = (float)rand() * 6.2831853f / (float)RAND_MAX;
      temp_mat = glm::rotate(temp_mat, rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
      float scale1 = 2.5f + (float)rand() * 1.0f / (float)RAND_MAX;
      float scale2 = 2.0f + (float)rand() * 2.0f / (float)RAND_MAX;
      float scale3 = 2.5f + (float)rand() * 1.0f / (float)RAND_MAX;
      temp_object.model_matrix = glm::scale(temp_mat, glm::vec3(scale1, scale2, scale3) * global_scale);
      temp_object.distance = distance2(temp_object, camera->get_position());
      temp_object.identifier = SPRUCE;
      object.push_back(temp_object);
    }

    for (int i = (num_instances * 3) / 10; i < num_instances; i++) {
      OBJECT temp_object;
      int k = rand() * (obj_positions.size() - 1) / RAND_MAX;
      glm::vec3 temp_position = obj_positions[k];
      obj_positions.erase(obj_positions.begin() + k);
      glm::mat4 temp_mat = glm::mat4(1.0f);
      temp_mat = glm::translate(temp_mat, temp_position);
      float rot_angle = (float)rand() * 6.2831853f / (float)RAND_MAX;
      temp_mat = glm::rotate(temp_mat, rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
      float scale1 = 1.5f + (float)rand() * 0.5f / (float)RAND_MAX;
      float scale2 = 1.0f + (float)rand() * 1.0f / (float)RAND_MAX;
      float scale3 = 1.5f + (float)rand() * 0.5f / (float)RAND_MAX;
      temp_object.model_matrix = glm::scale(temp_mat, glm::vec3(scale1, scale2, scale3) * global_scale);
      temp_object.distance = distance2(temp_object, camera->get_position());
      temp_object.identifier = TREE;
      object.push_back(temp_object);
    }

    const quadtree::AABB boundary(quadtree::Point(0.5f, 0.5f), quadtree::Point(0.5f, 0.5f));
    quadtree.set_boundary(boundary);
    for (auto&& ob:object) {
      if (!quadtree.insert({quadtree::Point(ob.model_matrix[3][0], ob.model_matrix[3][2]), &ob})) {
	std::cout << "Failed insertion!" << std::endl;
	std::exit(-1);
      }
    }
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    camera->set_proj_matrix(info.aspect, 0.001f, 10.f);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
  }

    virtual void render(double currentTime) {

      deltaTime = currentTime - lastTime;
      lastTime = currentTime;

      if (mouse.pressed) {
	double xpos, ypos;
	glfwGetCursorPos(info.window, &xpos, &ypos);
	camera->rotate((float)(xpos - mouse.xpos), (float)(ypos - mouse.ypos));
	mouse.xpos = xpos;
	mouse.ypos = ypos;
      }

      camera->update();
      
      rendered_objects.clear();
      sample_boundary = {quadtree::Point(camera->get_position().x, camera->get_position().z), quadtree::Point(sampling_area, sampling_area)};
      quadtree.queryRange(sample_boundary, rendered_objects);

      for (auto&& ob:rendered_objects)
	{
	  ob->distance = distance2(*ob, camera->get_position());
	}

      std::sort(rendered_objects.begin(), rendered_objects.end(),
		[&](const OBJECT* a, const OBJECT* b)
		{
		  return a->distance > b->distance;
		});

      static const GLfloat sky_blue[] = { 0.7f, 1.0f, 1.0f, 0.0f };
      static const GLfloat one  = 1.0f;
      glClearBufferfv(GL_COLOR, 0, sky_blue);
      glClearBufferfv(GL_DEPTH, 0, &one);

      glUseProgram(program_obj);
      glUniformMatrix4fv(uniforms.obj.projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->get_proj_matrix()));
      glUniformMatrix4fv(uniforms.obj.view_matrix, 1, GL_FALSE, glm::value_ptr(camera->get_view_matrix()));

      for (auto&& ob: rendered_objects) {
	glUniformMatrix4fv(uniforms.obj.model_matrix, 1,
			   GL_FALSE, &(ob->model_matrix[0].x));
	switch(ob->identifier) {
	case(ROCK):
	  rock.render();
	  break;
	case(TREE):
	  glEnable(GL_BLEND);
	  glDisable(GL_CULL_FACE);
	  tree.render();
	  glEnable(GL_CULL_FACE);
	  glDisable(GL_BLEND);
	  break;
	case(TREE_OLD):
	  glEnable(GL_BLEND);
	  glDisable(GL_CULL_FACE);
	  tree_old.render();
	  glEnable(GL_CULL_FACE);
	  glDisable(GL_BLEND);
	  break;
	case(SPRUCE):
	  glEnable(GL_BLEND);
	  glDisable(GL_CULL_FACE);
	  spruce.render();
	  glEnable(GL_CULL_FACE);
	  glDisable(GL_BLEND);
	  break;
	}
      }
    }

    virtual void shutdown() {
      glDeleteProgram(program_obj);
      tree.free();
      tree_old.free();
      spruce.free();
      rock.free();
    }

private:
  double lastTime = 0.0;
  double deltaTime;
  struct MOUSE {
    bool pressed = false;
    double xpos;
    double ypos;
  } mouse;
  Camera *camera;
  GLuint program_obj;
  struct {
    struct {
      GLint projection_matrix;
      GLint view_matrix;
      GLint model_matrix;
    } obj;
  } uniforms;
  Object tree, spruce, rock, tree_old;
  std::vector<glm::mat4> object_model_matrix;
  GLsizei num_instances;
  std::vector<OBJECT> object;
  quadtree::Quadtree<OBJECT> quadtree;
  std::vector<OBJECT*> rendered_objects;
  quadtree::AABB sample_boundary;
  GLfloat sampling_area = 0.05f;
};

Application * Application::app = 0;

DECLARE_MAIN(test_app)
