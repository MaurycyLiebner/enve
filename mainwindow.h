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
#include "vectorshapesmenu.h"
#include "actionbutton.h"

class PaintControler;

class BoxesList;

class UpdateScheduler;

class ColorSettingsWidget;

class BoxesListAnimationDockWidget;

class CanvasWidget;

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

    void (MainWindow::*mBoxesUpdateFinishedFunction)(void) = NULL;

    static MainWindow *getInstance();

    void createDetachedUndoRedoStack();
    void deleteDetachedUndoRedoStack();

    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(UpdateScheduler *scheduler);

    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    void callUpdateSchedulers();
    void schedulePivotUpdate();

    AnimationDockWidget *getAnimationDockWidget();
    KeysView *getKeysView();
    BoxScrollWidget *getBoxesList();
    BoxScrollWidget *getObjectSettingsList();

    CanvasWidget *getCanvasWidget() { return mCanvasWidget; }
    FillStrokeSettingsWidget *getFillStrokeSettings();
    void saveToFile(QString path);
    void loadFile(QString path);
    void clearAll();
    void importFile(QString path, bool loadInBox);
    void exportSelected(QString path);
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

    int getMinFrame();
    int getMaxFrame();

    void previewFinished();
    void updateDisplayedFillStrokeSettings();
    void scheduleDisplayedFillStrokeSettingsUpdate();
    void updateDisplayedFillStrokeSettingsIfNeeded();
    void updateCanvasModeButtonsChecked();

    void addBoxAwaitingUpdate(BoundingBox *box);
    void setCurrentShapesMenuBox(BoundingBox *box);
    void setCurrentObjectSettingsWidgetBox(BoundingBox *box);
    void setCurrentBox(BoundingBox *box);

    void nextSaveOutputFrame();
    void nextPlayPreviewFrame();

    void setResolutionPercent(qreal percent);

    void setCurrentFrameForAllWidgets(int frame);
    void updateDisplayedShapesInMenu();
    void updateSettingsForCurrentCanvas();

    void replaceClipboard(ClipboardContainer *container);
    void pasteFromClipboard(const ClipboardContainerType &type);
public slots:
    void setCurrentFrame(int frame);
    void setGraphEnabled(bool graphEnabled);
    void setAllPointsRecord(bool allPointsRecord);
    void playPreview();
    void stopPreview();
    void setResolutionPercentId(int id);
    void createNewCanvas();
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
    void linkFile();
    void importAnimation();
    void exportSelected();
    void revert();
signals:
    void updateBoxPixmaps(BoundingBox*);
private:
    static MainWindow *mMainWindowInstance;

    QList<ClipboardContainer*> mClipboardContainers;
    bool mRendering = false;

    QComboBox *mCurrentCanvasComboBox;
    bool mCancelLastBoxUpdate = false;
    BoundingBox *mLastUpdatedBox = NULL;
    QList<BoundingBox*> mBoxesAwaitingUpdate;
    bool mNoBoxesAwaitUpdate = true;

    QThread *mPaintControlerThread;
    PaintControler *mPaintControler;

    QDockWidget *mRightDock;
    QDockWidget *mBottomDock;
    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget;
    BoxScrollWidget *mBoxListWidget;
    KeysView *mKeysView;

    QToolBar *mToolBar;

    ActionButton *mMovePathMode;
    ActionButton *mMovePointMode;
    ActionButton *mAddPointMode;
    ActionButton *mCircleMode;
//
    ActionButton *mRectangleMode;
    ActionButton *mTextMode;
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
    QAction *mActionHighQualityView;
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

//    SoundComposition *mSoundComposition = NULL;

    Canvas *mCanvas;
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

    BoxScrollWidget *mObjectSettingsWidget = NULL;
    ScrollArea *mObjectSettingsScrollArea;

    void setupToolBar();
    void connectToolBarActions();
    void setupMenuBar();
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *, QEvent *e);
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
