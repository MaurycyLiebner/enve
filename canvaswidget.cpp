#include "canvaswidget.h"
#include "canvas.h"

CanvasWidget::CanvasWidget(FillStrokeSettingsWidget *fillStrokeSettingsWidget,
                           QWidget *parent) : QWidget(parent)
{
    mCanvas = new Canvas(fillStrokeSettingsWidget, this);;
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

Canvas *CanvasWidget::getCanvas()
{
    return mCanvas;
}

void CanvasWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    mCanvas->paintEvent(&p);
    p.end();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    mCanvas->mousePressEvent(event);
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mCanvas->mouseReleaseEvent(event);
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    mCanvas->mouseMoveEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    mCanvas->wheelEvent(event);
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    mCanvas->mouseDoubleClickEvent(event);
}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{
    mCanvas->keyPressEvent(event);
}
