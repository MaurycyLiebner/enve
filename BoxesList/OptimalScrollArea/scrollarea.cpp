#include "scrollarea.h"
#include <QResizeEvent>
#include <QScrollBar>

ScrollArea::ScrollArea(QWidget *parent) : QScrollArea(parent)
{
    setFocusPolicy(Qt::NoFocus);
    verticalScrollBar()->setSingleStep(20);

    setHorizontalScrollBarPolicy(
                Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(
                Qt::ScrollBarAlwaysOn);
    setBackgroundRole(
                QPalette::Window);
    setFrameShadow(
                QFrame::Plain);
    setFrameShape(
                QFrame::NoFrame);
    setWidgetResizable(true);
}

void ScrollArea::resizeEvent(QResizeEvent *e) {
    int newHeight = e->size().height();
    if(newHeight == mLastHeight) return;
    mLastHeight = newHeight;
    emit heightChanged(newHeight);
    QScrollArea::resizeEvent(e);
}

void ScrollArea::callWheelEvent(QWheelEvent *event) {
    verticalScrollBar()->triggerAction(
                (event->delta() > 0) ?
                    QAbstractSlider::SliderSingleStepSub :
                    QAbstractSlider::SliderSingleStepAdd);
}
