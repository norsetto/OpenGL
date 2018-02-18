#include <QOpenGLWidget>
#include <QApplication>
#include <QDesktopWidget>

#include "openglwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef GL_DEBUG
    format.setOption(QSurfaceFormat::DebugContext);
#endif
    QSurfaceFormat::setDefaultFormat(format);

    MyGLWidget widget;
    widget.resize(QSize(800, 600));
    int desktopArea = QApplication::desktop()->width() * QApplication::desktop()->height();
    int widgetArea = widget.width() * widget.height();

    widget.setWindowTitle("OpenGL with Qt");

    if (((float)widgetArea / (float)desktopArea) < 0.75f)
        widget.show();
    else
        widget.showMaximized();

    return app.exec();
}
