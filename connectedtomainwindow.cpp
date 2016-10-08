#include "connectedtomainwindow.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include <QApplication>
#include "boxeslist.h"

QString ConnectedToMainWindow::boolToSql(bool bT) {
    return (bT) ? "1" : "0";
}

ConnectedToMainWindow::ConnectedToMainWindow() :
    SmartPointerTarget()
{
    mMainWindow = MainWindow::getInstance();
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
    return mMainWindow->isShiftPressed();
}

bool ConnectedToMainWindow::isCtrlPressed() {
    return mMainWindow->isCtrlPressed();
}

bool ConnectedToMainWindow::isAltPressed() {
    return mMainWindow->isAltPressed();
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

void ConnectedToMainWindow::graphUpdateAfterKeysChanged()
{
    mMainWindow->getKeysView()->graphUpdateAfterKeysChanged();
}

void ConnectedToMainWindow::graphScheduleUpdateAfterKeysChanged() {
    mMainWindow->getKeysView()->scheduleGraphUpdateAfterKeysChanged();
}

void ConnectedToMainWindow::startNewUndoRedoSet()
{
     mMainWindow->getUndoRedoStack()->startNewSet();
}

void ConnectedToMainWindow::finishUndoRedoSet()
{
     mMainWindow->getUndoRedoStack()->finishSet();
}

void ConnectedToMainWindow::schedulePivotUpdate() {
    mMainWindow->schedulePivotUpdate();
}
