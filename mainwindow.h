#ifndef MAINWINDOW_H
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
#include "actionbutton.h"

enum ClipboardContainerType : short;

class ClipboardContainer;

class BoxesList;

class UpdateScheduler;

class ColorSettingsWidget;

class BoxesListAnimationDockWidget;

class CanvasWidget;
class MemoryHandler;

class ObjectSettingsWidget;
class BoxScrollWidget;
class ScrollWidget;
class ScrollArea;
//class SoundComposition;

const QString MENU_STYLESHEET =
        "QMenu {\
            background-color: rgb(255, 255, 255);\
            border: 1px solid black;\
        }\
        \
        QMenu::item {\
            spacing: 3px;\
            padding: 2px 25px 2px 25px;\
            background: transparent;\
            color: black;\
        }\
        \
        QMenu::item:selected {\
            background-color: rgb(200, 200, 200);\
            color: black;\
        };";

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

//    void (MainWindow::*mBoxesUpdateFinishedFunction)(void) = NULL;

    static MainWindow *getInstance();

    void createDetachedUndoRedoStack();
    void deleteDetachedUndoRedoStack();

    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(UpdateScheduler *scheduler);

    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    void schedulePivotUpdate();

    AnimationDockWidget *getAnimationDockWidget();
    BoxScrollWidget *getObjectSettingsList();

    CanvasWidget *getCanvasWidget() { return mCanvasWidget; }
    FillStrokeSettingsWidget *getFillStrokeSettings();
    void saveToFile(QString path);
    void loadAVFile(QString path);
    void clearAll();
    void importFile(QString path);
    void setCurrentPath(QString newPath);
    void createTablesInSaveDatabase(QSqlQuery *query);
    void updateTitle();
    void setFileChangedSinceSaving(bool changed);
    void disableEventFilter();
    void enableEventFilter();

    void scheduleBoxesListRepaint();
    void disable();
    void enable();

    int getCurrentFrame();

    bool isRecordingAllPoints();

    int getFrameCount();

    void previewFinished();
    void updateDisplayedFillStrokeSettings();
    void scheduleDisplayedFillStrokeSettingsUpdate();
    void updateDisplayedFillStrokeSettingsIfNeeded();
    void updateCanvasModeButtonsChecked();

    //void addBoxAwaitingUpdate(BoundingBox *box);
    void setCurrentBox(BoundingBox *box);

//    void nextSaveOutputFrame();
//    void nextPlayPreviewFrame();

    void setResolutionPercent(qreal percent);

    void setCurrentFrameForAllWidgets(int frame);
    void updateSettingsForCurrentCanvas();

    void replaceClipboard(ClipboardContainer *container);
    ClipboardContainer *getClipboardContainer(
            const ClipboardContainerType &type);
    void addCanvas(Canvas *newCanvas);

    Gradient *getLoadedGradientBySqlId(const int &id);
    void clearLoadedGradientsList();
public slots:
    void setCurrentFrame(int frame);
    void setGraphEnabled(bool graphEnabled);
    void setAllPointsRecord(bool allPointsRecord);
    //void playPreview();
   // void stopPreview();
    void setResolutionPercentId(int id);
    void createNewCanvas();

    void callUpdateSchedulers();
private slots:
    //void saveOutput(QString renderDest);
    //void renderOutput();
    //void sendNextBoxForUpdate();

    void newFile();
    bool askForSaving();
    void openFile();
    void saveFile();
    void saveFileAs();
    void saveBackup();
    void closeProject();
    void importFile();
    void linkFile();
    void importImageSequence();
    //void importVideo();
    void revert();
signals:
    void updateAll();
private:
    static MainWindow *mMainWindowInstance;
    MemoryHandler *mMemoryHandler;

    QList<ClipboardContainer*> mClipboardContainers;
//    bool mRendering = false;

    QComboBox *mCurrentCanvasComboBox;
//    bool mCancelLastBoxUpdate = false;
//    BoundingBox *mLastUpdatedBox = NULL;
//    QList<BoundingBox*> mBoxesAwaitingUpdate;
//    bool mNoBoxesAwaitUpdate = true;

    QDockWidget *mRightDock;
    QDockWidget *mBottomDock;
    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget;
    KeysView *mKeysView;

    QToolBar *mToolBar;

    ActionButton *mMovePathMode;
    ActionButton *mMovePointMode;
    ActionButton *mAddPointMode;
    ActionButton *mCircleMode;
//
    ActionButton *mRectangleMode;
    ActionButton *mTextMode;

    ActionButton *mParticleBoxMode;
    ActionButton *mParticleEmitterMode;
//
    ActionButton *mActionConnectPoints;
    ActionButton *mActionDisconnectPoints;
    ActionButton *mActionMergePoints;
//
    ActionButton *mActionSymmetricPointCtrls;
    ActionButton *mActionSmoothPointCtrls;
    ActionButton *mActionCornerPointCtrls;
//
    ActionButton *mActionLine;
    ActionButton *mActionCurve;
//
    QAction *mActionEffectsPaintEnabled;

    QPushButton *mNewCanvasButton;

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

    CanvasWidget *mCanvasWidget;
    UndoRedoStack mUndoRedoStack;
    bool mDetachedUndoRedoStack = false;
    UndoRedoStack *mCurrentUndoRedoStack;

    QList<UpdateScheduler*> mUpdateSchedulers;
    bool processKeyEvent(QKeyEvent *event);
    FillStrokeSettingsWidget *mFillStrokeSettings;

    bool mChangedSinceSaving = false;
    bool mEventFilterDisabled = false;
    bool isEnabled();
    QWidget *mGrayOutWidget = NULL;

    qreal mFPS = 24.;

    bool mAllPointsRecording = false;
    bool mDisplayedFillStrokeSettingsUpdateNeeded = false;

    bool mShiftPressed = false;
    bool mAltPressed = false;
    bool mCtrlPressed = false;

    BoxScrollWidget *mObjectSettingsWidget = NULL;
    ScrollArea *mObjectSettingsScrollArea;

    void setupToolBar();
    void connectToolBarActions();
    void setupMenuBar();

    QList<Gradient*> mLoadedGradientsList;
    void loadAllGradientsFromSql();
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
