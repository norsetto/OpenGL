#include "camera.hpp"

void Camera::set_position(glm::vec3 position) {
  this->position = position;
}

void Camera::set_direction(glm::vec2 direction) {
  this->pitch = direction.x;
  this->yaw   = direction.y;
}

void Camera::set_fov(float fov) {
  this->fov = fov;
}

void Camera::set_speed(float speed) {
  this->speed = speed;
}

void Camera::set_rate(float rate) {
  this->rate = rate;
}

void Camera::move_forward(float deltaTime) {
  position += speed * front * deltaTime;
}

void Camera::move_backward(float deltaTime) {
  position -= speed * front * deltaTime;
}

void Camera::move_right(float deltaTime) {
  position += speed * right * deltaTime;
}

void Camera::move_left(float deltaTime) {
  position -= speed * right * deltaTime;
}

void Camera::move_up(float deltaTime) {
  position += speed * up * deltaTime;
}

void Camera::move_down(float deltaTime) {
  position -= speed * up * deltaTime;
}

void Camera::change_fov(float delta_fov) {
  fov += delta_fov;
  if (fov > 1.2f) fov = 1.2f;
  else if (fov < 0.01f) fov = 0.01f;
}

void Camera::change_speed(float delta_speed) {
  speed *= delta_speed;
}

void Camera::rotate(float delta_x, float delta_y) {
  yaw   += delta_x * rate;
  pitch += delta_y * rate;
  yaw -= TAU * truncf(yaw / TAU);
  if (pitch >  PIo2 - 0.5f * fov) {
    pitch =  PIo2 - 0.5f * fov;
  } else if (pitch < -PIo2 + 0.5f * fov) pitch = -PIo2 + 0.5f * fov;
}

void Camera::update(void) {
  
  proj_matrix = glm::perspective(fov, aspect, minz, maxz);

  front = glm::vec3(sinf(yaw) * cosf(pitch),
		    sinf(pitch),
		    -cosf(yaw) * cosf(pitch));
  front = glm::normalize(front);
  right = glm::normalize(glm::cross(front, glm::vec3(0.0, 1.0, 0.0)));
  //(-front.z, 0, front.x)
  up = glm::normalize(glm::cross(right, front));
  //(-front.x*front.y, 1.0, -front.y*front.z)
  view_matrix = glm::lookAt(position,
			    position + front,
			    up);
  vp_matrix = proj_matrix * view_matrix;

  near_plane = glm::row(vp_matrix, 3) + glm::row(vp_matrix, 2);
  near_plane /= glm::length(glm::vec3(near_plane));
  far_plane = glm::row(vp_matrix, 3) - glm::row(vp_matrix, 2);
  far_plane /= glm::length(glm::vec3(far_plane));
  left_plane = glm::row(vp_matrix, 3) + glm::row(vp_matrix, 0);
  left_plane /= glm::length(glm::vec3(left_plane));
  right_plane = glm::row(vp_matrix, 3) - glm::row(vp_matrix, 0);
  right_plane /= glm::length(glm::vec3(right_plane));
  bottom_plane = glm::row(vp_matrix, 3) + glm::row(vp_matrix, 1);
  bottom_plane /= glm::length(glm::vec3(bottom_plane));
  top_plane = glm::row(vp_matrix, 3) - glm::row(vp_matrix, 1);
  top_plane /= glm::length(glm::vec3(top_plane));
}

void Camera::set_proj_matrix(float aspect, float minz, float maxz) {
  proj_matrix = glm::perspective(fov, aspect, minz, maxz);
  this->aspect = aspect;
  this->minz = minz;
  this->maxz = maxz;
}

glm::mat4 Camera::get_proj_matrix(void) { return proj_matrix; }
glm::mat4 Camera::get_view_matrix(void) { return view_matrix; }
glm::mat4 Camera::get_vp_matrix(void) { return vp_matrix; }
glm::vec3 Camera::get_position(void) { return position; }
glm::vec2 Camera::get_direction(void) { return glm::vec2(pitch, yaw); }

bool Camera::AABBInsidePlane(const glm::vec4 &plane, const glm::vec3 &max_v, const glm::vec3 &min_v) {

  glm::vec3 p = min_v;
  
  if (plane.x >= 0)
    p.x = max_v.x;
  if (plane.y >= 0)
    p.y = max_v.y;
  if (plane.z >= 0)
    p.z = max_v.z;

  float dotProd = glm::dot(p, glm::vec3(plane));
  if (dotProd > -plane.w)
    return true;
  else
    return false;
}

bool Camera::AABBInsideFrustum(const AABB & boundary) {

  glm::vec3 min_v = boundary.centre - boundary.halfSize;
  glm::vec3 max_v = boundary.centre + boundary.halfSize;

  if (!AABBInsidePlane(near_plane, max_v, min_v))
    return false;

  if (!AABBInsidePlane(far_plane, max_v, min_v))
    return false;

  if (!AABBInsidePlane(left_plane, max_v, min_v))
    return false;

  if (!AABBInsidePlane(right_plane, max_v, min_v))
    return false;

  if (!AABBInsidePlane(bottom_plane, max_v, min_v))
    return false;

  if (!AABBInsidePlane(top_plane, max_v, min_v))
    return false;

  return true;
}
