#include "widgetstack.h"

void gMoveX(const int x, QWidget * const widget) {
    widget->move(x, 0);
}

void gResizeW(QWidget * const widget, const int w) {
    widget->resize(w, widget->height());
}

void gMoveY(const int y, QWidget * const widget) {
    widget->move(0, y);
}

void gResizeH(QWidget * const widget, const int h) {
    widget->resize(widget->width(), h);
}

VWidgetStack::VWidgetStack(QWidget * const parent) :
    QWidget(parent), WidgetStackBase<V_STACK_TMPL>() {
    setThis(this);
}

HWidgetStack::HWidgetStack(QWidget * const parent) :
    QWidget(parent), WidgetStackBase<H_STACK_TMPL>() {
    setThis(this);
}

bool gReplaceWidget(QWidget * const from, QWidget * const to,
                    bool * const centralWid) {
    const QSize size = from->size();
    const auto fromParent = from->parentWidget();
    if(!fromParent) return false;
    const auto layout = fromParent->layout();
    const auto window = qobject_cast<QMainWindow*>(fromParent);
    VWidgetStack * vStack = nullptr;
    HWidgetStack * hStack = nullptr;
    if(!layout && !window) {
        vStack = dynamic_cast<VWidgetStack*>(fromParent);
        if(!vStack) hStack = dynamic_cast<HWidgetStack*>(fromParent);
        if(!vStack && !hStack) return false;
    }
    if(window) {
        window->takeCentralWidget()->setParent(nullptr);
        window->setCentralWidget(to);
    } else if(layout) {
        layout->replaceWidget(from, to, Qt::FindDirectChildrenOnly);
    } else if(vStack) {
        vStack->replaceWidget(from, to);
    } else if(hStack) { // hStack
        hStack->replaceWidget(from, to);
    } else return false;
    if(centralWid) *centralWid = window || layout;
    to->resize(size.width(), size.height());
    return true;
}
