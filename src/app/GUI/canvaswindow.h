#ifndef CANVASWINDOW_H
#define CANVASWINDOW_H

#include <QWidget>
#include "glwindow.h"
#include "singlewidgettarget.h"
#include "keyfocustarget.h"
#include "smartPointers/sharedpointerdefs.h"
#include "Tasks/gpupostprocessor.h"
#include "canvas.h"
class Brush;
class WindowSingleWidgetTarget;
enum class ColorMode : short;
enum CanvasMode : short;
class Gradient;
class BoundingBox;
class ContainerBox;
class TaskExecutor;
class SoundComposition;
class PaintSettingsApplier;
class RenderInstanceSettings;
class Task;
class ImageBox;
class SingleSound;
class VideoBox;
class Canvas;
class PaintSettingsAnimator;
class OutlineSettingsAnimator;
class SimpleBrushWrapper;
class Actions;

class CanvasWindow : public GLWindow, public KeyFocusTarget {
    Q_OBJECT
public:
    explicit CanvasWindow(Document& document,
                          QWidget * const parent = nullptr);
    ~CanvasWindow();
    Canvas *getCurrentCanvas();
    const QMatrix& getViewTransform() const { return mViewTransform; }
    void blockAutomaticSizeFit();
    void unblockAutomaticSizeFit();
    void setViewTransform(const QMatrix& transform) {
        mViewTransform = transform; }
    void openWelcomeDialog();
    void closeWelcomeDialog();

    void setCurrentCanvas(Canvas * const canvas);
    bool hasNoCanvas();

    void queTasksAndUpdate();
    bool KFT_handleKeyEventForTarget(QKeyEvent *event);

    void setResolutionFraction(const qreal percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    ContainerBox *getCurrentGroup();
    void applyPaintSettingToSelected(const PaintSettingsApplier &setting);
    void setSelectedFillColorMode(const ColorMode mode);
    void setSelectedStrokeColorMode(const ColorMode mode);

    int getCurrentFrame();
    int getMaxFrame();

    void grabMouse();
    void releaseMouse();
    bool isMouseGrabber();

    void openSettingsWindowForCurrentCanvas();

    bool event(QEvent *e);

    void resizeEvent(QResizeEvent* e);

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
protected:
    void KFT_setFocusToWidget() {
        if(mCurrentCanvas) mDocument.setActiveScene(mCurrentCanvas);
        setFocus();
        update();
    }

    void KFT_clearFocus() {
        if(mCurrentCanvas) mDocument.setActiveScene(nullptr);
        clearFocus();
        update();
    }
private:
    void setCanvasMode(const CanvasMode mode);
    void updatePaintModeCursor();

    QWidget * mWelcomeDialog = nullptr;

    Document& mDocument;
    Actions& mActions;

    QSize mOldSize{-1, -1};
    QMatrix mViewTransform;
    QPointF mPrevMousePos;
    QPointF mPrevPressPos;
    bool mValidPaintTarget = false;

    bool mBlockInput = false;
    bool mMouseGrabber = false;

    qsptr<WindowSingleWidgetTarget> mWindowSWTTarget;

    Canvas* mCurrentCanvas = nullptr;

    //void paintEvent(QPaintEvent *);

    void renderSk(SkCanvas * const canvas,
                  GrContext * const grContext);
    void tabletEvent(QTabletEvent *e);

    bool handleCanvasModeChangeKeyPress(QKeyEvent *event);
    bool handleCutCopyPasteKeyPress(QKeyEvent *event);
    bool handleTransformationKeyPress(QKeyEvent *event);
    bool handleZValueKeyPress(QKeyEvent *event);
    bool handleParentChangeKeyPress(QKeyEvent *event);
    bool handleGroupChangeKeyPress(QKeyEvent *event);
    bool handleResetTransformKeyPress(QKeyEvent *event);
    bool handleRevertPathKeyPress(QKeyEvent *event);
    bool handleStartTransformKeyPress(const KeyEvent &e);
    bool handleSelectAllKeyPress(QKeyEvent *event);
    bool handleShiftKeysKeyPress(QKeyEvent *event);    
signals:
    void changeCanvasFrameRange(FrameRange);
public:
    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int id);

    QPointF mapToCanvasCoord(const QPointF& windowCoord);
    void translateView(const QPointF &trans);
    void zoomView(const qreal scaleBy, const QPointF &absOrigin);

    void fitCanvasToSize();
    void requestFitCanvasToSize();
    void resetTransormation();
};

#endif // CANVASWINDOW_H
