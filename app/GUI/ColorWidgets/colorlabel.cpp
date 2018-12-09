#include "colorlabel.h"
#include "GUI/ColorWidgets/helpers.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWidget>

ColorLabel::ColorLabel(QWidget *parent) : ColorWidget(parent) {
    //setMinimumSize(80, 20);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void ColorLabel::mousePressEvent(QMouseEvent *e) {
    Q_UNUSED(e);
}

void ColorLabel::setAlpha(GLfloat alpha_t) {
    alpha = alpha_t;
    update();
}

void ColorLabel::paintGL() {

}
