#include "verticalwidgetsstack.h"

void moveY(const int y, QWidget * const widget) {
    widget->move(widget->x(), y);
}

VWidgetStack::VWidgetStack(QWidget * const parent) :
    QWidget(parent) {
    setThis(this);
}

void moveX(const int x, QWidget * const widget) {
    widget->move(x, widget->y());
}

HWidgetStack::HWidgetStack(QWidget * const parent) :
    QWidget(parent) {
    setThis(this);
}
