#include "connectedtomainwindow.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include <QApplication>

ConnectedToMainWindow::ConnectedToMainWindow()/* :
    SmartPointerTarget()*/
{
    mMainWindow = MainWindow::getInstance();
}

void ConnectedToMainWindow::startNewUndoRedoSet()
{
    mMainWindow->getUndoRedoStack()->startNewSet();
}

void ConnectedToMainWindow::finishUndoRedoSet()
{
    mMainWindow->getUndoRedoStack()->finishSet();
}

void ConnectedToMainWindow::createDetachedUndoRedoStack() {
    mMainWindow->createDetachedUndoRedoStack();
}

void ConnectedToMainWindow::deleteDetachedUndoRedoStack() {
    mMainWindow->deleteDetachedUndoRedoStack();
}

void ConnectedToMainWindow::addUndoRedo(UndoRedo *undoRedo)
{
    mMainWindow->getUndoRedoStack()->addUndoRedo(undoRedo);
}

void ConnectedToMainWindow::callUpdateSchedulers() {
    mMainWindow->callUpdateSchedulers();
}

MainWindow *ConnectedToMainWindow::getMainWindow() {
    return mMainWindow;
}

bool ConnectedToMainWindow::isShiftPressed() {
    return mMainWindow->isShiftPressed();
}

bool ConnectedToMainWindow::isShiftPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::ShiftModifier;
}

bool ConnectedToMainWindow::isCtrlPressed() {
    return mMainWindow->isCtrlPressed();
}

bool ConnectedToMainWindow::isCtrlPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::ControlModifier;
}

bool ConnectedToMainWindow::isAltPressed() {
    return mMainWindow->isAltPressed();
}

bool ConnectedToMainWindow::isAltPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::AltModifier;
}

int ConnectedToMainWindow::getCurrentFrameFromMainWindow() {
    return mMainWindow->getCurrentFrame();
}

int ConnectedToMainWindow::getFrameCount() {
    return mMainWindow->getFrameCount();
}

void ConnectedToMainWindow::graphUpdateAfterKeysChanged()
{
    //mMainWindow->getKeysView()->graphUpdateAfterKeysChanged();
}

void ConnectedToMainWindow::graphScheduleUpdateAfterKeysChanged() {
    //mMainWindow->getKeysView()->scheduleGraphUpdateAfterKeysChanged();
}

void ConnectedToMainWindow::schedulePivotUpdate() {
    mMainWindow->schedulePivotUpdate();
}
