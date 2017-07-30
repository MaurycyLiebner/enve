#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDockWidget>
#include <QMainWindow>
#include <QGraphicsView>
#include <QComboBox>
#include <QPushButton>
#include <QSqlQuery>
#include "undoredo.h"

enum ClipboardContainerType : short;

class ClipboardContainer;
class ActionButton;
class BoxesList;
class FontsWidget;
class UpdateScheduler;
class AnimationDockWidget;
class ColorSettingsWidget;
class FillStrokeSettingsWidget;
class BoxesListAnimationDockWidget;
class Canvas;
class CanvasWindow;
class MemoryHandler;
class Updatable;

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

    void addUpdateScheduler(Updatable *scheduler);

    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    void schedulePivotUpdate();

    AnimationDockWidget *getAnimationDockWidget();
    BoxScrollWidget *getObjectSettingsList();

    CanvasWindow *getCanvasWindow() { return mCanvasWindow; }
    FillStrokeSettingsWidget *getFillStrokeSettings();
    void saveToFile(QString path);
    void loadAVFile(QString path);
    void clearAll();
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

    void setResolutionFraction(qreal percent);

    void setCurrentFrameForAllWidgets(int frame);
    void updateSettingsForCurrentCanvas();

    void replaceClipboard(ClipboardContainer *container);
    ClipboardContainer *getClipboardContainer(
            const ClipboardContainerType &type);
    void addCanvas(Canvas *newCanvas);

    Gradient *getLoadedGradientBySqlId(const int &id);
    void clearLoadedGradientsList();
    void previewBeingPlayed();
    void previewBeingRendered();
    void previewPaused();

    void blockUndoRedo() {
        mUndoRedoStack.blockUndoRedo();
    }

    void unblockUndoRedo() {
        mUndoRedoStack.unblockUndoRedo();
    }
public slots:
    void setCurrentFrame(int frame);
    void setAllPointsRecord(bool allPointsRecord);
    //void playPreview();
   // void stopPreview();
    void setResolutionFractionValue(const qreal &value);
    void createNewCanvas();

    void callUpdateSchedulers();
    void addCanvasToRenderQue();
    void canvasNameChanged(Canvas *canvas, const QString &name);
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
    void linkFile();
    void importImageSequence();
    //void importVideo();
    void revert();

    void undo();
    void redo();

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
    QDockWidget *mLeftDock;
    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget;

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

    CanvasWindow *mCanvasWindow;
    UndoRedoStack mUndoRedoStack;
    bool mDetachedUndoRedoStack = false;
    UndoRedoStack *mCurrentUndoRedoStack;

    QList<std::shared_ptr<Updatable> > mUpdateSchedulers;
    bool processKeyEvent(QKeyEvent *event);
    FillStrokeSettingsWidget *mFillStrokeSettings;

    bool mChangedSinceSaving = false;
    bool mEventFilterDisabled = true;
    bool isEnabled();
    QWidget *mGrayOutWidget = NULL;
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
