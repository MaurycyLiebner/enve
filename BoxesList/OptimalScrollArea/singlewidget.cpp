#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "scrollwidgetvisiblepart.h"
#include "global.h"

SingleWidget::SingleWidget(ScrollWidgetVisiblePart *parent) :
    QWidget(parent) {
    mParent = parent;
    setFixedHeight(MIN_WIDGET_HEIGHT);
}

void SingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    mTarget = abs;
    if(abs == NULL) {
        hide();
    } else {
        if(isHidden()) show();
    }
}
