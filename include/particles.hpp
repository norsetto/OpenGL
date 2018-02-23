#pragma once

#include "glad.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <functional>

class Particle
{
public:

  struct PARTICLE
  {
    glm::vec3 position;
    float rotation;
    glm::vec4 color;
    float size;
    GLuint type;
    glm::vec3 velocity;
    float rotationRate;
    
    PARTICLE(glm::vec3 position = {},
	     float rotation = 0.0f,
	     glm::vec4 color = glm::vec4(1.0f),
	     glm::vec3 velocity = {},
	     float rotationRate = 0.0f,
	     float size = 0.1f,
	     GLuint type = 0) :
      position(position),
      rotation(rotation),
      color(color),
      size(size),
      type(type),
      velocity(velocity),
      rotationRate(rotationRate)
    {};
  };
  
public:

  Particle(GLuint program = 0, GLuint texture1 = 0, GLuint texture2 = 0) : m_program(program), m_texture1(texture1), m_texture2(texture2) {};
  void startup(unsigned int num_particles, std::function<PARTICLE(void)> generator);
  virtual void update(float delta_time);
  void render(void);
  void destroy(void);
  
  void setProgram(GLuint program) {m_program = program;};
  void setTexture1(GLuint texture) {m_texture1 = texture;};
  void setTexture2(GLuint texture) {m_texture2 = texture;};
  void setAlphaRate(float alpha_rate) {m_alpha_rate = alpha_rate;};
  void setSizeRate(float size_rate) {m_size_rate = size_rate;};
  void setParticles(std::vector<PARTICLE> &particles) {m_particles = particles; };
  
protected:

  std::function<PARTICLE(void)> m_generate_particle;
  std::vector<PARTICLE> m_particles;
  GLuint m_program;
  GLuint m_texture1, m_texture2;
  float m_alpha_rate = 0.05f;
  float m_size_rate = 0.01f;
  GLuint m_vao;
  GLuint m_vbo;

};

void Particle::startup(unsigned int num_particles, std::function<PARTICLE(void)> generator)
{
  m_generate_particle = generator;
  for (unsigned int counter = 0; counter < num_particles; counter++)
    {
      m_particles.emplace_back(m_generate_particle());
    }
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(PARTICLE)*m_particles.size(), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PARTICLE), NULL);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PARTICLE), (const GLvoid *)(sizeof(glm::vec3)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(PARTICLE), (const GLvoid *)(sizeof(glm::vec3) + sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(PARTICLE), (const GLvoid *)(sizeof(glm::vec3) + sizeof(float) + sizeof(glm::vec4)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(4, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(PARTICLE), (const GLvoid *)(sizeof(glm::vec3) + sizeof(float) + sizeof(glm::vec4)+sizeof(float)));
  glEnableVertexAttribArray(4);
  glBindVertexArray(0);
}

void Particle::update(float delta_time)
{
  for (auto &particle : m_particles)
    {
      if (particle.color.w < 0.0f)
	{
	  particle = m_generate_particle();
	} else
	{
	  particle.position += particle.velocity * delta_time;
	  particle.rotation += particle.rotationRate * delta_time;
	  particle.color.w -= m_alpha_rate * delta_time;
	}
    }
}

void Particle::render(void)
{
  //Save state
  GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
  GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
  GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
  GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
  GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
  GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
  GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
  GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
  GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
  GLboolean last_enabled_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enabled_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enabled_depth_test = glIsEnabled(GL_DEPTH_TEST);
  
  //Draw			     
  if (m_program > 0)
    glUseProgram(m_program);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glActiveTexture(GL_TEXTURE0);
  glBindSampler(0, 0);
  if (m_texture1 > 0)
    glBindTexture(GL_TEXTURE_2D, m_texture1);
  glActiveTexture(GL_TEXTURE1);
  if (m_texture2 > 0)
    glBindTexture(GL_TEXTURE_2D, m_texture2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PARTICLE)*m_particles.size(), m_particles.data());
  glDrawArrays(GL_POINTS, 0, m_particles.size());

  //Restore state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindSampler(0, last_sampler);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
  if (!last_enabled_blend) glDisable(GL_BLEND);
  if (last_enabled_depth_test) glEnable(GL_DEPTH_TEST);
  if (last_enabled_cull_face) glEnable(GL_CULL_FACE);
}

void Particle::destroy(void)
{
  glDeleteProgram(m_program);
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
}
