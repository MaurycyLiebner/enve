#include "connectedtomainwindow.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include <QApplication>

QString ConnectedToMainWindow::boolToSql(bool bT) {
    return (bT) ? "1" : "0";
}

ConnectedToMainWindow::ConnectedToMainWindow(ConnectedToMainWindow *parent) :
    SmartPointerTarget()
{
    mMainWindow = parent->getMainWindow();
}

ConnectedToMainWindow::ConnectedToMainWindow(MainWindow *parent) :
    SmartPointerTarget()
{
    mMainWindow = parent;
}

void ConnectedToMainWindow::addUndoRedo(UndoRedo *undoRedo)
{
    mMainWindow->getUndoRedoStack()->addUndoRedo(undoRedo);
}

void ConnectedToMainWindow::addUpdateScheduler(UpdateScheduler *scheduler)
{
    mMainWindow->addUpdateScheduler(scheduler);
}

void ConnectedToMainWindow::callUpdateSchedulers()
{
    mMainWindow->callUpdateSchedulers();
}

MainWindow *ConnectedToMainWindow::getMainWindow()
{
    return mMainWindow;
}

bool ConnectedToMainWindow::isShiftPressed() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

bool ConnectedToMainWindow::isCtrlPressed() {
    return (QApplication::keyboardModifiers() & Qt::ControlModifier);
}

bool ConnectedToMainWindow::isAltPressed() {
    return (QApplication::keyboardModifiers() & Qt::AltModifier);
}

int ConnectedToMainWindow::getCurrentFrame()
{
    return mMainWindow->getCurrentFrame();
}

int ConnectedToMainWindow::getMaxFrame()
{
    return mMainWindow->getMaxFrame();
}

int ConnectedToMainWindow::getMinFrame()
{
    return mMainWindow->getMinFrame();
}

bool ConnectedToMainWindow::isRecording()
{
    return mMainWindow->isRecording();
}

bool ConnectedToMainWindow::isRecordingAllPoints()
{
    return mMainWindow->isRecordingAllPoints();
}

void ConnectedToMainWindow::startNewUndoRedoSet()
{
     mMainWindow->getUndoRedoStack()->startNewSet();
}

void ConnectedToMainWindow::finishUndoRedoSet()
{
     mMainWindow->getUndoRedoStack()->finishSet();
}

void ConnectedToMainWindow::scheduleRepaint() {
    mMainWindow->scheduleRepaint();
}

void ConnectedToMainWindow::scheduleBoxesListRepaint() {
    mMainWindow->scheduleBoxesListRepaint();
}

void ConnectedToMainWindow::schedulePivotUpdate() {
    mMainWindow->schedulePivotUpdate();
}
