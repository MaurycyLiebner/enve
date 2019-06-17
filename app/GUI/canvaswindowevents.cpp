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

void CanvasWindow::fitCanvasToSize() {
    if(!mCurrentCanvas) return;
    mViewTransform.reset();
    const auto canvasSize = mCurrentCanvas->getCanvasSize();
    const qreal widWidth = width();
    const qreal widHeight = height();
    const qreal widthScale = (widWidth - MIN_WIDGET_DIM)/canvasSize.width();
    const qreal heightScale = (widHeight - MIN_WIDGET_DIM)/canvasSize.height();
    const qreal minScale = qMin(widthScale, heightScale);
    mViewTransform.scale(minScale, minScale);
    translateView({(widWidth - canvasSize.width()*minScale)*0.5,
                   (widHeight - canvasSize.height()*minScale)*0.5});
}

void CanvasWindow::resetTransormation() {
    if(!mCurrentCanvas) return;
    mViewTransform.reset();
    translateView({(width() - mCurrentCanvas->getCanvasWidth())*0.5,
                   (height() - mCurrentCanvas->getCanvasHeight())*0.5});
}
