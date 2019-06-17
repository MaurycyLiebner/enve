#ifndef VERTICALWIDGETSSTACK_H
#define VERTICALWIDGETSSTACK_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

#include "global.h"

#define STACK_TMPL_DEFS \
    int (QWidget::*DimGetter)() const, void (QWidget::*DimSetter)(int), \
    int (QWidget::*PosGetter)() const, void (*PosSetter)(int, QWidget*), \
    int (QWidget::*OtherDimGetter)() const, void (QWidget::*OtherDimSetter)(int), \
    int (QMouseEvent::*MousePosGetter)() const, class TResizer

#define V_STACK_TMPL \
    &QWidget::height, &QWidget::setFixedHeight, \
    &QWidget::y, &moveY, \
    &QWidget::width, &QWidget::setFixedWidth, \
    &QMouseEvent::y, VStackResizer

#define H_STACK_TMPL \
    &QWidget::width, &QWidget::setFixedWidth, \
    &QWidget::x, &moveX, \
    &QWidget::height, &QWidget::setFixedHeight, \
    &QMouseEvent::x, HStackResizer

void moveY(const int y, QWidget * const widget);
void moveX(const int x, QWidget * const widget);

template <STACK_TMPL_DEFS>
class StackResizerBase {
protected:
    StackResizerBase() {}

    void setThis(QWidget * const thisP) {
        mThis = thisP;
        (mThis->*DimSetter)(10);
        (mThis->*OtherDimSetter)(2*MIN_WIDGET_DIM*100);
        mThis->setCursor(Qt::SplitVCursor);
        mThis->setWindowFlags(Qt::WindowStaysOnTopHint);
        mThis->show();
    }

    void displace(const int totDDim) {
        int dDim = totDDim;
        const int newAboveDim = (mPrevWidget->*DimGetter)() + dDim;
        const int newBelowDim = (mNextWidget->*DimGetter)() - dDim;
        if(newAboveDim < 2*MIN_WIDGET_DIM) {
            dDim = 2*MIN_WIDGET_DIM - (mPrevWidget->*DimGetter)();
        } else if(newBelowDim < 2*MIN_WIDGET_DIM) {
            dDim = (mNextWidget->*DimGetter)() - 2*MIN_WIDGET_DIM;
        }
        if(totDDim != dDim) {
            if(totDDim > 0) {
                if(mNextResizer)
                    mNextResizer->displace(totDDim - dDim);
            } else {
                if(mPrevResizer)
                    mPrevResizer->displace(totDDim - dDim);
            }
        }
        (mPrevWidget->*DimSetter)((mPrevWidget->*DimGetter)() + dDim);
        (mNextWidget->*DimSetter)((mNextWidget->*DimGetter)() - dDim);
        PosSetter((mNextWidget->*PosGetter)() + dDim, mNextWidget);
        PosSetter((mThis->*PosGetter)() + dDim, mThis);
    }

    void mouseMoveEventB(QMouseEvent * const event) {
        displace((event->*MousePosGetter)() - mPressDim);
    }

    void mousePressEventB(QMouseEvent * const event) {
        mPressed = true;
        mPressDim = (event->*MousePosGetter)();
        mThis->update();
    }

    void mouseReleaseEventB(QMouseEvent * const) {
        mPressed = false;
        mThis->update();
    }

    void enterEventB() {
        mHover = true;
        mThis->update();
    }

    void leaveEventB() {
        mHover = false;
        mThis->update();
    }
public:
    void setAboveWidget(QWidget * const aboveWidget) {
        mPrevWidget = aboveWidget;
    }

    void setBelowWidget(QWidget * const belowWidget) {
        mNextWidget = belowWidget;
    }

    void setAboveResizer(StackResizerBase * const aboveResizer) {
        mPrevResizer = aboveResizer;
    }

    void setBelowResizer(StackResizerBase * const belowResizer) {
        mNextResizer = belowResizer;
    }

    bool hover() const { return mHover; }
    bool pressed() const { return mPressed; }
private:
    QWidget *mThis = nullptr;

    bool mHover = false;
    bool mPressed = false;
    int mPressDim;
    StackResizerBase *mPrevResizer = nullptr;
    StackResizerBase *mNextResizer = nullptr;
    QWidget *mPrevWidget = nullptr;
    QWidget *mNextWidget = nullptr;
};

class VStackResizer : public QWidget, public StackResizerBase<V_STACK_TMPL> {
    Q_OBJECT
public:
    VStackResizer(QWidget * const parent) : QWidget(parent) {
        setThis(this);
    }
protected:
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        if(pressed()) {
            p.fillRect(rect().adjusted(0, 3, 0, -4), Qt::black);
        } else if(hover()) {
            p.fillRect(rect().adjusted(0, 4, 0, -4), Qt::black);
        } else {
            p.fillRect(rect().adjusted(0, 5, 0, -4), Qt::black);
        }
        p.end();
    }

    void mouseMoveEvent(QMouseEvent * const event) {
        mouseMoveEventB(event);
    }

    void mousePressEvent(QMouseEvent * const event) {
        mousePressEventB(event);
    }

    void mouseReleaseEvent(QMouseEvent * const event) {
        mouseReleaseEventB(event);
        emit finishedChanging();
    }

    void enterEvent(QEvent *) {
        enterEventB();
    }

    void leaveEvent(QEvent *) {
        leaveEventB();
    }
signals:
    void finishedChanging();
};

class HStackResizer : public QWidget, public StackResizerBase<H_STACK_TMPL> {
    Q_OBJECT
public:
    HStackResizer(QWidget * const parent) : QWidget(parent) {
        setThis(this);
    }
protected:
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        if(pressed()) {
            p.fillRect(rect().adjusted(3, 0, -4, 0), Qt::black);
        } else if(hover()) {
            p.fillRect(rect().adjusted(4, 0, -4, 0), Qt::black);
        } else {
            p.fillRect(rect().adjusted(5, 0, -4, 0), Qt::black);
        }
        p.end();
    }

    void mouseMoveEvent(QMouseEvent * const event) {
        mouseMoveEventB(event);
    }

    void mousePressEvent(QMouseEvent * const event) {
        mousePressEventB(event);
    }

    void mouseReleaseEvent(QMouseEvent * const event) {
        mouseReleaseEventB(event);
        emit finishedChanging();
    }

    void enterEvent(QEvent *) {
        enterEventB();
    }

    void leaveEvent(QEvent *) {
        leaveEventB();
    }
signals:
    void finishedChanging();
};

template <STACK_TMPL_DEFS>
class WidgetStackBase {
protected:
    WidgetStackBase() {}

    void setThis(QWidget * const thisP) {
        mThis = thisP;
    }

    void updateSizesAndPositions() {
        if(mWidgets.isEmpty()) return;

        int accumulated = 0;
        const int wCount = mWidgets.count() - 1;
        const int thisDim = (mThis->*DimGetter)();
        const int thisOtherDim = (mThis->*OtherDimGetter)();;
        for(int i = 0; i < wCount; i++) {
            const auto widget = mWidgets.at(i);
            PosSetter(accumulated, widget);
            const int iNewDim = qMax(qRound(mDimPercent.at(i)*thisDim),
                                     2*MIN_WIDGET_DIM);
            (widget->*DimSetter)(iNewDim);
            (widget->*OtherDimSetter)(thisOtherDim);
            if(i < mResizers.count()) {
                PosSetter(accumulated + iNewDim - 5, mResizers.at(i));
            }
            accumulated += iNewDim;
        }
        const auto lastWidget = mWidgets.last();
        PosSetter(accumulated, lastWidget);
        const int lastDim = thisDim - accumulated;
        (lastWidget->*DimSetter)(lastDim);
        (lastWidget->*OtherDimSetter)(thisOtherDim);
    }

    void updateResizers() {
        TResizer *lastRes = nullptr;
        for(int i = 0; i < mWidgets.count() - 1; i++) {
            const auto currWid = mWidgets.at(i);
            const auto nextWid = mWidgets.at(i + 1);

            TResizer *res;
            if(i < mResizers.count()) {
                res = mResizers.at(i);
            } else {
                res = new TResizer(mThis);
                mResizers << res;
                QObject::connect(res, &TResizer::finishedChanging,
                                 mThis, [this]() { updatePercent(); });
            }
            res->raise();
            res->setAboveWidget(currWid);
            res->setBelowWidget(nextWid);
            PosSetter((nextWid->*PosGetter)(), res);
            res->setAboveResizer(lastRes);
            if(lastRes) lastRes->setBelowResizer(res);
            lastRes = res;
        }
        if(lastRes) lastRes->setBelowResizer(nullptr);
        for(int i = qMax(0, mWidgets.count() - 1); i < mResizers.count(); i++) {
            delete mResizers.takeAt(i);
        }
    }

    void updatePercent() {
        int totDim = 0;
        for(const auto wid : mWidgets) {
            const int widDim = (wid->*DimGetter)();
            totDim += qMax(2*MIN_WIDGET_DIM, widDim);
        }

        mDimPercent.clear();
        for(int i = 0; i < mWidgets.count(); i++) {
            const int widDim = qMax(2*MIN_WIDGET_DIM,
                                    (mWidgets.at(i)->*DimGetter)());
            mDimPercent << qreal(widDim)/totDim;
        }
    }

public:
    int childId(QWidget * const child) const {
        return mWidgets.indexOf(child);
    }

    void appendWidget(QWidget * const widget) {
        insertWidget(mWidgets.count(), widget);
    }

    void insertWidget(const int id, QWidget * const widget) {
        mWidgets.insert(id, widget);
        widget->setParent(mThis);
        if(id > 0 && mWidgets.count() > 1) {
            const auto prevWid = mWidgets.at(id - 1);
            const int dim = (prevWid->*DimGetter)();
            const int halfDim = dim/2;
            (prevWid->*DimSetter)(dim % 2 == 0 ? halfDim : halfDim + 1);
            (widget->*DimSetter)(halfDim);
        } else (widget->*DimSetter)(3*MIN_WIDGET_DIM);
        updatePercent();
        updateResizers();
        updateSizesAndPositions();
        widget->show();

        QObject::connect(widget, &QObject::destroyed, mThis,
                         [this, widget]() { takeWidget(widget); });
    }

    QWidget *takeWidget(QWidget * const widget) {
        if(mWidgets.removeOne(widget)) {
            QObject::disconnect(widget, &QObject::destroyed, mThis, nullptr);
            updatePercent();
            updateResizers();
            updateSizesAndPositions();
            return widget;
        }
        return nullptr;
    }
private:
    QWidget * mThis = nullptr;
    QList<QWidget*> mWidgets;
    QList<qreal> mDimPercent;
    QList<TResizer*> mResizers;
};

class VWidgetStack : public QWidget, public WidgetStackBase<V_STACK_TMPL> {
public:
    VWidgetStack(QWidget * const parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *) {
        updateSizesAndPositions();
    }
};

class HWidgetStack : public QWidget, public WidgetStackBase<H_STACK_TMPL> {
public:
    HWidgetStack(QWidget * const parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *) {
        updateSizesAndPositions();
    }
};


#endif // VERTICALWIDGETSSTACK_H
