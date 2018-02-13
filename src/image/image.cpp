#include <glad.h>
#include <image.hpp>

#include <png.h>

#include <iostream>
#include <fstream>
#include <string>
#include <exception>

Image::Image() {
  m_data = nullptr;
  m_components = 3;
  m_internalformat = GL_RGB8;
  m_format = GL_RGB;
  m_height = 0;
  m_width = 0;
  m_numMipMaps = 0;
}

Image::Image(GLint width, GLint height, GLint components, GLubyte *data) {
  m_data = data;
  m_components = components;
  switch(components) {
  case 1:
    m_format = GL_RED;
    m_internalformat = GL_R8;
    break;
  case 2:
    m_format = GL_RG;
    m_internalformat = GL_RG8;
    break;
  case 3:
    m_internalformat = GL_RGB8;
    m_format = GL_RGB;
    break;
  case 4:
    m_internalformat = GL_RGBA8;
    m_format = GL_RGBA;
    break;
  }
  m_height = height;
  m_width = width;
  m_numMipMaps = 0;
}

Image::~Image() {
  if (m_data) delete [] m_data;
}

GLint Image::width(void) {return m_width;}
GLint Image::height(void) {return m_height;}
GLenum Image::internalformat(void) {return m_internalformat;}
GLenum Image::format(void) {return m_format;}
GLint Image::components(void) {return m_components;}
GLubyte *Image::data(void) {return m_data;}

//Read data function used by libpng with istreams
void read_data_fn(png_structp png_ptr, png_bytep data, png_size_t length) {

    // set IO pointer from the read struct
    png_voidp fs = png_get_io_ptr(png_ptr);

    // read 'length' bytes into 'data'
    ((std::istream *)fs)->read(reinterpret_cast<char *>(data), length);
}

#define PNGSIGSIZE 8

void Image::loadPng(const std::string filename) {

  std::ifstream fs;
  png_bytepp row_pointers = nullptr;

  try {
#ifdef GL_DEBUG
    std::string type;
#endif
    png_bytep header = new png_byte [PNGSIGSIZE];
    png_structp png_ptr;
    png_infop info_ptr, end_info;

    fs.open(filename.c_str(), std::ios::binary);
    if (fs.fail()) throw(filename);

    // read the header
    fs.read(reinterpret_cast<char *>(header), PNGSIGSIZE);

    // check if it is indeed a PNG file
    if (png_sig_cmp(header, 0, PNGSIGSIZE)) throw(filename);

    // create png read struct
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
      throw "png_create_read_struct failed";
    }

    // create png info struct
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      throw "png_create_info_struct failed";
    }

    // create png end info struct
    end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	throw "png_create_info_struct failed";
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      throw "error from libpng";
    }

    // let libpng know you already read the signature
    png_set_sig_bytes(png_ptr, PNGSIGSIZE);

    // init png reading
    png_set_read_fn(png_ptr, reinterpret_cast<png_voidp>(&fs), read_data_fn);
      
    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    png_uint_32 width, height;
    png_byte colour_type, bit_depth;

    // get info about png
    width       = png_get_image_width(png_ptr, info_ptr);
    height      = png_get_image_height(png_ptr, info_ptr);
    colour_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth   = png_get_bit_depth(png_ptr, info_ptr);

    m_width = static_cast<GLint>(width);
    m_height = static_cast<GLint>(height);

    switch(colour_type) {
    case PNG_COLOR_TYPE_GRAY:
#ifdef GL_DEBUG
      type = "PNG_COLOR_TYPE_GRAY";
#endif
      if (bit_depth == 8) {
	m_format = GL_RED;
	m_internalformat = GL_R8;
	m_components = 1;
      }
      else if (bit_depth == 16) {
	m_format = GL_RG;
	m_internalformat = GL_RG8;
	m_components = 2;
      }
      else
	throw "not a valid bit depth";
      break;
    case PNG_COLOR_TYPE_RGB:
#ifdef GL_DEBUG
      type = "PNG_COLOR_TYPE_RGB";
#endif
      m_format = GL_RGB;
      if (bit_depth == 8)
	m_internalformat = GL_RGB8;
      else
	throw "not a valid bit depth";
      m_components = 3;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
#ifdef GL_DEBUG
      type = "PNG_COLOR_TYPE_RGB_ALPHA";
#endif
      m_format = GL_RGBA;
      if (bit_depth == 8)
	m_internalformat = GL_RGBA8;
      else
	throw "not a valid bit depth";
      m_components = 4;
      break;
    default:
      throw "colour type not valid";
    }

    // update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // row size in bytes.
    png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // allocate data buffer
    if (m_data) delete [] m_data;
    m_data = new png_byte [rowbytes * height];
    
#ifdef GL_DEBUG
    std::cout << filename << ": " << m_width << "x" << m_height << "@" << (int)bit_depth << "bits type: " << type << std::endl;
#endif
    
    // row_pointers to read image_data
    row_pointers = new png_bytep [height];

    // set the individual row_pointers to point at the correct offsets of image_data
    for (png_uint_32 row = 0; row < height; row++) {
        row_pointers[height - 1 - row] = m_data + row * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // cleanup
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    
  } catch(std::string error_message) {
    std::cerr << "error: " << error_message << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  // cleanup
  if (row_pointers) delete [] row_pointers;
  fs.close();
}

//Write data function used by libpng with ostreams
void write_data_fn(png_structp png_ptr, png_bytep data, png_size_t length) {

    // set IO pointer from the write struct
    png_voidp fs = png_get_io_ptr(png_ptr);

    // write 'length' bytes into 'data'
    ((std::ostream *)fs)->write(reinterpret_cast<char *>(data), length);
}

//Flush data function used by libpng with ostreams
void output_flush_fn(png_structp png_ptr) {

    // set IO pointer from the write struct
    png_voidp fs = png_get_io_ptr(png_ptr);

    // flush ostream
    ((std::ostream *)fs)->flush();
}

void Image::savePng(std::string filename, int compression_level) {

  std::ofstream fs;
  png_bytepp row_pointers = nullptr;

  if (filename.substr(filename.find_last_of(".") + 1) != "png") filename.append(".png");
  
  try {
    png_structp png_ptr;
    png_infop info_ptr;

    fs.open(filename.c_str(), std::ios::binary);
    if (fs.fail()) throw(filename);

    // create png write struct
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
      throw "png_create_write_struct failed";
    }

    // create png info struct
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      throw "png_create_info_struct failed";
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      throw "error from libpng";
    }

    // init png writing
    png_set_write_fn(png_ptr, reinterpret_cast<png_voidp>(&fs), write_data_fn, output_flush_fn);

    // set compression level TODO: make this a user variable!
    png_set_compression_level(png_ptr, compression_level);
      
    // Output is 8bit depth, RGB(a) format.
    png_byte PNG_COLOR_TYPE;
    if (m_components == 3) PNG_COLOR_TYPE = PNG_COLOR_TYPE_RGB;
    else if (m_components == 4) PNG_COLOR_TYPE = PNG_COLOR_TYPE_RGBA; else {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      throw "invalid number of components";
    }
    
    png_set_IHDR(
		 png_ptr,
		 info_ptr,
		 m_width, m_height,
		 8,
		 PNG_COLOR_TYPE,
		 PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT,
		 PNG_FILTER_TYPE_DEFAULT
		 );

    //png_set_gAMA(png_ptr, info_ptr, gamma);
    //png_set_tIME(png_ptr, info_ptr, &time);
    //png_set_text(png_ptr, info_ptr, text_ptr, text);

    // update the png info struct.
    png_write_info(png_ptr, info_ptr);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

    // row_pointers to write image_data
    row_pointers = new png_bytep [m_height];

    // set the individual row_pointers to point at the correct offsets of image_data
    for (png_uint_32 row = 0; row < static_cast<png_uint_32>(m_height); row++) {
        row_pointers[m_height - 1 - row] = m_data + row * m_width * m_components;
    }

    // write the image_data
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);

    // cleanup
    png_destroy_write_struct(&png_ptr, &info_ptr);

  } catch(std::string error_message) {
    std::cerr << "error: " << error_message << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  // cleanup
  if (row_pointers) delete [] row_pointers;
  fs.close();
}

void Image::load(const std::string filename) {
  if (filename.substr(filename.find_last_of(".") + 1) == "png")
    loadPng(filename);
  else
    std::cerr << "error: " << filename << std::endl;
}

void Image::load(GLuint &texture_id, const std::string filename) {
  this->load(filename);
  if (m_data) this->toTexture(texture_id);
}

void Image::toTexture(GLuint &texture_id) {
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0,
	       m_internalformat,
	       m_width,
	       m_height,
	       0,
	       m_format,
	       GL_UNSIGNED_BYTE,
	       m_data);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Image::fromTexture(GLuint texture_id, std::string filename, int compression_level) {

  glBindTexture(GL_TEXTURE_2D, texture_id);

  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &m_width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_height);
  m_components = 4;
  m_format = GL_RGBA;
  m_internalformat = GL_RGBA8;
  
  if (m_data) delete [] m_data;
  m_data = new GLubyte [m_width * m_height * m_components];

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);

  if (!filename.empty()) savePng(filename, compression_level);
}

void Image::save(std::string filename, int compression_level) {
  if (!filename.empty()) savePng(filename, compression_level);
}

void Image::fromScreen(std::string filename, int compression_level) {
  GLint ViewPort[4];

  glGetIntegerv(GL_VIEWPORT, ViewPort);

  m_width  = ViewPort[2];
  m_height = ViewPort[3];
  m_components = 3;
  m_format = GL_RGB;
  m_internalformat = GL_RGB8;
  
  if (m_data) delete [] m_data;
  m_data = new GLubyte [m_width * m_height * m_components];

  glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_data);

  if (!filename.empty()) savePng(filename, compression_level);
}
