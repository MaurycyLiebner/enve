#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
#include "Paint/canvashandler.h"
struct TileSkDrawer;

struct PaintBoxRenderData : public BoundingBoxRenderData {
    PaintBoxRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {

    }

    void drawSk(SkCanvas *canvas);

    void updateRelBoundingRect();

    QList<TileSkDrawer*> tileDrawers;
};

class PaintBox : public BoundingBox,
                 public CanvasHandler {
public:
    PaintBox(const ushort &canvasWidthT,
             const ushort &canvasHeightT);

    bool SWT_isPaintBox() { return true; }
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void renderDataFinished(BoundingBoxRenderData *renderData);
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);
    void updateDrawRenderContainerTransform();
    void mapToPaintCanvasHandler(qreal *x_t, qreal *y_t);
    BoundingBox *createNewDuplicate() {
        return new PaintBox(100, 100);
    }

    BoundingBoxRenderData *createRenderData();
private:
    RenderContainer mTemporaryOverlayCont;
    sk_sp<SkImage> mLastPaintImage;
};

#endif // PAINTBOX_H
