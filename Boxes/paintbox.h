#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
struct TileSkDrawer;
class Surface;
class Brush;
class AnimatedSurface;
class PointAnimator;

struct PaintBoxRenderData : public BoundingBoxRenderData {
    PaintBoxRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {

    }

    void drawSk(SkCanvas *canvas);

    QList<TileSkDrawer*> tileDrawers;
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

    void prp_setAbsFrame(const int &frame);

    void finishSizeSetup();
    bool SWT_isPaintBox() { return true; }
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void processSchedulers();
    void renderDataFinished(BoundingBoxRenderData *renderData);
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);
    BoundingBox *createNewDuplicate() {
        return new PaintBox(100, 100);
    }

    BoundingBoxRenderData *createRenderData();

    void tabletEvent(const qreal &xT,
                     const qreal &yT,
                     const ulong &time_stamp,
                     const qreal &pressure,
                     const bool &erase, Brush *brush);
    void tabletReleaseEvent();
    void tabletPressEvent(const qreal &xT,
                          const qreal &yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          Brush *brush);
    void mouseReleaseEvent();
    void mousePressEvent(const qreal &xT,
                         const qreal &yT,
                         const ulong &timestamp,
                         const qreal &pressure,
                         Brush *brush);
    void mouseMoveEvent(const qreal &xT,
                        const qreal &yT,
                        const ulong &time_stamp,
                        const bool &erase,
                        Brush *brush);
    void paintPress(const qreal &xT,
                    const qreal &yT,
                    const ulong &timestamp,
                    const qreal &pressure,
                    Brush *brush);

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
private:
    PointAnimator *mTopLeftPoint = NULL;
    PointAnimator *mBottomRightPoint = NULL;
    ushort mWidth = 0;
    ushort mHeight = 0;
    AnimatedSurface *mMainHandler = NULL;
    Surface *mTemporaryHandler = NULL;
};

#endif // PAINTBOX_H
