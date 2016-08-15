#ifndef CONNECTEDTOMAINWINDOW_H
#define CONNECTEDTOMAINWINDOW_H

class MainWindow;

class UndoRedo;

class UpdateScheduler;

class ConnectedToMainWindow
{
public:
    ConnectedToMainWindow(ConnectedToMainWindow *parent);
    ConnectedToMainWindow(MainWindow *parent);
    void addUndoRedo(UndoRedo *undoRedo);
    void addUpdateScheduler(UpdateScheduler *scheduler);
    void callUpdateSchedulers();
    MainWindow *getMainWindow();

    void startNewUndoRedoSet();
    void finishUndoRedoSet();
    virtual void scheduleRepaint();
    virtual void schedulePivotUpdate();
protected:
    MainWindow *mMainWindow;
};

#endif // CONNECTEDTOMAINWINDOW_H
