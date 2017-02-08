#include "singlewidget.h"
#include "singlewidgetabstraction.h"

SingleWidget::SingleWidget(QWidget *parent) :
    QWidget(parent) {
    setFixedHeight(20);
}

void SingleWidget::setTargetAbstraction(SingleWidgetAbstraction *abs) {
    mTarget = abs;
    if(abs == NULL) {
        hide();
    } else {
        if(isHidden()) show();
    }
}
