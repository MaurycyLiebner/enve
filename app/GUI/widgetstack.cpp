#include "widgetstack.h"

void gMoveY(const int y, QWidget * const widget) {
    widget->move(0, y);
}

VWidgetStack::VWidgetStack(QWidget * const parent) :
    QWidget(parent), WidgetStackBase<V_STACK_TMPL>() {
    setThis(this);
}

void gMoveX(const int x, QWidget * const widget) {
    widget->move(x, 0);
}

HWidgetStack::HWidgetStack(QWidget * const parent) :
    QWidget(parent), WidgetStackBase<H_STACK_TMPL>() {
    setThis(this);
}

bool gReplaceWidget(QWidget * const from, QWidget * const to,
                    bool * const centralWid) {
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
        window->takeCentralWidget();
        window->setCentralWidget(to);
    } else if(layout) {
        layout->replaceWidget(from, to, Qt::FindDirectChildrenOnly);
    } else if(vStack) {
        vStack->replaceWidget(from, to);
    } else { // hStack
        hStack->replaceWidget(from, to);
    }
    if(centralWid) *centralWid = window;
    return true;
}
