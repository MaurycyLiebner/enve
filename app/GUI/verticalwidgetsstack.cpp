#include "verticalwidgetsstack.h"
#include <QResizeEvent>
#include <QPainter>
#include "global.h"

BoxesListKeysViewWidgetResizer::BoxesListKeysViewWidgetResizer(
                                                QWidget *parent) :
    QWidget(parent) {
    setFixedHeight(10);
    setFixedWidth(2*MIN_WIDGET_HEIGHT*100);
    setCursor(Qt::SplitVCursor);
    show();
}

void BoxesListKeysViewWidgetResizer::paintEvent(QPaintEvent *) {
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
#include <QDebug>
void BoxesListKeysViewWidgetResizer::displace(int totDy) {
    int dY = totDy;
    int newAboveHeight = mAboveWidget->height() + dY;
    int newBelowHeight = mBelowWidget->height() - dY;
    if(newAboveHeight < 2*MIN_WIDGET_HEIGHT) {
        dY = 2*MIN_WIDGET_HEIGHT - mAboveWidget->height();
    } else if(newBelowHeight < 2*MIN_WIDGET_HEIGHT) {
        dY = mBelowWidget->height() - 2*MIN_WIDGET_HEIGHT;
    }
    qDebug() << totDy << dY;
    if(totDy != dY) {
        if(totDy > 0) {
            if(mBelowResizer) {
                mBelowResizer->displace(totDy - dY);
            }
        } else {
            if(mAboveResizer) {
                mAboveResizer->displace(totDy - dY);
            }
        }
    }
    mAboveWidget->setFixedHeight(mAboveWidget->height() + dY);
    mBelowWidget->setFixedHeight(mBelowWidget->height() - dY);
    mBelowWidget->move(0, mBelowWidget->y() + dY);
    move(0, y() + dY);
}

void BoxesListKeysViewWidgetResizer::mouseMoveEvent(QMouseEvent *event) {
    displace(event->y() - mPressY);
}

void BoxesListKeysViewWidgetResizer::mousePressEvent(QMouseEvent *event) {
    mPressed = true;
    mPressY = event->y();
    update();
}

void BoxesListKeysViewWidgetResizer::mouseReleaseEvent(QMouseEvent *) {
    mPressed = false;
    update();
    emit finishedChanging();
}

void BoxesListKeysViewWidgetResizer::enterEvent(QEvent *) {
    mHover = true;
    update();
}

void BoxesListKeysViewWidgetResizer::leaveEvent(QEvent *) {
    mHover = false;
    update();
}

void BoxesListKeysViewWidgetResizer::setAboveWidget(QWidget *aboveWidget) {
    mAboveWidget = aboveWidget;
}

void BoxesListKeysViewWidgetResizer::setBelowWidget(QWidget *belowWidget) {
    mBelowWidget = belowWidget;
}

void BoxesListKeysViewWidgetResizer::setAboveResizer(
        BoxesListKeysViewWidgetResizer *aboveResizer) {
    mAboveResizer = aboveResizer;
}

void BoxesListKeysViewWidgetResizer::setBelowResizer(
        BoxesListKeysViewWidgetResizer *belowResizer) {
    mBelowResizer = belowResizer;
}

VerticalWidgetsStack::VerticalWidgetsStack(QWidget *parent) :
    QWidget(parent) {

}
#include <QDebug>
void VerticalWidgetsStack::updateSizesAndPositions() {
    int newHeight = height();
    if(mWidgets.isEmpty()) {
        return;
    }

    int accumulated = 0;
    int wCount = mWidgets.count() - 1;
    for(int i = 0; i < wCount; i++) {
        QWidget *widget = mWidgets.at(i);
        widget->move(0, accumulated);
        int newWidgetHeight = mHeightPercent.at(i)*newHeight;
        if(newWidgetHeight < 2*MIN_WIDGET_HEIGHT) newWidgetHeight = 2*MIN_WIDGET_HEIGHT;
        widget->setFixedHeight(newWidgetHeight);
        widget->setFixedWidth(width());
        accumulated += newWidgetHeight;
        if(i < mResizers.count()) {
            mResizers.at(i)->move(0, widget->y() + widget->height() - 5);
        }
    }
    QWidget *lastWidget = mWidgets.last();
    lastWidget->move(0, accumulated);
    int newWidgetHeight = newHeight - accumulated;
    lastWidget->setFixedHeight(newWidgetHeight);
    lastWidget->setFixedWidth(width());
}

void VerticalWidgetsStack::updateResizers() {
    int resId = 0;
    BoxesListKeysViewWidgetResizer *lastRes = nullptr;
    for(int i = 0; i < mWidgets.count() - 1; i++) {
        QWidget *currWid = mWidgets.at(i);
        QWidget *nextWid = mWidgets.at(i + 1);

        BoxesListKeysViewWidgetResizer *res;
        if(resId < mResizers.count()) {
            res = mResizers.at(resId);
        } else {
            res = new BoxesListKeysViewWidgetResizer(this);
            mResizers << res;
            connect(res, SIGNAL(finishedChanging()),
                    this, SLOT(updatePercent()));
        }
        res->setAboveWidget(currWid);
        res->setBelowWidget(nextWid);
        resId++;
        res->move(0, nextWid->y());
        res->setAboveResizer(lastRes);
        if(lastRes) {
            lastRes->setBelowResizer(res);
        }
        lastRes = res;
    }
    if(lastRes) {
        lastRes->setBelowResizer(nullptr);
    }
    for(int i = resId; i < mResizers.count(); i++) {
        delete mResizers.takeAt(i);
    }
}

int VerticalWidgetsStack::getIdOf(QWidget *idOf) {
    return mWidgets.indexOf(idOf);
}

void VerticalWidgetsStack::updatePercent() {
    qreal totHeight = 0.;
    for(QWidget *wid : mWidgets) {
        totHeight += wid->height();
        if(wid->height() == 0) totHeight += 2*MIN_WIDGET_HEIGHT;
    }

    mHeightPercent.clear();
    for(int i = 0; i < mWidgets.count(); i++) {
        int widHeight = mWidgets.at(i)->height();
        if(widHeight == 0) widHeight = 2*MIN_WIDGET_HEIGHT;
        mHeightPercent << widHeight/(qreal)totHeight;
    }
}

void VerticalWidgetsStack::appendWidget(QWidget *widget) {
    mWidgets.append(widget);
    widget->setParent(this);
    widget->setFixedHeight(3*MIN_WIDGET_HEIGHT);
    updatePercent();
    updateResizers();
    updateSizesAndPositions();
}

void VerticalWidgetsStack::insertWidget(const int &id,
                                        QWidget *widget) {
    mWidgets.insert(id, widget);
    widget->setParent(this);
    widget->setFixedHeight(3*MIN_WIDGET_HEIGHT);
    updatePercent();
    updateResizers();
    updateSizesAndPositions();
    widget->show();
}

void VerticalWidgetsStack::removeWidget(QWidget *widget) {
    if(mWidgets.removeOne(widget) ) {
        delete widget;
        updatePercent();
        updateResizers();
        updateSizesAndPositions();
    }
}

void VerticalWidgetsStack::resizeEvent(QResizeEvent *) {
    updateSizesAndPositions();
}
