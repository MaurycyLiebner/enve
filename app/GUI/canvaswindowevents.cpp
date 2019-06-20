#include "canvaswindow.h"
#include "canvas.h"
#include "global.h"

QPointF CanvasWindow::mapToCanvasCoord(const QPointF& windowCoord) {
    return mViewTransform.inverted().map(windowCoord);
}

void CanvasWindow::translateView(const QPointF &trans) {
    if(!mCurrentCanvas) return;
    mViewTransform.translate(trans.x(), trans.y());
}

void CanvasWindow::zoomView(const qreal scaleBy, const QPointF &absOrigin) {
    if(!mCurrentCanvas) return;
    const QPointF transPoint = -mapToCanvasCoord(absOrigin);

    mViewTransform.translate(-transPoint.x(), -transPoint.y());
    mViewTransform.scale(scaleBy, scaleBy);
    mViewTransform.translate(transPoint.x(), transPoint.y());
}
#include <QApplication>
#include <QEvent>
void CanvasWindow::requestFitCanvasToSize() {
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

bool CanvasWindow::event(QEvent *event) {
    if(event->type() == QEvent::User) {
        fitCanvasToSize();
    }
    return QWidget::event(event);
}

#include <QResizeEvent>
void CanvasWindow::resizeEvent(QResizeEvent *e) {
    if(e->size().isValid()) {
        if(mOldSize.isValid()) {
            const auto dSize = e->size() - mOldSize;
            const qreal div = 2*mViewTransform.m11();
            const QPointF trans{dSize.width()/div, dSize.height()/div};
            translateView(trans);
        }
        // e->oldSize() returns {-1, -1} after chaning parent
        mOldSize = e->size();
    }
    GLWindow::resizeEvent(e);
}

void CanvasWindow::fitCanvasToSize() {
    if(!mCurrentCanvas) return;
    mViewTransform.reset();
    const auto canvasSize = mCurrentCanvas->getCanvasSize();
    const qreal widWidth = width();
    const qreal widHeight = height();
    const qreal widthScale = (widWidth - MIN_WIDGET_DIM)/canvasSize.width();
    const qreal heightScale = (widHeight - MIN_WIDGET_DIM)/canvasSize.height();
    const qreal minScale = qMin(widthScale, heightScale);
    translateView({(widWidth - canvasSize.width()*minScale)*0.5,
                   (widHeight - canvasSize.height()*minScale)*0.5});
    mViewTransform.scale(minScale, minScale);
}

void CanvasWindow::resetTransormation() {
    if(!mCurrentCanvas) return;
    mViewTransform.reset();
    translateView({(width() - mCurrentCanvas->getCanvasWidth())*0.5,
                   (height() - mCurrentCanvas->getCanvasHeight())*0.5});
}
