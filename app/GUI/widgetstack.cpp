#include "widgetstack.h"

void moveY(const int y, QWidget * const widget) {
    widget->move(widget->x(), y);
}

VWidgetStack::VWidgetStack(QWidget * const parent) :
    QWidget(parent), WidgetStackBase<V_STACK_TMPL>(QBoxLayout::TopToBottom) {
    setThis(this);
}

void moveX(const int x, QWidget * const widget) {
    widget->move(x, widget->y());
}

HWidgetStack::HWidgetStack(QWidget * const parent) :
    QWidget(parent), WidgetStackBase<H_STACK_TMPL>(QBoxLayout::LeftToRight) {
    setThis(this);
}
