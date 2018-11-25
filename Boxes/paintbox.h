#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
#include "Paint/PaintLib/tile.h"
struct TileSkDrawer;
class Surface;
class Brush;
class AnimatedSurface;
class QPointFAnimator;

struct PaintBoxRenderData : public BoundingBoxRenderData {
    PaintBoxRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {

    }

    void drawSk(SkCanvas *canvas);

    QList<TileSkDrawerCollection> tileDrawers;

    SkPoint trans;
};

class PaintBox : public BoundingBox {
public:
    PaintBox();
    PaintBox(const ushort &canvasWidthT,
             const ushort &canvasHeightT);

    void setSize(const ushort &width,
                 const ushort &height) {
        mWidth = width;
        mHeight = height;
    }

    const ushort &getWidth() {
        return mWidth;
    }

    const ushort &getHeight() {
        return mHeight;
    }

    void prp_setAbsFrame(const int &frame);

    void finishSizeSetup();
    bool SWT_isPaintBox() { return true; }
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void processSchedulers();
    void renderDataFinished(BoundingBoxRenderData *renderData);
    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                BoundingBoxRenderData* data);
    BoundingBox *createNewDuplicate() {
        return new PaintBox(100, 100);
    }

    BoundingBoxRenderDataSPtr createRenderData();

    void tabletMoveEvent(const qreal &xT,
                     const qreal &yT,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase,
                         const Brush *brush);
    void tabletReleaseEvent();
    void tabletPressEvent(const qreal &xT,
                          const qreal &yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          const Brush *brush);
    void mouseReleaseEvent();
    void mousePressEvent(const qreal &xT,
                         const qreal &yT,
                         const ulong &timestamp,
                         const qreal &pressure,
                         const Brush *brush);
    void mouseMoveEvent(const qreal &xT,
                        const qreal &yT,
                        const ulong &time_stamp,
                        const bool &erase,
                        const Brush *brush);
    MovablePoint *getTopLeftPoint();
    MovablePoint *getBottomRightPoint();
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void startAllPointsTransform();

    void newPaintFrameOnCurrentFrame();
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void newEmptyPaintFrameOnCurrentFrame();
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    void newEmptyPaintFrameAtFrame(const int &relFrame);

    void setOverlapFrames(const int &overlapFrames);

    const int &getOverlapFrames() {
        return mOverlapFrames;
    }
    void setFrameStep(const int &frameStep) {
        mFrameStep = frameStep;
    }

    const int &getFrameStep() {
        return mFrameStep;
    }

    void finishSizeAndPosSetup();
    void scheduleFinishSizeSetup();
    void scheduleFinishSizeAndPosSetup();
    void loadFromImage(const QImage &img);
    bool prp_nextRelFrameWithKey(const int &relFrame,
                                 int &nextRelFrame);
    bool prp_prevRelFrameWithKey(const int &relFrame,
                                 int &prevRelFrame);

    const bool &isDraft() const;

    void setIsDraft(const bool &bT);

    bool isSurfaceAnimated();
private:
    bool mFinishSizeAndPosSetupScheduled = false;
    bool mFinishSizeSetupScheduled = false;

    int mOverlapFrames = 1;
    int mFrameStep = 1;
    QPointFAnimator *mTopLeftPoint = nullptr;
    QPointFAnimator *mBottomRightPoint = nullptr;
    ushort mWidth = 0;
    ushort mHeight = 0;
    AnimatedSurface *mMainHandler = nullptr;
    Surface *mTemporaryHandler = nullptr;
};

#endif // PAINTBOX_H
