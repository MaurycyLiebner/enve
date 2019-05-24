#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDockWidget>
#include <QMainWindow>
#include <QGraphicsView>
#include <QComboBox>
#include <QPushButton>
#include "undoredo.h"
#include "clipboardcontainer.h"
#include "taskscheduler.h"
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
class BrushSelectionWidget;
class Canvas;
class CanvasWindow;
class MemoryHandler;
class Task;

class ObjectSettingsWidget;
class BoxScrollWidget;
class ScrollWidget;
class ScrollArea;
class Brush;
class UsageWidget;
class Gradient;
class SimpleBrushWrapper;
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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

//    void (MainWindow::*mBoxesUpdateFinishedFunction)(void) = nullptr;

    static PropertyClipboardContainer* getPropertyClipboardContainer() {
        auto contT = getInstance()->getClipboardContainer(CCT_PROPERTY);
        return static_cast<PropertyClipboardContainer*>(contT);
    }

    static BoxesClipboardContainer* getBoxesClipboardContainer() {
        auto contT = getInstance()->getClipboardContainer(CCT_BOXES);
        return static_cast<BoxesClipboardContainer*>(contT);
    }

    static MainWindow *getInstance();
    static void addUndoRedo(const stdsptr<UndoRedo>& uR) {
        UndoRedoStack *stack = MainWindow::getInstance()->getUndoRedoStack();
        if(!stack) return;
        stack->addUndoRedo(uR);
    }

    void setCurrentUndoRedoStack(UndoRedoStack *stack) {
        mCurrentUndoRedoStack = stack;
    }
    UndoRedoStack *getUndoRedoStack();

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

    void updateSettingsForCurrentCanvas();

    void replaceClipboard(const stdsptr<ClipboardContainer> &container);
    ClipboardContainer *getClipboardContainer(
            const ClipboardContainerType &type);
    void addCanvas(const qsptr<Canvas> &newCanvas);

    Gradient *getLoadedGradientById(const int &id);
    void clearLoadedGradientsList();
    void previewBeingPlayed();
    void previewBeingRendered();
    void previewPaused();

    void addLoadedGradient(Gradient * const gradient);
    FontsWidget *getFontsWidget() const {
        return mFontWidget;
    }
    SimpleBrushWrapper *getCurrentBrush() const;

    UsageWidget* getUsageWidget() const {
        return mUsageWidget;
    }

    void setCurrentFrame(const int &frame);
    //void playPreview();
   // void stopPreview();
    void setResolutionFractionValue(const qreal &value);
    void createNewCanvas();

    void queScheduledTasksAndUpdate();
    void addCanvasToRenderQue();
    void canvasNameChanged(Canvas *canvas, const QString &name);

    const QStringList& getRecentFiles() const {
        return mRecentFiles;
    }
    stdsptr<void> lock();
public:
    //void saveOutput(QString renderDest);
    //void renderOutput();
    //void sendNextBoxForUpdate();

    void newFile();
    bool askForSaving();
    void openFile();
    void openFile(const QString& openPath);
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
protected:
    void lockFinished();
private:
    friend class Lock;
    class Lock : public StdSelfRef {
        friend class StdSelfRef;
    protected:
        Lock(MainWindow * const window) : mWindow(window) {}
    public:
        ~Lock() { mWindow->lockFinished(); }
    private:
        MainWindow * const mWindow;
    };
    stdptr<Lock> mLock;
    static MainWindow *mMainWindowInstance;
    MemoryHandler *mMemoryHandler;

    void updateRecentMenu();

    void addRecentFile(const QString& recent) {
        if(mRecentFiles.contains(recent))
            mRecentFiles.removeOne(recent);
        while(mRecentFiles.count() >= 8)
            mRecentFiles.removeLast();
        mRecentFiles.prepend(recent);
        updateRecentMenu();
        writeRecentFiles();
    }

    void readRecentFiles() {
        QFile file(QDir::homePath() + "/.AniVect/recent");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            while(!stream.atEnd()) {
                const QString path = stream.readLine();
                if(path.isEmpty()) continue;
                //if(QFile(path).exists())
                mRecentFiles.append(path);
            }
        }
    }

    void writeRecentFiles() {
        QFile file(QDir::homePath() + "/.AniVect/recent");
        if(file.open(QIODevice::WriteOnly | QIODevice::Text |
                     QIODevice::Truncate)) {
            QTextStream stream(&file);
            for(const auto& recent : mRecentFiles) {
                stream << recent << endl;
            }
        }
    }

    QStringList mRecentFiles;

    stdsptr<ClipboardContainer> mClipboardContainer;
//    bool mRendering = false;

    QComboBox *mCurrentCanvasComboBox;
//    bool mCancelLastBoxUpdate = false;
//    BoundingBox *mLastUpdatedBox = nullptr;
//    QList<BoundingBox*> mBoxesAwaitingUpdate;
//    bool mNoBoxesAwaitUpdate = true;

    QDockWidget *mFillStrokeSettingsDock;
    QDockWidget *mBottomDock;
    QDockWidget *mLeftDock;
    QDockWidget *mLeftDock2;
    QDockWidget *mBrushSettingsDock;
    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget = nullptr;
    BrushSelectionWidget *mBrushSelectionWidget = nullptr;

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
    QAction *mLowQuality;
    QAction *mMediumQuality;
    QAction *mHighQuality;
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
    QMenu *mRecentMenu;
    QMenu *mSelectSameMenu;
    QMenu *mEditMenu;
    QMenu *mObjectMenu;
    QMenu *mPathMenu;
    QMenu *mEffectsMenu;
    QMenu *mViewMenu;
    QMenu *mPanelsMenu;
    QMenu *mRenderMenu;

    TaskScheduler mTaskScheduler;
    CanvasWindow *mCanvasWindow;
    stdptr<UndoRedoStack> mCurrentUndoRedoStack;

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

    stdsptr<VideoEncoder> mVideoEncoder;
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
