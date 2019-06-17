#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "scrollwidgetvisiblepart.h"
#include "global.h"

SingleWidget::SingleWidget(ScrollWidgetVisiblePart *parent) :
    QWidget(parent) {
    mParent = parent;
    setFixedHeight(MIN_WIDGET_DIM);
}

void SingleWidget::setTargetAbstraction(SWT_Abstraction *abs) {
    mTarget = abs;
    if(!abs) hide();
    else if(isHidden()) show();
}

SingleWidgetTarget *SingleWidget::getTarget() const {
    if(mTarget) return mTarget->getTarget();
    return nullptr;
}
