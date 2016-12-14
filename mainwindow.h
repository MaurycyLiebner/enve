﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "canvas.h"
#include "undoredo.h"
#include "Colors/color.h"
#include <QDockWidget>
#include "fillstrokesettings.h"
#include "animationdockwidget.h"
#include "keysview.h"
#include "fontswidget.h"
#include "vectorshapesmenu.h"
#include "BoxesList/boxeslistwidget.h"

class PaintControler;

class BoxesList;

class UpdateScheduler;

class ColorSettingsWidget;

class BoxesListAnimationDockWidget;

class CanvasWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    static MainWindow *mMainWindowInstance;

    bool mCancelLastBoxUpdate = false;
    BoundingBox *mLastUpdatedBox = NULL;
    QList<BoundingBox*> mBoxesAwaitingUpdate;
    bool mNoBoxesAwaitUpdate = true;

    QThread *mPaintControlerThread;
    PaintControler *mPaintControler;

    QDockWidget *mRightDock;
    QDockWidget *mBottomDock;
    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget;
    BoxesListWidget *mBoxListWidget;
    KeysView *mKeysView;

    QToolBar *mToolBar;

    QAction *mMovePathMode;
    QAction *mMovePointMode;
    QAction *mAddPointMode;
    QAction *mCircleMode;
    QAction *mRectangleMode;
    QAction *mTextMode;
//
    QAction *mActionConnectPoints;
    QAction *mActionDisconnectPoints;
    QAction *mActionMergePoints;
//
    QAction *mActionSymmetricPointCtrls;
    QAction *mActionSmoothPointCtrls;
    QAction *mActionCornerPointCtrls;
//
    QAction *mActionHighQualityView;
    QAction *mActionEffectsPaintEnabled;

    FontsWidget *mFontWidget = NULL;

    QString mCurrentFilePath = "";

    QMenuBar *mMenuBar;
    QMenu *mFileMenu;
    QMenu *mSelectSameMenu;
    QMenu *mEditMenu;
    QMenu *mObjectMenu;
    QMenu *mPathMenu;
    QMenu *mEffectsMenu;
    QMenu *mViewMenu;
    QMenu *mRenderMenu;

    Canvas *mCanvas;
    CanvasWidget *mCanvasWidget;
    UndoRedoStack mUndoRedoStack;
    QList<UpdateScheduler*> mUpdateSchedulers;
    bool processKeyEvent(QKeyEvent *event);
    FillStrokeSettingsWidget *mFillStrokeSettings;

    bool mChangedSinceSaving = false;
    bool mEventFilterDisabled = false;
    QWidget *grayOutWidget = NULL;

    qreal mFPS = 24.;
    int mMinFrame = 0;
    int mMaxFrame = 200;
    int mSavedCurrentFrame = 0;
    int mCurrentFrame = 0;
    bool mRecording = false;
    bool mAllPointsRecording = false;

    bool mDisplayedFillStrokeSettingsUpdateNeeded = false;

    bool mShiftPressed = false;
    bool mAltPressed = false;
    bool mCtrlPressed = false;

    VectorShapesMenu *mVectorShapesMenu = NULL;

    QString mOutputString;
    int mCurrentRenderFrame;

    bool mPreviewInterrupted = false;
protected:    
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *, QEvent *e);
    void closeEvent(QCloseEvent *e);
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void (MainWindow::*mBoxesUpdateFinishedFunction)(void) = NULL;

    static MainWindow *getInstance();

    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(UpdateScheduler *scheduler);

    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    void callUpdateSchedulers();
    void schedulePivotUpdate();

    AnimationDockWidget *getAnimationDockWidget();
    KeysView *getKeysView();
    BoxesListWidget *getBoxesList();
    Canvas *getCanvas();
    CanvasWidget *getCanvasWidget() { return mCanvasWidget; }
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

    int getCurrentFrame();

    bool isRecording();

    bool isRecordingAllPoints();

    int getMinFrame();
    int getMaxFrame();

    void previewFinished();
    void updateDisplayedFillStrokeSettings();
    void scheduleDisplayedFillStrokeSettingsUpdate();
    void updateDisplayedFillStrokeSettingsIfNeeded();
    void updateCanvasModeButtonsChecked(CanvasMode currentMode);

    void addBoxAwaitingUpdate(BoundingBox *box);
    void setCurrentShapesMenuBox(BoundingBox *box);
    void nextSaveOutputFrame();
    void nextPlayPreviewFrame();

    void setResolutionPercent(qreal percent);
public slots:
    void setCurrentFrame(int frame);
    void setGraphEnabled(bool graphEnabled);
    void setAllPointsRecord(bool allPointsRecord);
    void playPreview();
    void stopPreview();
    void setResolutionPercentId(int id);
private slots:
    void saveOutput(QString renderDest);
    void renderOutput();
    void sendNextBoxForUpdate();

    void newFile();
    bool askForSaving();
    void openFile();
    void saveFile();
    void saveFileAs();
    void saveBackup();
    void closeProject();
    void importFile();
    void exportSelected();
    void revert();

    void setHighQualityView(bool bT);
    void setEffectsPaintEnabled(bool bT);
signals:
    void updateBox(BoundingBox*);
};

#endif // MAINWINDOW_H
