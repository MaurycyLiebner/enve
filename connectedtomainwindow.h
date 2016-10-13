#ifndef CONNECTEDTOMAINWINDOW_H
#define CONNECTEDTOMAINWINDOW_H
#include <QString>
#include "smartpointertarget.h"

#define getAtIndexOrGiveNull(index, list) (( (index) >= (list).count() || (index) < 0 ) ? NULL : (list).at( (index) ))

#define foreachInverted(item, list) item = getAtIndexOrGiveNull((list).count() - 1, (list)); \
    for(int i = (list).count() - 1; i >= 0; i--, item = getAtIndexOrGiveNull(i, (list)) )

class MainWindow;

class UndoRedo;

class UpdateScheduler;

class QKeyEvent;

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
    void graphScheduleUpdateAfterKeysChanged();
    bool isShiftPressed(QKeyEvent *event);
    bool isCtrlPressed(QKeyEvent *event);
    bool isAltPressed(QKeyEvent *event);
protected:
    MainWindow *mMainWindow;
};

#endif // CONNECTEDTOMAINWINDOW_H
