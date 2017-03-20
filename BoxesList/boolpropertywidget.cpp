#include "boolpropertywidget.h"
#include "Properties/boolproperty.h"
#include "mainwindow.h"

BoolPropertyWidget::BoolPropertyWidget(QWidget *parent) :
    QWidget(parent) {
    setFixedSize(20, 20);
}

void BoolPropertyWidget::setTarget(BoolProperty *property) {
    mTarget = property;
}

void BoolPropertyWidget::mousePressEvent(QMouseEvent *) {
    mTarget->setValue(!mTarget->getValue());
    MainWindow::getInstance()->callUpdateSchedulers();
}

void BoolPropertyWidget::paintEvent(QPaintEvent *) {
    if(mTarget == NULL) return;
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    if(mHovered) {
        p.setPen(Qt::white);
    } else {
        p.setPen(Qt::black);
    }

    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5., 5.);

    if(mTarget->getValue()) {
        p.setPen(QPen(Qt::black, 2.));
        p.drawLine(QPoint(6, height()/2), QPoint(width()/2, height() - 6));
        p.drawLine(QPoint(width()/2, height() - 6), QPoint(width() - 6, 6));
    }

    p.end();
}

void BoolPropertyWidget::enterEvent(QEvent *) {
    mHovered = true;
    repaint();
}

void BoolPropertyWidget::leaveEvent(QEvent *) {
    mHovered = false;
    repaint();
}

