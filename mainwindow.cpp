#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Z &&
            (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
        if(QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            mUndoRedoStack.redo();
        } else {
            mUndoRedoStack.undo();
        }
    } else {
        canvas->callKeyPress(event);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    canvas = new Canvas(this);
    setCentralWidget(canvas);
}

MainWindow::~MainWindow()
{

}

UndoRedoStack *MainWindow::getUndoRedoStack()
{
    return &mUndoRedoStack;
}
