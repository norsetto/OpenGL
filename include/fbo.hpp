#pragma once
#include "glad.h"

class Fbo {
 public:
  Fbo();
  ~Fbo();

  GLboolean create(GLsizei width, GLsizei height = 0, GLenum color_type = GL_RGB8, GLboolean stencil = false);
  GLboolean create2(GLsizei width, GLsizei height = 0, GLenum color_type1 = GL_RGB8, GLenum color_type2 = GL_RGB8, GLboolean stencil = false);
  void free();
  GLuint color();
  GLuint color1();
  GLuint depth();
  GLuint buffer();

 private:
  GLuint buffer_id = 0;
  GLuint sdrd_id = 0;
  GLuint color_texture  = 0;
  GLuint color_texture1 = 0;
  GLuint depth_texture = 0;
};

#ifdef GL_DEBUG
#include <iostream>
#endif
  
Fbo::Fbo(): buffer_id(0),
            color_texture(0),
            color_texture1(0),
            depth_texture(0) {}

Fbo::~Fbo() {}

GLboolean Fbo::create(GLsizei width, GLsizei height, GLenum color_type, GLboolean stencil) {
  glGenFramebuffers(1, &buffer_id);
  glBindFramebuffer(GL_FRAMEBUFFER, buffer_id);

  if (height == 0)
    height = width;
    
  glGenTextures(1, &color_texture);
  glBindTexture(GL_TEXTURE_2D, color_texture);
  glTexStorage2D(GL_TEXTURE_2D, 1, color_type, width, height);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

  if (stencil)
    {
      glGenRenderbuffers(1, &sdrd_id);
      glBindRenderbuffer(GL_RENDERBUFFER, sdrd_id);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sdrd_id);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sdrd_id);
    }
  else
    {
      glGenTextures(1, &depth_texture);
      glBindTexture(GL_TEXTURE_2D, depth_texture);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }
    
  static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, draw_buffers);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
#ifdef GL_DEBUG
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "<W> Framebuffer not complete" << std::endl;
    return GL_FALSE;
  }
#endif

  return GL_TRUE;
}

GLboolean Fbo::create2(GLsizei width, GLsizei height, GLenum color_type, GLenum color_type1,
			 GLboolean stencil) {
  glGenFramebuffers(1, &buffer_id);
  glBindFramebuffer(GL_FRAMEBUFFER, buffer_id);

  if (height == 0)
    height = width;
    
  glGenTextures(1, &color_texture);
  glBindTexture(GL_TEXTURE_2D, color_texture);
  glTexStorage2D(GL_TEXTURE_2D, 1, color_type, width, height);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

  glGenTextures(1, &color_texture1);
  glBindTexture(GL_TEXTURE_2D, color_texture1);
  glTexStorage2D(GL_TEXTURE_2D, 1, color_type1, width, height);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, color_texture1, 0);

  if (stencil)
    {
      glGenRenderbuffers(1, &sdrd_id);
      glBindRenderbuffer(GL_RENDERBUFFER, sdrd_id);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sdrd_id);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sdrd_id);
    }
  else
    {
      glGenTextures(1, &depth_texture);
      glBindTexture(GL_TEXTURE_2D, depth_texture);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }
    
  static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, draw_buffers);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
#ifdef GL_DEBUG
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "<W> Framebuffer not complete" << std::endl;
    return GL_FALSE;
  }
#endif

  return GL_TRUE;
}

void Fbo::free() {
  glDeleteTextures(1, &color_texture);
  if (color_texture1)
    glDeleteTextures(1, &color_texture1);
  if (depth_texture)
    glDeleteTextures(1, &depth_texture);
  if (sdrd_id)
    glDeleteRenderbuffers(1, &sdrd_id);
  glDeleteFramebuffers(1, &buffer_id);
}
  
GLuint Fbo::color()
{
  return color_texture;
}

GLuint Fbo::color1()
{
  return color_texture1;
}

GLuint Fbo::depth()
{
  return depth_texture;
}

GLuint Fbo::buffer()
{
  return buffer_id;
}


