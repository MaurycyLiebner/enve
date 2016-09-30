#ifndef CONNECTEDTOMAINWINDOW_H
#define CONNECTEDTOMAINWINDOW_H
#include <QString>
#include "smartpointertarget.h"

class MainWindow;

class UndoRedo;

class UpdateScheduler;

class ConnectedToMainWindow : public SmartPointerTarget
{
public:
    ConnectedToMainWindow();
    ~ConnectedToMainWindow() {}

    void addUndoRedo(UndoRedo *undoRedo);
    void addUpdateScheduler(UpdateScheduler *scheduler);
    void callUpdateSchedulers();
    MainWindow *getMainWindow();
    QString boolToSql(bool bT);

    void startNewUndoRedoSet();
    void finishUndoRedoSet();
    virtual void schedulePivotUpdate();
    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    int getCurrentFrame();
    int getMaxFrame();
    int getMinFrame();
    bool isRecording();
    bool isRecordingAllPoints();
    void graphUpdateAfterKeysChanged();
protected:
    MainWindow *mMainWindow;
};

#endif // CONNECTEDTOMAINWINDOW_H
