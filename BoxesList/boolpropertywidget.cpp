#include "boolpropertywidget.h"
#include "Properties/boolproperty.h"
#include "mainwindow.h"
#include "global.h"

BoolPropertyWidget::BoolPropertyWidget(QWidget *parent) :
    QWidget(parent) {
    setFixedSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
}

void BoolPropertyWidget::setTarget(BoolProperty *property) {
    mTarget = property;
    mTargetContainer = NULL;
}

void BoolPropertyWidget::setTarget(BoolPropertyContainer *property) {
    mTargetContainer = property;
    mTarget = NULL;
}

void BoolPropertyWidget::mousePressEvent(QMouseEvent *) {
    if(mTargetContainer != NULL) {
        mTargetContainer->setValue(!mTargetContainer->getValue());
    }
    if(mTarget != NULL) {
        mTarget->setValue(!mTarget->getValue());
    }
    MainWindow::getInstance()->callUpdateSchedulers();
}

void BoolPropertyWidget::paintEvent(QPaintEvent *) {
    if(mTarget == NULL && mTargetContainer == NULL) return;
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    if(mHovered) {
        p.setPen(Qt::white);
    } else {
        p.setPen(Qt::black);
    }

    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5., 5.);

    bool valueT;
    if(mTarget == NULL) {
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

