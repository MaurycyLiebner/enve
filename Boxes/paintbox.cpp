#include "paintbox.h"
#include "Paint/layer.h"
#include "canvas.h"
#include "Animators/animatorupdater.h"

PaintBox::PaintBox() :
    BoundingBox(TYPE_PAINT) {
    setName("Paint Box");
    mTopLeftPoint = new MovablePoint(this, TYPE_PATH_POINT);
    mBottomRightPoint = new MovablePoint(this, TYPE_PATH_POINT);

    mTopLeftPoint->prp_setUpdater(
                new PaintBoxSizeUpdater(this));
    mTopLeftPoint->prp_setName("top left");
    mBottomRightPoint->prp_setUpdater(
                new PaintBoxSizeUpdater(this));
    mBottomRightPoint->prp_setName("bottom right");
}

PaintBox::PaintBox(const ushort &canvasWidthT,
                   const ushort &canvasHeightT) :
    PaintBox() {
    mBottomRightPoint->setCurrentPointValue(QPointF(canvasWidthT,
                                                    canvasHeightT),
                                            false, true);
    finishSizeSetup();
}

MovablePoint *PaintBox::getPointAtAbsPos(const QPointF &absPtPos,
                                      const CanvasMode &currentCanvasMode,
                                      const qreal &canvasScaleInv) {
    if(currentCanvasMode == MOVE_POINT) {
        if(mTopLeftPoint->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return mTopLeftPoint;
        }
        if(mBottomRightPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mBottomRightPoint;
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        MovablePoint *pivotMovable = mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return pivotMovable;
        }
    }
    return NULL;
}

void PaintBox::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                  QList<MovablePoint *> *list) {
    if(!mTopLeftPoint->isSelected()) {
        if(mTopLeftPoint->isContainedInRect(absRect)) {
            mTopLeftPoint->select();
            list->append(mTopLeftPoint);
        }
    }
    if(!mBottomRightPoint->isSelected()) {
        if(mBottomRightPoint->isContainedInRect(absRect)) {
            mBottomRightPoint->select();
            list->append(mBottomRightPoint);
        }
    }
}

void PaintBox::drawSelectedSk(SkCanvas *canvas,
                              const CanvasMode &currentCanvasMode,
                              const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mTopLeftPoint->drawSk(canvas, invScale);
            mBottomRightPoint->drawSk(canvas, invScale);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

void PaintBox::startAllPointsTransform() {
    mBottomRightPoint->startTransform();
    mTopLeftPoint->startTransform();
    startTransform();
}

MovablePoint *PaintBox::getBottomRightPoint() {
    return mBottomRightPoint;
}

void PaintBox::finishSizeSetup() {
    QPointF bR = mBottomRightPoint->getCurrentPointValue();
    if(bR.x() < 1. || bR.y() < 1.) return;
    ushort widthT = bR.x();
    ushort heightT = bR.y();
    if(widthT == mWidth && heightT == mHeight) return;
    mWidth = widthT;
    mHeight = heightT;
    if(mMainHandler == NULL) {
        mMainHandler = new CanvasHandler(mWidth, mHeight,
                                         1., true);
    } else {
        mMainHandler->setSize(mWidth, mHeight);
    }
    if(mTemporaryHandler == NULL) {
        mTemporaryHandler = new CanvasHandler(mWidth/4, mHeight/4,
                                              0.25, false);
    } else {
        mTemporaryHandler->setSize(mWidth/4, mHeight/4);
    }
}

void PaintBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    canvas->saveLayer(NULL, paint);
    BoundingBox::drawPixmapSk(canvas, NULL);
    if(mTemporaryHandler != NULL) {
        canvas->concat(
                QMatrixToSkMatrix(
                    mTransformAnimator->getCombinedTransform()) );
        mTemporaryHandler->drawSk(canvas, paint);
    }

    canvas->restore();
}

void PaintBox::processSchedulers() {
    if(mTemporaryHandler != NULL) {
        mTemporaryHandler->saveToTmp();
    }
    BoundingBox::processSchedulers();
}

void PaintBox::renderDataFinished(BoundingBoxRenderData *renderData) {
    BoundingBox::renderDataFinished(renderData);
    if(mTemporaryHandler != NULL) {
        mTemporaryHandler->clearTmp();
    }
}

void PaintBox::setupBoundingBoxRenderDataForRelFrame(
        const int &relFrame, BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    PaintBoxRenderData *paintData = (PaintBoxRenderData*)data;
    if(mMainHandler == NULL) return;
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
                           const bool &erase,
                           Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));
    int seedT = rand() % 1000;
    srand(seedT);
    mMainHandler->tabletEvent(relPos.x(), relPos.y(),
                              time_stamp, pressure,
                              erase, brush);
    srand(seedT);
    mTemporaryHandler->tabletEvent(relPos.x(), relPos.y(),
                              time_stamp, pressure,
                              erase, brush);
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
    mTemporaryHandler->mousePressEvent(relPos.x(), relPos.y(),
                                  timestamp, pressure,
                                  brush);
}

void PaintBox::mouseMoveEvent(const qreal &xT,
                              const qreal &yT,
                              const ulong &time_stamp,
                              const bool &erase,
                              Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT));
    int seedT = rand() % 1000;
    srand(seedT);
    mMainHandler->mouseMoveEvent(relPos.x(), relPos.y(),
                                 time_stamp, erase, brush);
    srand(seedT);
    mTemporaryHandler->mouseMoveEvent(relPos.x(), relPos.y(),
                                 time_stamp, erase, brush);
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
