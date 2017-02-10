#include "boxeslistactionbutton.h"
#include <QPainter>

BoxesListActionButton::BoxesListActionButton(
        QWidget *parent) :
    QWidget(parent) {
    setFixedSize(20, 20);
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
