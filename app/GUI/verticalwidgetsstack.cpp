#include "verticalwidgetsstack.h"
#include <QResizeEvent>
#include <QPainter>

StackWidgetResizer::StackWidgetResizer(QWidget * const parent) :
    QWidget(parent) {
    setFixedHeight(10);
    setFixedWidth(2*MIN_WIDGET_DIM*100);
    setCursor(Qt::SplitVCursor);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    show();
}

void StackWidgetResizer::paintEvent(QPaintEvent *) {
    QPainter p(this);
    if(mPressed) {
        p.fillRect(rect().adjusted(0, 3, 0, -4), Qt::black);
    } else if(mHover) {
        p.fillRect(rect().adjusted(0, 4, 0, -4), Qt::black);
    } else {
        p.fillRect(rect().adjusted(0, 5, 0, -4), Qt::black);
    }
    p.end();
}

void StackWidgetResizer::displace(const int totDy) {
    int dY = totDy;
    const int newAboveHeight = mAboveWidget->height() + dY;
    const int newBelowHeight = mBelowWidget->height() - dY;
    if(newAboveHeight < 2*MIN_WIDGET_DIM) {
        dY = 2*MIN_WIDGET_DIM - mAboveWidget->height();
    } else if(newBelowHeight < 2*MIN_WIDGET_DIM) {
        dY = mBelowWidget->height() - 2*MIN_WIDGET_DIM;
    }
    if(totDy != dY) {
        if(totDy > 0) {
            if(mBelowResizer)
                mBelowResizer->displace(totDy - dY);
        } else {
            if(mAboveResizer)
                mAboveResizer->displace(totDy - dY);
        }
    }
    mAboveWidget->setFixedHeight(mAboveWidget->height() + dY);
    mBelowWidget->setFixedHeight(mBelowWidget->height() - dY);
    mBelowWidget->move(0, mBelowWidget->y() + dY);
    move(0, y() + dY);
}

void StackWidgetResizer::mouseMoveEvent(QMouseEvent *event) {
    displace(event->y() - mPressY);
}

void StackWidgetResizer::mousePressEvent(QMouseEvent *event) {
    mPressed = true;
    mPressY = event->y();
    update();
}

void StackWidgetResizer::mouseReleaseEvent(QMouseEvent *) {
    mPressed = false;
    update();
    emit finishedChanging();
}

void StackWidgetResizer::enterEvent(QEvent *) {
    mHover = true;
    update();
}

void StackWidgetResizer::leaveEvent(QEvent *) {
    mHover = false;
    update();
}

void StackWidgetResizer::setAboveWidget(QWidget *aboveWidget) {
    mAboveWidget = aboveWidget;
}

void StackWidgetResizer::setBelowWidget(QWidget *belowWidget) {
    mBelowWidget = belowWidget;
}

void StackWidgetResizer::setAboveResizer(
        StackWidgetResizer *aboveResizer) {
    mAboveResizer = aboveResizer;
}

void StackWidgetResizer::setBelowResizer(
        StackWidgetResizer *belowResizer) {
    mBelowResizer = belowResizer;
}

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
