#include "boxeslistactionbutton.h"
#include <QPainter>
#include "GUI/global.h"

BoxesListActionButton::BoxesListActionButton(QWidget * const parent) :
    QWidget(parent) {
    setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
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

void PixmapActionButton::paintEvent(QPaintEvent *) {
    if(!mPixmapChooser) return;
    const auto pix = mPixmapChooser();
    if(!pix) return;

    QPainter p(this);
    p.drawPixmap(0, 0, *pix);
    p.end();
}
