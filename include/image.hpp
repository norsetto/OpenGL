#pragma once

#include "/home/cesare/testgl/common/glad.h"

#include <string>
#include <zlib.h>

class Image {
  
public:
  
  Image();
  Image(GLint width, GLint height, GLint components, GLubyte *data);
  Image(const std::string filename) : Image() { load(filename); };
  Image(GLuint &texture_id, const std::string filename) : Image() { load(texture_id, filename); };
  Image(GLuint &texture_id, GLint width, GLint height, GLint components, GLubyte *data) : Image(width, height, components, data) { toTexture(texture_id); };
  ~Image();

  void load(const std::string filename);
  void load(GLuint &texture_id, const std::string filename);
  void save(std::string filename, int compression_level = Z_NO_COMPRESSION);
  void fromTexture(GLuint texture_id, std::string filename = std::string(), int compression_level = Z_NO_COMPRESSION);
  void toTexture(GLuint &texture_id);
  void fromScreen(std::string filename = std::string(), int compression_level = Z_NO_COMPRESSION);
  
  GLint width(void);
  GLint height(void);
  GLenum internalformat(void);
  GLenum format(void);
  GLint components(void);
  GLubyte *data(void);
  
private:

  void loadPng(const std::string filename);
  void savePng(const std::string filename, int compression_level = Z_NO_COMPRESSION);

  GLubyte *m_data;
  GLint m_components;
  GLenum m_internalformat;
  GLint m_width;
  GLint m_height;
  GLenum m_format;
  GLint m_numMipMaps;
};
