#include "boxeslistactionbutton.h"
#include <QPainter>
#include "global.h"

BoxesListActionButton::BoxesListActionButton(
        QWidget *parent) :
    QWidget(parent) {
    setFixedSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
}

void BoxesListActionButton::mousePressEvent(QMouseEvent *) {
    emit pressed();
}

void BoxesListActionButton::enterEvent(QEvent *) {
    mHover = true;
    update();
}

void BoxesListActionButton::leaveEvent(QEvent *) {
    mHover = false;
    update();
}
