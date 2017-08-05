#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
#include "Paint/paintcanvas.h"
struct TileSkDrawer;

struct PaintBoxRenderData : public BoundingBoxRenderData {
    void drawSk(SkCanvas *canvas);

    void updateRelBoundingRect();

    QList<TileSkDrawer> tileDrawers;
};

class PaintBox : public BoundingBox,
                 public PaintCanvas {
public:
    PaintBox();

    bool SWT_isPaintBox() { return true; }
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void renderDataFinished(BoundingBoxRenderData *renderData);
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);
    void updateDrawRenderContainerTransform();
    void mapToPaintCanvasHandler(qreal *x_t, qreal *y_t);
private:
    RenderContainer mTemporaryOverlayCont;
    sk_sp<SkImage> mLastPaintImage;
};

#endif // PAINTBOX_H
