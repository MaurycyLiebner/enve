#ifndef WIDGETSTACK_H
#define WIDGETSTACK_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QBoxLayout>
#include <QMainWindow>

#include "global.h"

#define STACK_TMPL_DEFS \
    int (QWidget::*DimGetter)() const, void (*DimSetter)(QWidget*, int), \
    int (QWidget::*PosGetter)() const, void (*PosSetter)(int, QWidget*), \
    int (QWidget::*OtherDimGetter)() const, void (*OtherDimSetter)(QWidget*, int), \
    int (QMouseEvent::*MousePosGetter)() const, class TResizer

#define V_STACK_TMPL \
    &QWidget::height, &gResizeH, \
    &QWidget::y, &gMoveY, \
    &QWidget::width, &gResizeW, \
    &QMouseEvent::y, VStackResizer

#define H_STACK_TMPL \
    &QWidget::width, &gResizeW, \
    &QWidget::x, &gMoveX, \
    &QWidget::height, &gResizeH, \
    &QMouseEvent::x, HStackResizer

void gMoveX(const int x, QWidget * const widget);
void gMoveY(const int y, QWidget * const widget);

void gResizeW(QWidget * const widget, const int w);
void gResizeH(QWidget * const widget, const int h);

bool gReplaceWidget(QWidget * const from, QWidget * const to,
                    bool * const centralWid = nullptr);

template <STACK_TMPL_DEFS>
class StackResizerBase {
protected:
    StackResizerBase() {}

    void setThis(QWidget * const thisP) {
        mThis = thisP;
        DimSetter(mThis, 10);
        OtherDimSetter(mThis, 2*MIN_WIDGET_DIM*100);
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
        DimSetter(mPrevWidget, (mPrevWidget->*DimGetter)() + dDim);
        DimSetter(mNextWidget, (mNextWidget->*DimGetter)() - dDim);
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
        setCursor(Qt::SplitVCursor);
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
        setCursor(Qt::SplitHCursor);
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
public:
    virtual ~WidgetStackBase() {}
protected:
    WidgetStackBase() {}

    void setThis(QWidget * const thisP) {
        Q_ASSERT(!mThis && thisP);
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
            DimSetter(widget, iNewDim);
            OtherDimSetter(widget, thisOtherDim);
            accumulated += iNewDim;
            if(i < mResizers.count()) {
                PosSetter(accumulated - 5, mResizers.at(i));
            }
        }
        const auto lastWidget = mWidgets.last();
        PosSetter(accumulated, lastWidget);
        const int lastDim = qMax(2*MIN_WIDGET_DIM, thisDim - accumulated);
        DimSetter(lastWidget, lastDim);
        OtherDimSetter(lastWidget, thisOtherDim);
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
        mWidgets.insert(id >= 0 ? id : mWidgets.count(), widget);
        widget->setParent(mThis);
        if(id > 0 && mWidgets.count() > 1) {
            const auto prevWid = mWidgets.at(id - 1);
            const int dim = (prevWid->*DimGetter)();
            const int halfDim = dim/2;
            DimSetter(prevWid, dim % 2 == 0 ? halfDim : halfDim + 1);
            DimSetter(widget, halfDim);
        } else DimSetter(widget, 3*MIN_WIDGET_DIM);
        updateAll();
        widget->show();

        QObject::connect(widget, &QObject::destroyed, mThis,
                         [this, widget]() { removeWidgetBeforeDestroyed(widget); });
    }

    QWidget* replaceWidget(QWidget * const oldWid, QWidget * const newWid) {
        const int id = mWidgets.indexOf(oldWid);
        if(id == -1) return nullptr;
        mWidgets.replace(id, newWid);
        newWid->setParent(mThis);
        DimSetter(newWid, (oldWid->*DimGetter)());
        updateAll();
        newWid->show();
        QObject::connect(newWid, &QObject::destroyed, mThis,
                         [this, newWid]() { removeWidgetBeforeDestroyed(newWid); });
        QObject::disconnect(oldWid, nullptr, mThis, nullptr);
        return oldWid;
    }

    QWidget* takeWidget(const int id) {
        return takeWidget(mWidgets.at(id));
    }

    QWidget* takeWidget(QWidget * const widget) {
        if(mWidgets.removeOne(widget)) {
            QObject::disconnect(widget, nullptr, mThis, nullptr);
            updateAll();
            widget->setParent(nullptr);
            return widget;
        }
        return nullptr;
    }

    QWidget* asWidget() { return mThis; }

    int count() const { return mWidgets.count(); }
    bool isEmpty() const { return mWidgets.isEmpty(); }
private:
    void updateAll() {
        updatePercent();
        updateResizers();
        updateSizesAndPositions();
    }

    bool removeWidgetBeforeDestroyed(QObject * const widObj) {
        mWidgets.removeOne(static_cast<QWidget*>(widObj));
        updateAll();
        return false;
    }

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

#endif // WIDGETSTACK_H
