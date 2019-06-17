#ifndef VERTICALWIDGETSSTACK_H
#define VERTICALWIDGETSSTACK_H

#include <QWidget>
#include "global.h"

class StackWidgetResizer : public QWidget {
    Q_OBJECT
public:
    StackWidgetResizer(QWidget * const parent);

    void paintEvent(QPaintEvent *);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    void enterEvent(QEvent *);

    void leaveEvent(QEvent *);

    void setAboveWidget(QWidget *aboveWidget);
    void setBelowWidget(QWidget *belowWidget);

    void setAboveResizer(StackWidgetResizer *aboveResizer);
    void setBelowResizer(StackWidgetResizer *belowResizer);
    void displace(const int totDy);
signals:
    void finishedChanging();
private:
    bool mHover = false;
    bool mPressed = false;
    int mPressY;
    StackWidgetResizer *mAboveResizer = nullptr;
    StackWidgetResizer *mBelowResizer = nullptr;
    QWidget *mAboveWidget = nullptr;
    QWidget *mBelowWidget = nullptr;
};

template <int (QWidget::*DimGetter)() const, void (QWidget::*DimSetter)(int),
          int (QWidget::*PosGetter)() const, void (*PosSetter)(int, QWidget*),
          int (QWidget::*OtherDimGetter)() const, void (QWidget::*OtherDimSetter)(int)>
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
            int iNewDim = qRound(mDimPercent.at(i)*thisDim);
            if(iNewDim < 2*MIN_WIDGET_DIM) iNewDim = 2*MIN_WIDGET_DIM;
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
        int resId = 0;
        StackWidgetResizer *lastRes = nullptr;
        for(int i = 0; i < mWidgets.count() - 1; i++) {
            const auto currWid = mWidgets.at(i);
            const auto nextWid = mWidgets.at(i + 1);

            StackWidgetResizer *res;
            if(resId < mResizers.count()) {
                res = mResizers.at(resId);
            } else {
                res = new StackWidgetResizer(mThis);
                mResizers << res;
                QObject::connect(res, &StackWidgetResizer::finishedChanging,
                                 mThis, [this]() { updatePercent(); });
            }
            res->setAboveWidget(currWid);
            res->setBelowWidget(nextWid);
            resId++;
            PosSetter((nextWid->*PosGetter)(), res);
            res->setAboveResizer(lastRes);
            if(lastRes) lastRes->setBelowResizer(res);
            lastRes = res;
        }
        if(lastRes) lastRes->setBelowResizer(nullptr);
        for(int i = resId; i < mResizers.count(); i++) {
            delete mResizers.takeAt(i);
        }
    }

    void updatePercent() {
        qreal totDim = 0;
        for(const auto wid : mWidgets) {
            const int widDim = (wid->*DimGetter)();
            if(widDim < 2*MIN_WIDGET_DIM) totDim += 2*MIN_WIDGET_DIM;
            else totDim += widDim;
        }

        mDimPercent.clear();
        for(int i = 0; i < mWidgets.count(); i++) {
            int widDim = (mWidgets.at(i)->*DimGetter)();
            if(widDim < 2*MIN_WIDGET_DIM) widDim = 2*MIN_WIDGET_DIM;
            mDimPercent << widDim/qreal(totDim);
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
        (widget->*DimSetter)(3*MIN_WIDGET_DIM);
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
    QList<StackWidgetResizer*> mResizers;
};

void moveY(const int y, QWidget * const widget);

class VWidgetStack : public QWidget,
        public WidgetStackBase<&QWidget::height, &QWidget::setFixedHeight,
                               &QWidget::y, &moveY,
                               &QWidget::width, &QWidget::setFixedWidth> {
public:
    VWidgetStack(QWidget * const parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *) {
        updateSizesAndPositions();
    }
};

void moveX(const int x, QWidget * const widget);

class HWidgetStack : public QWidget,
        public WidgetStackBase<&QWidget::width, &QWidget::setFixedWidth,
                               &QWidget::x, &moveX,
                               &QWidget::height, &QWidget::setFixedHeight> {
public:
    HWidgetStack(QWidget * const parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *) {
        updateSizesAndPositions();
    }
};


#endif // VERTICALWIDGETSSTACK_H
