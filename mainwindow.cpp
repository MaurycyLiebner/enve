#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QToolBar>
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
    mToolBar = new QToolBar(this);
    mActionConnectPoints = new QAction("CONNECT POINTS", this);
    mToolBar->addAction(mActionConnectPoints);
    connect(mActionConnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(connectPoints()));
    mActionDisconnectPoints = new QAction("DISCONNECT POINTS", this);
    mToolBar->addAction(mActionDisconnectPoints);
    connect(mActionDisconnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(disconnectPoints()));
    mActionMergePoints = new QAction("MERGE POINTS", this);
    mToolBar->addAction(mActionMergePoints);
    connect(mActionMergePoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(mergePoints()));

    addToolBar(mToolBar);

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
