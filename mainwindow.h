#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "canvas.h"
#include "undoredo.h"

class UpdateScheduler;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QToolBar *mToolBar;
    QAction *mActionConnectPoints;
    QAction *mActionDisconnectPoints;
    QAction *mActionMergePoints;

    Canvas *mCanvas;
    UndoRedoStack mUndoRedoStack;
    QList<UpdateScheduler*> mUpdateSchedulers;
protected:
    void keyPressEvent(QKeyEvent *event);
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(UpdateScheduler *scheduler);

    void callUpdateSchedulers();
};

#endif // MAINWINDOW_H
