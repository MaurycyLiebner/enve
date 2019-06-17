#include "boolpropertywidget.h"
#include "Properties/boolproperty.h"
#include "GUI/mainwindow.h"
#include "global.h"

BoolPropertyWidget::BoolPropertyWidget(QWidget *parent) :
    QWidget(parent) {
    setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
}

void BoolPropertyWidget::setTarget(BoolProperty *property) {
    mTarget = property;
    mTargetContainer = nullptr;
}

void BoolPropertyWidget::setTarget(BoolPropertyContainer *property) {
    mTargetContainer = property;
    mTarget = nullptr;
}

void BoolPropertyWidget::mousePressEvent(QMouseEvent *) {
    if(mTargetContainer) {
        mTargetContainer->setValue(!mTargetContainer->getValue());
    }
    if(mTarget) {
        mTarget->setValue(!mTarget->getValue());
    }
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void BoolPropertyWidget::paintEvent(QPaintEvent *) {
    if(mTarget == nullptr && mTargetContainer == nullptr) return;
    QPainter p(this);
    if(mTarget) {
        if(mTarget->SWT_isDisabled()) p.setOpacity(.5);
    } else if(mTargetContainer) {
        if(mTargetContainer->SWT_isDisabled()) p.setOpacity(.5);
    }

    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    if(mHovered) {
        p.setPen(Qt::white);
    } else {
        p.setPen(Qt::black);
    }

    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5., 5.);

    bool valueT;
    if(!mTarget) {
        valueT = mTargetContainer->getValue();
    } else {
        valueT = mTarget->getValue();
    }
    if(valueT) {
        p.setPen(QPen(Qt::black, 2.));
        p.drawLine(QPoint(6, height()/2), QPoint(width()/2, height() - 6));
        p.drawLine(QPoint(width()/2, height() - 6), QPoint(width() - 6, 6));
    }

    p.end();
}

void BoolPropertyWidget::enterEvent(QEvent *) {
    mHovered = true;
    update();
}

void BoolPropertyWidget::leaveEvent(QEvent *) {
    mHovered = false;
    update();
}

