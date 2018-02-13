/*
 *
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#define GLFW_INCLUDE_GLCOREARB 1

#include <glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#ifdef GL_DEBUG
#include <iostream>
#endif

class Application {
 private:
#ifdef GL_DEBUG
  static void APIENTRY debug_callback(GLenum source,
				      GLenum type,
				      GLuint id,
				      GLenum severity,
				      GLsizei length,
				      const GLchar* message,
				      GLvoid* userParam)
  {
    reinterpret_cast<Application *>(userParam)->onDebugMessage(source, type, id, severity, length, message);
  }
#endif

 public:
  Application() {}
  virtual ~Application() {}
  virtual void run(Application* myapp, int argc, const char ** argv)
  {
    app = myapp;
    info.argc = argc;
    info.argv = argv;
	    
    if (!glfwInit()) {
      fprintf(stderr, "Failed to initialize GLFW\n");
      return;
    }

    init();
			
    monitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(monitor);
	    
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
#ifndef GL_DEBUG
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.majorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.minorVersion);
#ifdef GL_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, info.samples);
	    
    window = glfwCreateWindow(mode->width, mode->height, "", monitor, NULL);
    if (!window) {
      fprintf(stderr, "Failed to open window\n");
      glfwTerminate();
      return;
    }

    info.window = window;
    info.windowHeight = mode->height;
    info.windowWidth = mode->width;
    info.aspect = (GLfloat)info.windowWidth / (GLfloat)info.windowHeight;

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouseButton_callback);
    glfwSetCursorPosCallback(window, mouseMove_callback);
    glfwSetScrollCallback(window, mouseWheel_callback);
    if (!info.show_cursor)
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    
#ifdef GL_DEBUG
    fprintf(stderr, "OpenGL vendor string: %s\n", (char *)glGetString(GL_VENDOR));
    fprintf(stderr, "OpenGL renderer string: %s\n", (char *)glGetString(GL_RENDERER));
    fprintf(stderr, "OpenGL version string: %s\n", (char *)glGetString(GL_VERSION));

    if (GLAD_GL_ATI_meminfo) {
      int i[4];

      fprintf(stderr, "Memory info (GL_ATI_meminfo):\n");

      glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, i);
      fprintf(stderr, "    VBO free memory - total: %u MB, largest block: %u MB\n",
	      i[0] / 1024, i[1] / 1024);
      fprintf(stderr, "    VBO free aux. memory - total: %u MB, largest block: %u MB\n",
	      i[2] / 1024, i[3] / 1024);

      glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, i);
      fprintf(stderr, "    Texture free memory - total: %u MB, largest block: %u MB\n",
	      i[0] / 1024, i[1] / 1024);
      fprintf(stderr, "    Texture free aux. memory - total: %u MB, largest block: %u MB\n",
	      i[2] / 1024, i[3] / 1024);

      glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, i);
      fprintf(stderr, "    Renderbuffer free memory - total: %u MB, largest block: %u MB\n",
	      i[0] / 1024, i[1] / 1024);
      fprintf(stderr, "    Renderbuffer free aux. memory - total: %u MB, largest block: %u MB\n",
	      i[2] / 1024, i[3] / 1024);
    }

    if (GLAD_GL_NVX_gpu_memory_info) {
      int i;

      fprintf(stderr, "Memory info (GL_NVX_gpu_memory_info):\n");

      glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &i);
      fprintf(stderr, "    Dedicated video memory: %u MB\n", i / 1024);

      glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &i);
      fprintf(stderr, "    Total available memory: %u MB\n", i / 1024);

      glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &i);
      fprintf(stderr, "    Currently available dedicated video memory: %u MB\n", i / 1024);
    }

    if (GLAD_GL_VERSION_4_3)
      {
	glDebugMessageCallback((GLDEBUGPROC)debug_callback, this);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      }
    else if (GLAD_GL_ARB_debug_output)
      {
	glDebugMessageCallbackARB((GLDEBUGPROC)debug_callback, this);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
      }
#endif	    
    startup();

    while (!glfwWindowShouldClose(window)) {
      render(glfwGetTime());
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
  }

  virtual void init() {
    info.majorVersion = 4;
    info.minorVersion = 3;
    info.samples = 0;
    info.show_cursor = false;
  }
		   
  virtual void startup() {
  }

  virtual void render(double currentTime) {
  }

  virtual void shutdown() {
  }

  virtual void onKey(int key, int action, int mod) {
  }

  virtual void onMouseButton(int button, int action) {
  }

  virtual void onMouseMove(int x, int y) {
  }

  virtual void onMouseWheel(double pos) {
  }

#ifdef GL_DEBUG
  virtual void onDebugMessage(GLenum source,
			      GLenum type,
			      GLuint id,
			      GLenum severity,
			      GLsizei length,
			      const GLchar* message) {

    std::cerr << "Debug message with source: ";
    switch (source) {
    case GL_DEBUG_SOURCE_API:
      std::cerr << "OPENGL API";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      std::cerr << "SHADER COMPILER";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      std::cerr << "WINDOW SYSTEM";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      std::cerr << "THIRD PARTY";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      std::cerr << "THIS APPLICATION";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      std::cerr << "OTHER";
      break;
    }
    std::cerr << std::endl;
	    
    std::cerr << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      std::cerr << "ERROR";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      std::cerr << "DEPRECATED BEHAVIOR";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      std::cerr << "UNDEFINED BEHAVIOR";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      std::cerr << "PORTABILITY";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      std::cerr << "PERFORMANCE";
      break;
    case GL_DEBUG_TYPE_OTHER:
      std::cerr << "OTHER";
      break;
    }
    std::cerr << std::endl;
	    
    std::cerr << "id: " << id << std::endl;
	    
    std::cerr << "severity: ";
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
      std::cerr << "LOW";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      std::cerr << "MEDIUM";
      break;
    case GL_DEBUG_SEVERITY_HIGH:
      std::cerr << "HIGH";
      break;
    }
    std::cerr << std::endl;
	    
    std::cerr << "message: " << message << std::endl;
  }
#endif
 public:
  struct APPINFO {
    GLFWwindow* window;
    GLint windowHeight;
    GLint windowWidth;
    GLint majorVersion;
    GLint minorVersion;
    GLfloat aspect;
    GLint samples;
    bool show_cursor;
    int argc;
    const char **argv;
  };

 protected:
  APPINFO info;
  static Application* app;
  GLFWmonitor* monitor; 
  const GLFWvidmode* mode;
  GLFWwindow* window;

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    app->onKey(key, action, mods);
  }

  static void mouseButton_callback(GLFWwindow* window, int button, int action, int mods) {
    app->onMouseButton(button, action);
  }

  static void mouseMove_callback(GLFWwindow* window, double x, double y) {
    app->onMouseMove(static_cast<int>(x), static_cast<int>(y));
  }

  static void mouseWheel_callback(GLFWwindow* window, double xoffset, double yoffset) {
    app->onMouseWheel(yoffset);
  }
};

#define DECLARE_MAIN(a)				\
  int main(int argc, const char ** argv)	\
  {						\
    a *app = new a;				\
    app->run(app, argc, argv);			\
    delete app;					\
    return 0;					\
  }
#endif /* __MAIN_H__ */
