#include "paintbox.h"
#include "Paint/layer.h"

PaintBox::PaintBox(const ushort &canvasWidthT,
                   const ushort &canvasHeightT) : BoundingBox(TYPE_PAINT) {
    mMainHandler = new CanvasHandler(canvasWidthT, canvasHeightT, true);
    mTemporaryHandler = new CanvasHandler(canvasWidthT/4, canvasHeightT/4,
                                          false);
}

void PaintBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    canvas->saveLayer(NULL, paint);
    BoundingBox::drawPixmapSk(canvas, NULL);
    canvas->concat(
            QMatrixToSkMatrix(
                mTransformAnimator->getCombinedTransform()) );
    canvas->scale(4., 4.);
    mTemporaryHandler->drawSk(canvas, paint);
    canvas->restore();
}

void PaintBox::renderDataFinished(BoundingBoxRenderData *renderData) {
    BoundingBox::renderDataFinished(renderData);
    mTemporaryHandler->clear();
}

void PaintBox::setupBoundingBoxRenderDataForRelFrame(
        const int &relFrame, BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    PaintBoxRenderData *paintData = (PaintBoxRenderData*)data;
    mMainHandler->getTileDrawers(&paintData->tileDrawers);
    foreach(TileSkDrawer *drawer, paintData->tileDrawers) {
        if(!drawer->finished()) {
            drawer->addDependent(paintData);
        }
    }
}

BoundingBoxRenderData *PaintBox::createRenderData() {
    return new PaintBoxRenderData(this);
}

void PaintBox::tabletEvent(const qreal &xT,
                           const qreal &yT,
                           const ulong &time_stamp,
                           const qreal &pressure,
                           const bool &erase) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));
    mMainHandler->tabletEvent(relPos.x(), relPos.y(),
                              time_stamp, pressure,
                              erase);
    relPos *= 0.25;
    mTemporaryHandler->tabletEvent(relPos.x(), relPos.y(),
                              time_stamp, pressure,
                              erase);
}

void PaintBox::tabletReleaseEvent() {
    mMainHandler->tabletReleaseEvent();
    mTemporaryHandler->tabletReleaseEvent();
}

void PaintBox::tabletPressEvent(const qreal &xT,
                                const qreal &yT,
                                const ulong &time_stamp,
                                const qreal &pressure,
                                const bool &erase,
                                Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));

    mMainHandler->tabletPressEvent(relPos.x(), relPos.y(),
                                   time_stamp, pressure,
                                   erase, brush);
    relPos *= 0.25;
    mTemporaryHandler->tabletPressEvent(relPos.x(), relPos.y(),
                                   time_stamp, pressure,
                                   erase, brush);
}

void PaintBox::mouseReleaseEvent() {
    mMainHandler->mouseReleaseEvent();
    mTemporaryHandler->mouseReleaseEvent();
}

void PaintBox::mousePressEvent(const qreal &xT,
                               const qreal &yT,
                               const ulong &timestamp,
                               const qreal &pressure,
                               Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));

    mMainHandler->mousePressEvent(relPos.x(), relPos.y(),
                                  timestamp, pressure,
                                  brush);
    relPos *= 0.25;
    mTemporaryHandler->mousePressEvent(relPos.x(), relPos.y(),
                                  timestamp, pressure,
                                  brush);
}

void PaintBox::mouseMoveEvent(const qreal &xT,
                              const qreal &yT,
                              const ulong &time_stamp,
                              const bool &erase) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));

    mMainHandler->mouseMoveEvent(relPos.x(), relPos.y(),
                                 time_stamp, erase);
    relPos *= 0.25;
    mTemporaryHandler->mouseMoveEvent(relPos.x(), relPos.y(),
                                 time_stamp, erase);
}

void PaintBox::paintPress(const qreal &xT,
                          const qreal &yT,
                          const ulong &timestamp,
                          const qreal &pressure,
                          Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));

    mMainHandler->paintPress(relPos.x(), relPos.y(),
                             timestamp, pressure,
                             brush);
    relPos *= 0.25;
    mTemporaryHandler->paintPress(relPos.x(), relPos.y(),
                             timestamp, pressure,
                             brush);
}

void PaintBoxRenderData::drawSk(SkCanvas *canvas) {
    SkPaint paint;
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    foreach(TileSkDrawer *tile, tileDrawers) {
        tile->drawSk(canvas, &paint);
    }
}

void PaintBoxRenderData::updateRelBoundingRect() {
    int widthT = 0;
    int heightT = 0;
    foreach(TileSkDrawer *tile, tileDrawers) {
        widthT = qMax(widthT, tile->posX + TILE_DIM);
        heightT = qMax(heightT, tile->posY + TILE_DIM);
    }
    relBoundingRect = QRectF(0., 0.,
                             widthT, heightT);
}
