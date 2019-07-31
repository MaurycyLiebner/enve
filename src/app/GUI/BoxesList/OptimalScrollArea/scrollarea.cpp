#include "scrollarea.h"
#include <QResizeEvent>
#include <QScrollBar>

ScrollArea::ScrollArea(QWidget * const parent) : QScrollArea(parent) {
    setFocusPolicy(Qt::NoFocus);
    //verticalScrollBar()->setSingleStep(20);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setBackgroundRole(QPalette::Window);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);
}

void ScrollArea::scrollBy(const int x, const int y) {
    Q_UNUSED(x);
    verticalScrollBar()->setValue(verticalScrollBar()->value() + y);
}

void ScrollArea::resizeEvent(QResizeEvent *e) {
    int newHeight = e->size().height();
    if(newHeight != mLastHeight) {
        mLastHeight = newHeight;
        emit heightChanged(newHeight);
    }
    int newWidth = e->size().width();
    if(newWidth != mLastWidth) {
        mLastWidth = newWidth;
        emit widthChanged(newWidth);
    }
    QScrollArea::resizeEvent(e);
}
#include "GUI/global.h"
void ScrollArea::callWheelEvent(QWheelEvent *event) {
    if(event->delta() > 0) {
        scrollBy(0, -MIN_WIDGET_DIM);
    } else {
        scrollBy(0, MIN_WIDGET_DIM);
    }
//    verticalScrollBar()->triggerAction(
//                (event->delta() > 0) ?
//                    QAbstractSlider::SliderSingleStepSub :
//                    QAbstractSlider::SliderSingleStepAdd);
}
