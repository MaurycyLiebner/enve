#include "mainwindow.h"
#include "canvas.h"

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    canvas->callKeyPress(event);
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

void MainWindow::addUndoRedo(UndoRedo undoRedo)
{
    mUndoRedoStack.addUndoRedo(undoRedo);
}
