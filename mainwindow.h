#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "canvas.h"
#include "undoredo.h"
#include "Colors/color.h"
#include <QDockWidget>
#include "fillstrokesettings.h"

class BoxesList;

class UpdateScheduler;

class ColorSettingsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QDockWidget *mRightDock;
    QDockWidget *mBottomDock;
    BoxesList *mBoxListWidget;

    QToolBar *mToolBar;
    QAction *mActionConnectPoints;
    QAction *mActionDisconnectPoints;
    QAction *mActionMergePoints;
//
    QAction *mActionSymmetricPointCtrls;
    QAction *mActionSmoothPointCtrls;
    QAction *mActionCornerPointCtrls;
//

    QString mCurrentFilePath = "";

    QMenuBar *mMenuBar;
    QMenu *mFileMenu;

    Canvas *mCanvas;
    UndoRedoStack mUndoRedoStack;
    QList<UpdateScheduler*> mUpdateSchedulers;
    bool processKeyEvent(QKeyEvent *event);
    FillStrokeSettingsWidget *mFillStrokeSettings;

    bool mChangedSinceSaving = false;
    bool mEventFilterDisabled = false;
    QWidget *grayOutWidget = NULL;
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *, QEvent *e);
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(UpdateScheduler *scheduler);

    void callUpdateSchedulers();
    void scheduleRepaint();
    void schedulePivotUpdate();

    Canvas *getCanvas();
    FillStrokeSettingsWidget *getFillStrokeSettings();
    void saveToFile(QString path);
    void loadFile(QString path);
    void clearAll();
    void importFile(QString path, bool loadInBox);
    void exportSelected(QString path);
    void setCurrentPath(QString newPath);
    void createTablesInSaveDatabase();
    void updateTitle();
    void setFileChangedSinceSaving(bool changed);
    void disableEventFilter();
    void enableEventFilter();

    void scheduleBoxesListRepaint();
    void disable();
    void enable();
private slots:
    void newFile();
    bool askForSaving();
    void openFile();
    void saveFile();
    void saveFileAs();
    void saveBackup();
    void closeProject();
    void exitProgram();
    void importFile();
    void exportSelected();
    void revert();
};

#endif // MAINWINDOW_H
