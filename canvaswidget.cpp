#include "canvaswidget.h"
#include "canvas.h"
#include <QComboBox>
#include "mainwindow.h"
CanvasWidget::CanvasWidget(FillStrokeSettingsWidget *fillStrokeSettingsWidget,
                           QWidget *parent) : QWidget(parent) {
    addCanvasToListAndSetAsCurrent(new Canvas(fillStrokeSettingsWidget, this) );
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

Canvas *CanvasWidget::getCurrentCanvas() {
    return mCurrentCanvas;
}

void CanvasWidget::setCurrentCanvas(const int &id) {
    setCurrentCanvas(mCanvasList.at(id));
}

void CanvasWidget::setCurrentCanvas(Canvas *canvas) {
    mCurrentCanvas = canvas;
    update();
}

void CanvasWidget::addCanvasToList(Canvas *canvas) {
    mCanvasList << canvas;
}

void CanvasWidget::addCanvasToListAndSetAsCurrent(Canvas *canvas) {
    addCanvasToList(canvas);
    setCurrentCanvas(canvas);
}

void CanvasWidget::renameCanvas(Canvas *canvas, const QString &newName) {
    canvas->setName(newName);
}

void CanvasWidget::renameCanvas(const int &id, const QString &newName) {
    renameCanvas(mCanvasList.at(id), newName);
}

void CanvasWidget::renameCurrentCanvas(const QString &newName) {
    if(mCurrentCanvas == NULL) return;
    renameCanvas(mCurrentCanvas, newName);
}

void CanvasWidget::paintEvent(QPaintEvent *) {
    if(mCurrentCanvas == NULL) return;
    QPainter p(this);
    mCurrentCanvas->paintEvent(&p);
    p.end();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mousePressEvent(event);
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseReleaseEvent(event);
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseMoveEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->wheelEvent(event);
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseDoubleClickEvent(event);
}

void CanvasWidget::keyPressEvent(QKeyEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->keyPressEvent(event);
}
