#pragma once

#include <QWidget>

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *event);
};
