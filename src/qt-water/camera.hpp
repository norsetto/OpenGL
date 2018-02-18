#pragma once

#include "config.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

struct AABB {

  glm::vec3 centre;
  glm::vec3 halfSize;

  AABB(glm::vec3 centre = {}, glm::vec3 halfSize = {}) : centre(centre), halfSize(halfSize) {};

};

class Camera {
public:

  Camera(glm::vec3 position = {}) : position(position) {};

  void set_position(glm::vec3 position = glm::vec3(0.0f));
  void set_direction(glm::vec2 direction = glm::vec2(0.0f));
  void set_fov(float fov = 0.7f);
  void set_speed(float speed = 50.0f);
  void set_rate(float rate = 0.005f);
  void move_forward(float deltaTime);
  void move_backward(float deltaTime);
  void move_right(float deltaTime);
  void move_left(float deltaTime);
  void move_up(float deltaTime);
  void move_down(float deltaTime);
  void change_fov(float delta_fov);
  void change_speed(float delta_speed);
  void rotate(float delta_x, float delta_y);
  void update(void);
  void set_proj_matrix(float aspect, float minz, float maxz);
  glm::mat4 get_proj_matrix(void);
  glm::mat4 get_view_matrix(void);
  glm::mat4 get_vp_matrix(void);
  glm::vec3 get_position(void);
  glm::vec2 get_direction(void);
  bool AABBInsideFrustum(const AABB &boundary);
  bool AABBInsidePlane(const glm::vec4 &plane, const glm::vec3 &max_v, const glm::vec3 &min_v);
  
private:
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  float fov = 0.7f;
  float aspect = 1.78f;
  float minz = 0.001f;
  float maxz = 1000.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;
  float speed = 50.0f;
  float rate = 0.005f;
  glm::mat4 view_matrix;
  glm::mat4 proj_matrix;
  glm::mat4 vp_matrix;
  glm::vec4 near_plane;
  glm::vec4 far_plane;
  glm::vec4 left_plane;
  glm::vec4 right_plane;
  glm::vec4 bottom_plane;
  glm::vec4 top_plane;

};
