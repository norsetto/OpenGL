#include <QApplication>
#include <QSurfaceFormat>
#include <QDesktopWidget>

#include "window.h"

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

    Window window;
    window.resize(window.sizeHint());
    int desktopArea = QApplication::desktop()->width() * QApplication::desktop()->height();
    int windowArea = window.width() * window.height();

    window.setWindowTitle("OpenGL with Qt");

    if (((float)windowArea / (float)desktopArea) < 0.75f)
        window.show();
    else
        window.showMaximized();

    return app.exec();
}
