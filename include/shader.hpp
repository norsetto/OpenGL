#pragma once

#include <cstdio>

namespace shader {

    GLuint load(const char *fileName, GLenum shader_type, bool check_errors = false) {
      GLuint result = 0;
      FILE *fp;
      size_t filesize;
      char *data;
      
      // Open shader file
      fp = fopen(fileName, "r");

      if (!fp)
	{
	  fprintf(stderr, "<!> Cannot open shader file  \"%s\"\n", fileName);
	  return 0;
	}

      // Compute shader lenght
      fseek(fp, 0, SEEK_END);
      filesize = ftell(fp);
      fseek(fp, 0, SEEK_SET);

      data = new char [filesize + 1];

      if (!data)
	{
	  fprintf(stderr, "<!> Out of memory for shader \"%s\"\n", fileName);
	  return 0;
	}

      fread(data, 1, filesize, fp);
      data[filesize] = 0;
      fclose(fp);

      result = glCreateShader(shader_type);
  
      if (!result)
	{
	  fprintf(stderr, "<!> Cannot create shader for \"%s\"\n", fileName);
	  return 0;
	}

      glShaderSource(result, 1, &data, NULL);

      delete [] data;

      glCompileShader(result);

      if (check_errors)
	{
	  GLint status = 0;
	  glGetShaderiv(result, GL_COMPILE_STATUS, &status);

	  if (!status)
	    {
	      char buffer[4096];
	      glGetShaderInfoLog(result, 4096, NULL, buffer);
	      fprintf(stderr, "<!> \"%s\": %s\n", fileName, buffer);
	      glDeleteShader(result);
	      return 0;
	    }
	}

      return result;
    }

    GLuint load(const int count, const char **fileName, GLenum shader_type, bool check_errors = false) {
      GLuint result = 0;
      FILE *fp;
      size_t filesize;
      
      char **data = new char*[count];
      if (!data)
	{
	  fprintf(stderr, "<!> Out of memory to load %d shaders starting with \"%s\"\n", count, fileName[0]);
	  return 0;
	}

      for (int i = 0; i < count; i++) {

	// Open shader file
	fp = fopen(fileName[i], "r");

	if (!fp)
	  {
	    fprintf(stderr, "<!> Cannot open shader file  \"%s\"\n", fileName[i]);
	    return 0;
	  }

	// Compute shader lenght
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data[i] = new char [filesize + 1];

	if (!data[i])
	  {
	    fprintf(stderr, "<!> Out of memory for shader \"%s\"\n", fileName[i]);
	    return 0;
	  }

	fread(data[i], 1, filesize, fp);
	data[i][filesize] = 0;
	fclose(fp);
      }
      
      result = glCreateShader(shader_type);
  
      if (!result)
	{
	  fprintf(stderr, "<!> Cannot create shader for %d shaders starting with \"%s\"\n", count, fileName[0]);
	  return 0;
	}

      glShaderSource(result, count, data, NULL);

      for (int i = 0; i < count; ++i)
	{
	  delete[] data[i];
	}
      delete [] data;

      glCompileShader(result);

      if (check_errors)
	{
	  GLint status = 0;
	  glGetShaderiv(result, GL_COMPILE_STATUS, &status);

	  if (!status)
	    {
	      char buffer[4096];
	      glGetShaderInfoLog(result, 4096, NULL, buffer);
	      fprintf(stderr, "<!> %d \"%s\": %s\n", count, fileName[0], buffer);
	      glDeleteShader(result);
	      return 0;
	    }
	}

      return result;
    }
}

namespace program {

  GLuint link_from_shaders(const GLuint * shaders,
			   int shader_count,
			   bool delete_shaders,
			   bool check_errors = false)
  {
    int i;

    GLuint program;

    program = glCreateProgram();

    for (i = 0; i < shader_count; i++)
      {
	glAttachShader(program, shaders[i]);
      }

    glLinkProgram(program);

    if (check_errors)
      {
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (!status)
	  {
	    char buffer[4096];
	    glGetProgramInfoLog(program, 4096, NULL, buffer);
	    fprintf(stderr, "<!> Error in program %d: %s\n", program, buffer);
	    glDeleteProgram(program);
	    return 0;
	  }
      }

    if (delete_shaders)
      {
	for (i = 0; i < shader_count; i++)
	  {
	    glDeleteShader(shaders[i]);
	  }
      }

    return program;
  }

}




