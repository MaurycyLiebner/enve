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
    ConnectedToMainWindow(ConnectedToMainWindow *parent);
    ConnectedToMainWindow(MainWindow *parent);
    ~ConnectedToMainWindow() {}

    void addUndoRedo(UndoRedo *undoRedo);
    void addUpdateScheduler(UpdateScheduler *scheduler);
    void callUpdateSchedulers();
    MainWindow *getMainWindow();
    QString boolToSql(bool bT);

    void startNewUndoRedoSet();
    void finishUndoRedoSet();
    virtual void scheduleRepaint();
    virtual void schedulePivotUpdate();
    void scheduleBoxesListRepaint();
    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    int getCurrentFrame();
    int getMaxFrame();
    int getMinFrame();
    bool isRecording();
    bool isRecordingAllPoints();
protected:
    MainWindow *mMainWindow;
};

#endif // CONNECTEDTOMAINWINDOW_H
