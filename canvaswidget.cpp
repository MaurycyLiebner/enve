#include "canvaswidget.h"
#include "canvas.h"
#include <QComboBox>
#include "mainwindow.h"
CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

Canvas *CanvasWidget::getCurrentCanvas() {
    return mCurrentCanvas;
}

void CanvasWidget::setCurrentCanvas(const int &id) {
    if(mCanvasList.isEmpty()) {
        setCurrentCanvas((Canvas*)NULL);
    } else {
        setCurrentCanvas(mCanvasList.at(id));
    }
}

void CanvasWidget::setCurrentCanvas(Canvas *canvas) {
    mCurrentCanvas = canvas;
    if(mCurrentCanvas != NULL) {
        setCanvasMode(mCurrentCanvas->getCurrentCanvasMode());
    }
    update();
}

void CanvasWidget::addCanvasToList(Canvas *canvas) {
    canvas->incNumberPointers();
    mCanvasList << canvas;
}

void CanvasWidget::removeCanvas(const int &id) {
    Canvas *canvas = mCanvasList.takeAt(id);
    canvas->decNumberPointers();
    if(mCanvasList.isEmpty()) {
        setCurrentCanvas((Canvas*)NULL);
    } else if(id < mCanvasList.count()) {
        setCurrentCanvas(id);
    } else {
        setCurrentCanvas(id - 1);
    }
}

void CanvasWidget::setCanvasMode(const CanvasMode &mode) {
    if(hasNoCanvas()) {
        setCursor(QCursor(Qt::ArrowCursor) );
        return;
    }

    if(mode == MOVE_PATH) {
        setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mode == MOVE_POINT) {
        setCursor(QCursor(QPixmap(":/curors/cursor-node.xpm"), 0, 0) );
    } else if(mode == PICK_PATH_SETTINGS) {
        setCursor(QCursor(QPixmap(":/curors/cursor_color_picker.png"), 2, 20) );
    } else if(mode == ADD_CIRCLE) {
        setCursor(QCursor(QPixmap(":/curors/cursor-ellipse.xpm"), 4, 4) );
    } else if(mode == ADD_RECTANGLE) {
        setCursor(QCursor(QPixmap(":/curors/cursor-rect.xpm"), 4, 4) );
    } else if(mode == ADD_TEXT) {
        setCursor(QCursor(QPixmap(":/curors/cursor-text.xpm"), 4, 4) );
    } else {
        setCursor(QCursor(QPixmap(":/curors/cursor-pen.xpm"), 4, 4) );
    }

    mCurrentCanvas->setCanvasMode(mode);
    MainWindow::getInstance()->updateCanvasModeButtonsChecked();
    callUpdateSchedulers();
}

void CanvasWidget::callUpdateSchedulers() {
    MainWindow::getInstance()->callUpdateSchedulers();
}

void CanvasWidget::setMovePathMode() {
    setCanvasMode(MOVE_PATH);
}

void CanvasWidget::setMovePointMode() {
    setCanvasMode(MOVE_POINT);
}

void CanvasWidget::setAddPointMode() {
    setCanvasMode(ADD_POINT);
}

void CanvasWidget::setRectangleMode() {
    setCanvasMode(ADD_RECTANGLE);
}

void CanvasWidget::setCircleMode() {
    setCanvasMode(ADD_CIRCLE);
}

void CanvasWidget::setTextMode() {
    setCanvasMode(ADD_TEXT);
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

bool CanvasWidget::hasNoCanvas() {
    return mCurrentCanvas == NULL;
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

bool CanvasWidget::processUnfilteredKeyEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
        setCanvasMode(CanvasMode::ADD_POINT);
    } else if(event->key() == Qt::Key_F4) {
        setCanvasMode(CanvasMode::ADD_CIRCLE);
    } else if(event->key() == Qt::Key_F5) {
        setCanvasMode(CanvasMode::ADD_RECTANGLE);
    } else if(event->key() == Qt::Key_F6) {
        setCanvasMode(CanvasMode::ADD_TEXT);
    } else {
        return false;
    }
    return true;
}

bool CanvasWidget::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event)) return true;
    if(hasNoCanvas()) return false;
    return mCurrentCanvas->processFilteredKeyEvent(event);
}
