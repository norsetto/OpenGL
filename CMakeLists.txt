cmake_minimum_required (VERSION 3.9)

project (openGL_demos VERSION 0.0.0 DESCRIPTION "Various OpenGL demos")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(RUN_DIR ${PROJECT_SOURCE_DIR}/bin)

configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")

set(DEMOS
  butterfly
  cdlod_tess
  qt-culling
  water
  )

# Libraries
find_package(PkgConfig REQUIRED)

pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

find_package (OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

pkg_search_module(TINYXML2 REQUIRED tinyxml2)
include_directories(${TINYXML2_INCLUDE_DIRS})

pkg_search_module(ASSIMP REQUIRED assimp)
include_directories(${ASSIMP_INCLUDE_DIRS})

set(COMMON_LIBS image ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${TINYXML2_LIBRARIES} ${ASSIMP_LIBRARIES})

add_library(image SHARED src/image/image.cpp glad/glad.c)

find_package (PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
target_link_libraries (image ${PNG_LIBRARIES})

foreach(DEMO ${DEMOS})
  add_executable(${DEMO} src/${DEMO}/${DEMO}.cpp glad/glad.c)
  target_link_libraries(${DEMO} ${COMMON_LIBS})
endforeach(DEMO)

include_directories(include)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -std=c++14 -march=native -O2 -flto")
  set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -Wall -std=c++14 -march=native -O0 -flto -DGL_DEBUG")
endif()
