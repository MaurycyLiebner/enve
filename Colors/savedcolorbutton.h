#ifndef SAVEDCOLORBUTTON_H
#define SAVEDCOLORBUTTON_H

#include <QWidget>
#include <QMouseEvent>
#include "color.h"

class SavedColorButton : public QWidget
{
    Q_OBJECT
public:
    explicit SavedColorButton(Color color_t, QWidget *parent = 0);

    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
private:
    Color color;

signals:
    void colorButtonPressed(Color);
public slots:
};

#endif // SAVEDCOLORBUTTON_H
