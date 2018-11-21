#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDockWidget>
#include <QMainWindow>
#include <QGraphicsView>
#include <QComboBox>
#include <QPushButton>
#include "undoredo.h"
class VideoEncoder;
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
class BrushSettingsWidget;
class Canvas;
class CanvasWindow;
class MemoryHandler;
class _ScheduledExecutor;

class ObjectSettingsWidget;
class BoxScrollWidget;
class ScrollWidget;
class ScrollArea;
class Brush;
class UsageWidget;
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

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

//    void (MainWindow::*mBoxesUpdateFinishedFunction)(void) = nullptr;

    static MainWindow *getInstance();
    static void addUndoRedo(UndoRedo *uR) {
        UndoRedoStack *stack = MainWindow::getInstance()->getUndoRedoStack();
        if(stack == nullptr) return;
        stack->addUndoRedo(uR);
    }

    void setCurrentUndoRedoStack(UndoRedoStack *stack) {
        mCurrentUndoRedoStack = stack;
    }
    UndoRedoStack *getUndoRedoStack();

    void addUpdateScheduler(const std::shared_ptr<_ScheduledExecutor> &scheduler);

    static bool isShiftPressed();
    static bool isCtrlPressed();
    static bool isAltPressed();

    void schedulePivotUpdate();

    AnimationDockWidget *getAnimationDockWidget();
    BoxScrollWidget *getObjectSettingsList();

    CanvasWindow *getCanvasWindow() { return mCanvasWindow; }
    FillStrokeSettingsWidget *getFillStrokeSettings();
    void saveToFile(const QString &path);
    void loadAVFile(const QString &path);
    void clearAll();
    void setCurrentPath(QString newPath);
    void updateTitle();
    void setFileChangedSinceSaving(bool changed);
    void disableEventFilter();
    void enableEventFilter();

    void scheduleBoxesListRepaint();
    void disable();
    void enable();

    int getCurrentFrame();

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

    void setCurrentFrameForAllWidgets(const int &frame);
    void updateSettingsForCurrentCanvas();

    void replaceClipboard(ClipboardContainer *container);
    ClipboardContainer *getClipboardContainer(
            const ClipboardContainerType &type);
    void addCanvas(Canvas *newCanvas);

    Gradient *getLoadedGradientById(const int &id);
    void clearLoadedGradientsList();
    void previewBeingPlayed();
    void previewBeingRendered();
    void previewPaused();

    void incBrushRadius();
    void decBrushRadius();
    void addLoadedGradient(Gradient *gradient);
    FontsWidget *getFontsWidget() {
        return mFontWidget;
    }
    void addFileUpdateScheduler(const std::shared_ptr<_ScheduledExecutor>& scheduler);
    void finishUndoRedoSet();
    Brush *getCurrentBrush();

    UsageWidget* getUsageWidget() {
        return mUsageWidget;
    }
public slots:
    void setCurrentFrame(int frame);
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
    bool closeProject();
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

    ClipboardContainer *mClipboardContainer = nullptr;
//    bool mRendering = false;

    QComboBox *mCurrentCanvasComboBox;
//    bool mCancelLastBoxUpdate = false;
//    BoundingBox *mLastUpdatedBox = nullptr;
//    QList<BoundingBox*> mBoxesAwaitingUpdate;
//    bool mNoBoxesAwaitUpdate = true;

    QDockWidget *mRightDock;
    QDockWidget *mBottomDock;
    QDockWidget *mLeftDock;
    QDockWidget *mLeftDock2;
    QDockWidget *mBrushSettingsDock;
    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget = nullptr;
    BrushSettingsWidget *mBrushSettingsWidget = nullptr;

    QStatusBar* mStatusBar;
    UsageWidget* mUsageWidget = nullptr;
    QToolBar *mToolBar;

    ActionButton *mMovePathMode;
    ActionButton *mMovePointMode;
    ActionButton *mAddPointMode;
    ActionButton *mPickPaintSettingsMode;

    ActionButton *mCircleMode;
//
    ActionButton *mRectangleMode;
    ActionButton *mTextMode;
    ActionButton *mParticleBoxMode;
    ActionButton *mParticleEmitterMode;

    ActionButton *mPaintBoxMode;
    ActionButton *mPaintMode;
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
    QAction *mClipViewToCanvas;
    QAction *mRasterEffectsVisible;
    QAction *mPathEffectsVisible;
    QAction *mCurrentObjectDock;
    QAction *mFilesDock;
    QAction *mObjectsAndAnimationsDock;
    QAction *mFillAndStrokeSettingsDock;
    QAction *mBrushSettingsDockAction;

    QPushButton *mNewCanvasButton;

    FontsWidget *mFontWidget = nullptr;

    QString mCurrentFilePath = "";

    QMenuBar *mMenuBar;
    QMenu *mFileMenu;
    QMenu *mSelectSameMenu;
    QMenu *mEditMenu;
    QMenu *mObjectMenu;
    QMenu *mPathMenu;
    QMenu *mEffectsMenu;
    QMenu *mViewMenu;
    QMenu *mPanelsMenu;
    QMenu *mRenderMenu;

    CanvasWindow *mCanvasWindow;
    UndoRedoStack *mCurrentUndoRedoStack = nullptr;

    QList<std::shared_ptr<_ScheduledExecutor> > mUpdateSchedulers;
    bool processKeyEvent(QKeyEvent *event);
    FillStrokeSettingsWidget *mFillStrokeSettings;

    bool mChangedSinceSaving = false;
    bool mEventFilterDisabled = true;
    bool isEnabled();
    QWidget *mGrayOutWidget = nullptr;
    bool mDisplayedFillStrokeSettingsUpdateNeeded = false;

    BoxScrollWidget *mObjectSettingsWidget = nullptr;
    ScrollArea *mObjectSettingsScrollArea;

    void setupStatusBar();
    void setupToolBar();
    void connectToolBarActions();
    void setupMenuBar();

    QList<Gradient*> mLoadedGradientsList;

    std::shared_ptr<VideoEncoder> mVideoEncoder;
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
