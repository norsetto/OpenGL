#CONFIG += debug console
#DEFINES += "GL_DEBUG"
  
QT += core opengl gui widgets
INCLUDEPATH += $$PWD

LIBS += -limage

INCLUDEPATH += $$PWD
SOURCES += $$PWD/window.cpp \
           $$PWD/openglwindow.cpp \
           $$PWD/camera.cpp \
               
HEADERS += $$PWD/window.h \
           $$PWD/openglwindow.h \
           $$PWD/camera.hpp \
               
SOURCES += \
    testgl.cpp

target.path = $$PWD
INSTALLS += target
