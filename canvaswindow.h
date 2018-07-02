#ifndef CANVASWINDOW_H
#define CANVASWINDOW_H

#include <QWidget>
#include "glwindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"
#include "keyfocustarget.h"
class Brush;
class WindowSingleWidgetTarget;
class Canvas;
typedef QSharedPointer<Canvas> CanvasQSPtr;
enum ColorMode : short;
enum CanvasMode : short;
class Color;
class Gradient;
class BoundingBox;
class BoxesGroup;
class PaintControler;
class SoundComposition;
class PaintSetting;
class CanvasWidget;
class RenderInstanceSettings;
class _ScheduledExecutor;
class _Executor;
class ImageBox;
class SingleSound;
class VideoBox;

#include <QAudioOutput>

class CanvasWindow : public GLWindow,
        public KeyFocusTarget {
    Q_OBJECT
public:
    explicit CanvasWindow(QWidget *parent);
    ~CanvasWindow();

    Canvas *getCurrentCanvas();
    const QList<CanvasQSPtr> &getCanvasList() {
        return mCanvasList;
    }

    void setCurrentCanvas(Canvas *canvas);
    void addCanvasToList(Canvas *canvas);
    void removeCanvas(const int &id);
    void addCanvasToListAndSetAsCurrent(Canvas *canvas);
    void renameCanvas(Canvas *canvas, const QString &newName);
    void renameCanvas(const int &id, const QString &newName);
    bool hasNoCanvas();
    void setCanvasMode(const CanvasMode &mode);

    void callUpdateSchedulers();
    bool KFT_handleKeyEventForTarget(QKeyEvent *event);
    void KFT_setFocusToWidget() {
        setFocus();
        requestUpdate();
    }

    void KFT_clearFocus() {
        clearFocus();
    }

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void strokeCapStyleChanged(const Qt::PenCapStyle &capStyle);
    void strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle);
    void strokeWidthChanged(const qreal &strokeWidth,
                            const bool &finish);
    void updateDisplayedFillStrokeSettings();

    void setResolutionFraction(const qreal &percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    BoxesGroup *getCurrentGroup();
    void applyPaintSettingToSelected(const PaintSetting &setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);

    void updateAfterFrameChanged(const int &currentFrame);
    void clearAll();
    void createAnimationBoxForPaths(const QStringList &importPaths);
    void createLinkToFileWithPath(const QString &path);
    VideoBox *createVideoForPath(const QString &path);
    int getCurrentFrame();
    int getMaxFrame();
    void addUpdatableAwaitingUpdate(_Executor *updatable);
    void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);
    ImageBox *createImageForPath(const QString &path);
    SingleSound *createSoundForPath(const QString &path);
    void updateHoveredElements();

    void setLocalPivot(const bool &bT);
    void setBonesSelectionEnabled(const bool &bT);

    void importFile(const QString &path,
                    const QPointF &relDropPos = QPointF(0., 0.));

    QWidget *getCanvasWidget();

    void grabMouse();

    bool hasFocus() {
        return mHasFocus;
    }

    void setFocus() {
        mHasFocus = true;
    }

    void clearFocus() {
        mHasFocus = false;
    }

    void repaint();

    QRect rect();

    void releaseMouse();

    bool isMouseGrabber();

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void openSettingsWindowForCurrentCanvas();

    void rotate90CCW();
    void rotate90CW();
    void processSchedulers();
    bool shouldProcessAwaitingSchedulers();
    void writeCanvases(QIODevice *target);
    void readCanvases(QIODevice *target);
    void addFileUpdatableAwaitingUpdate(_Executor *updatable);
    WindowSingleWidgetTarget *getWindowSWT() {
        return mWindowSWTTarget;
    }

    void startDurationRectPosTransformForAllSelected();
    void finishDurationRectPosTransformForAllSelected();
    void moveDurationRectForAllSelected(const int &dFrame);
    void startMinFramePosTransformForAllSelected();
    void finishMinFramePosTransformForAllSelected();
    void moveMinFrameForAllSelected(const int &dFrame);
    void startMaxFramePosTransformForAllSelected();
    void finishMaxFramePosTransformForAllSelected();
    void moveMaxFrameForAllSelected(const int &dFrame);
protected:
    WindowSingleWidgetTarget *mWindowSWTTarget = nullptr;
    PaintControler *mFileControler = nullptr;
    RenderInstanceSettings *mCurrentRenderSettings = nullptr;
    bool mMouseGrabber = false;
    bool mHasFocus = false;
    QWidget *mCanvasWidget;
    void setRendering(const bool &bT);
    void setPreviewing(const bool &bT);

    QTimer *mPreviewFPSTimer = nullptr;

    qreal mSavedResolutionFraction = 100.;
    int mSavedCurrentFrame = 0;
    bool mPreviewing = false;
    bool mRendering = false;
    int mMaxRenderFrame = 0;

    QList<int> mFreeThreads;
    bool mNoBoxesAwaitUpdate = true;
    bool mNoFileAwaitUpdate = true;
    QList<QThread*> mControlerThreads;
    QThread *mFileControlerThread;
    QList<PaintControler*> mPaintControlers;
    QList<std::shared_ptr<_Executor> > mUpdatablesAwaitingUpdate;
    QList<std::shared_ptr<_Executor> > mFileUpdatablesAwaitingUpdate;

    int mCurrentRenderFrame;

    bool mClearBeingUpdated = false;

    void (CanvasWindow::*mBoxesUpdateFinishedFunction)(void) = nullptr;
    void (CanvasWindow::*mFilesUpdateFinishedFunction)(void) = nullptr;

    CanvasQSPtr mCurrentCanvas;
    QList<CanvasQSPtr> mCanvasList;

    //void paintEvent(QPaintEvent *);

    void nextCurrentRenderFrame();


    // AUDIO
    void initializeAudio();
    void startAudio();
    void stopAudio();
    void volumeChanged(int value);

    QAudioDeviceInfo mAudioDevice;
    SoundComposition *mCurrentSoundComposition;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioIOOutput; // not owned
    QAudioFormat mAudioFormat;

    QByteArray mAudioBuffer;
    // AUDIO

    void qRender(QPainter *p);
    void renderSk(SkCanvas *canvas);
    void tabletEvent(QTabletEvent *e);
signals:
    void updateUpdatable(_Executor*, int);
    void updateFileUpdatable(_Executor*, int);

    void changeCurrentFrame(int);
    void changeFrameRange(int, int);
public slots:
    void setCurrentBrush(const Brush *brush);
    void replaceBrush(const Brush *oldBrush,
                      const Brush *newBrush);

    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setPickPaintSettingsMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();
    void setParticleBoxMode();
    void setParticleEmitterMode();
    void setPaintBoxMode();
    void setPaintMode();

    void raiseAction();
    void lowerAction();
    void raiseToTopAction();
    void lowerToBottomAction();

    void objectsToPathAction();
    void strokeToPathAction();

    void setFontFamilyAndStyle(QString family, QString style);
    void setFontSize(qreal size);

    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();

    void makeSegmentLine();
    void makeSegmentCurve();

    void pathsUnionAction();
    void pathsDifferenceAction();
    void pathsIntersectionAction();
    void pathsDivisionAction();
    void pathsExclusionAction();
    void pathsCombineAction();
    void pathsBreakApartAction();

    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int &id);

    void setClipToCanvas(const bool &bT);
    void setRasterEffectsVisible(const bool &bT);
    void setPathEffectsVisible(const bool &bT);

    void interruptPreview();
    void outOfMemory();
    void interruptPreviewRendering();
    void interruptOutputRendering();

    void playPreview();
    void stopPreview();
    void pausePreview();
    void resumePreview();
    void renderPreview();
    void renderFromSettings(RenderInstanceSettings *settings);

    void importFile();

    void startSelectedStrokeWidthTransform();

    void deleteAction();
    void copyAction();
    void pasteAction();
    void cutAction();
    void duplicateAction();
    void selectAllAction();
    void invertSelectionAction();
    void clearSelectionAction();

    void groupSelectedBoxes();
    void ungroupSelectedBoxes();
    void rotate90CWAction();
    void rotate90CCWAction();

    void flipHorizontalAction();
    void flipVerticalAction();
private slots:
    void sendNextUpdatableForUpdate(const int &threadId,
                                    _Executor *lastUpdatable = nullptr);
    void sendNextFileUpdatableForUpdate(const int &threadId,
                                    _Executor *lastUpdatable = nullptr);
    void nextSaveOutputFrame();
    void nextPreviewRenderFrame();

    void pushTimerExpired();
};

#endif // CANVASWINDOW_H
