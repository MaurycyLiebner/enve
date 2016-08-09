#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include "updatescheduler.h"

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
        mCanvas->callKeyPress(event);
    }

    callUpdateSchedulers();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mCanvas = new Canvas(this);
    setCentralWidget(mCanvas);
}

MainWindow::~MainWindow()
{

}

UndoRedoStack *MainWindow::getUndoRedoStack()
{
    return &mUndoRedoStack;
}

void MainWindow::addUpdateScheduler(UpdateScheduler *scheduler)
{
    mUpdateSchedulers << scheduler;
}

void MainWindow::callUpdateSchedulers()
{
    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        sheduler->update();
        delete sheduler;
    }
    mUpdateSchedulers.clear();
    mCanvas->repaintIfNeeded();
}
