#include "paintbox.h"
#include "Paint/PaintLib/surface.h"
#include "Paint/PaintLib/animatedsurface.h"
#include "canvas.h"
#include "Animators/animatorupdater.h"
#include "pointanimator.h"

PaintBox::PaintBox() :
    BoundingBox(TYPE_PAINT) {
    setName("Paint Box");
    mTopLeftPoint = new PointAnimator(mTransformAnimator.data(),
                                      TYPE_PATH_POINT);
    mBottomRightPoint = new PointAnimator(mTransformAnimator.data(),
                                          TYPE_PATH_POINT);

    mTopLeftPoint->prp_setUpdater(
                new PaintBoxSizeUpdaterTL(this));
    mTopLeftPoint->prp_setName("top left");
    mBottomRightPoint->prp_setUpdater(
                new PaintBoxSizeUpdaterBR(this));
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

void PaintBox::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);

    if(mMainHandler == NULL) return;
    mMainHandler->setCurrentRelFrame(frame);
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

QRectF PaintBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    return QRectF(mTopLeftPoint->getRelativePosAtRelFrame(relFrame),
                  mBottomRightPoint->getRelativePosAtRelFrame(relFrame));
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

void PaintBox::newPaintFrameOnCurrentFrame() {
    mMainHandler->anim_saveCurrentValueAsKey();
    scheduleUpdate();
}

void PaintBox::newEmptyPaintFrameOnCurrentFrame() {
    mMainHandler->newEmptyPaintFrame();
    scheduleUpdate();
}

void PaintBox::newEmptyPaintFrameAtFrame(const int &relFrame) {
    mMainHandler->newEmptyPaintFrame(relFrame);
    scheduleUpdate();
}

void PaintBox::setOverlapFrames(const int &overlapFrames) {
    mOverlapFrames = overlapFrames;
    mMainHandler->setOverlapFrames(overlapFrames);
}

MovablePoint *PaintBox::getTopLeftPoint() {
    return mTopLeftPoint;
}

MovablePoint *PaintBox::getBottomRightPoint() {
    return mBottomRightPoint;
}

void PaintBox::finishSizeSetup() {
    QPointF tL = mTopLeftPoint->getCurrentPointValue();
    QPointF bR = mBottomRightPoint->getCurrentPointValue();
    QPointF sizeT = bR - tL;
    if(sizeT.x() < 1. || sizeT.y() < 1.) return;
    ushort widthT = sizeT.x();
    ushort heightT = sizeT.y();
    if(widthT == mWidth && heightT == mHeight) return;
    mWidth = widthT;
    mHeight = heightT;
    if(mMainHandler == NULL) {
        mMainHandler = new AnimatedSurface(mWidth, mHeight, 1., this);
        mMainHandler->setCurrentRelFrame(anim_mCurrentRelFrame);
        ca_addChildAnimator(mMainHandler);
    } else {
        mMainHandler->setSize(mWidth, mHeight);
    }
    if(mTemporaryHandler == NULL) {
        mTemporaryHandler = new Surface(mWidth/4, mHeight/4, 0.25, false);
    } else {
        mTemporaryHandler->setSize(mWidth/4, mHeight/4);
    }
}

void PaintBox::finishSizeAndPosSetup() {
    QPointF trans = mTopLeftPoint->getCurrentPointValue() -
            mTopLeftPoint->getSavedPointValue();
    int dX = -qRound(trans.x());
    int dY = -qRound(trans.y());
    if(dX <= 0 && dY <= 0) {
        mMainHandler->move(dX, dY);
    } else if(dX < 0) {
        mMainHandler->move(dX, 0);
    } else if(dY < 0) {
        mMainHandler->move(0, dY);
    }
    finishSizeSetup();
    if(dX >= 0 && dY >= 0) {
        mMainHandler->move(dX, dY);
    } else if(dX > 0) {
        mMainHandler->move(dX, 0);
    } else if(dY > 0) {
        mMainHandler->move(0, dY);
    }
}

void PaintBox::scheduleFinishSizeAndPosSetup() {
    mFinishSizeAndPosSetupScheduled = true;
}

void PaintBox::loadFromImage(const QImage &img) {
    if(!prp_hasKeys()) {
        mBottomRightPoint->setRelativePos(mTopLeftPoint->getRelativePos() +
                                          QPointF(img.width(), img.height()));
        finishSizeSetup();
    }
    mMainHandler->loadFromImage(img);
    scheduleUpdate();
}

void PaintBox::scheduleFinishSizeSetup() {
    mFinishSizeSetupScheduled = true;
}

void PaintBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    canvas->saveLayer(NULL, paint);
    BoundingBox::drawPixmapSk(canvas, NULL);
    if(mTemporaryHandler != NULL) {
        canvas->concat(
                QMatrixToSkMatrix(
                    mTransformAnimator->getCombinedTransform()) );
        QPointF trans = mTopLeftPoint->getRelativePosAtRelFrame(
                    anim_mCurrentRelFrame);
        canvas->translate(trans.x(), trans.y());
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
    if(mFinishSizeAndPosSetupScheduled) {
        mFinishSizeAndPosSetupScheduled = false;
        finishSizeAndPosSetup();
    } else if(mFinishSizeSetupScheduled) {
        mFinishSizeSetupScheduled = false;
        finishSizeSetup();
    }
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    PaintBoxRenderData *paintData = (PaintBoxRenderData*)data;
    if(mMainHandler == NULL) return;
    mMainHandler->getTileDrawers(&paintData->tileDrawers);
    foreach(TileSkDrawer *drawer, paintData->tileDrawers) {
        if(!drawer->finished()) {
            drawer->addDependent(paintData);
        }
    }
    QPointF topLeft;
    if(mTopLeftPoint->getBeingTransformed()) {
        topLeft = mTopLeftPoint->getSavedPointValue();
    } else {
        topLeft = mTopLeftPoint->getRelativePosAtRelFrame(relFrame);
    }
    paintData->trans = QPointFToSkPoint(topLeft);
}

BoundingBoxRenderData *PaintBox::createRenderData() {
    return new PaintBoxRenderData(this);
}

bool PaintBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                               const int &relFrame2) {
    if(ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1, relFrame2)) {
        return true;
    }
    if(mMainHandler != NULL) {
        if(mMainHandler->prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2)) {
            return true;
        }
    }
    return false;
}

void PaintBox::tabletEvent(const qreal &xT,
                           const qreal &yT,
                           const ulong &time_stamp,
                           const qreal &pressure,
                           const bool &erase,
                           Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT)) -
            mTopLeftPoint->getRelativePosAtRelFrame(
                        anim_mCurrentRelFrame);
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
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT)) -
            mTopLeftPoint->getRelativePosAtRelFrame(
                        anim_mCurrentRelFrame);

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
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT)) -
            mTopLeftPoint->getRelativePosAtRelFrame(
                        anim_mCurrentRelFrame);

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
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT)) -
            mTopLeftPoint->getRelativePosAtRelFrame(
                        anim_mCurrentRelFrame);
    int seedT = rand() % 1000;
    srand(seedT);
    mMainHandler->mouseMoveEvent(relPos.x(), relPos.y(),
                                 time_stamp, erase, brush);
    srand(seedT);
    mTemporaryHandler->mouseMoveEvent(relPos.x(), relPos.y(),
                                 time_stamp, erase, brush);
    scheduleUpdate();
}

void PaintBox::paintPress(const qreal &xT,
                          const qreal &yT,
                          const ulong &timestamp,
                          const qreal &pressure,
                          Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT)) -
            mTopLeftPoint->getRelativePosAtRelFrame(
                        anim_mCurrentRelFrame);

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
    canvas->translate(trans.x(), trans.y());
    foreach(TileSkDrawer *tile, tileDrawers) {
        tile->drawSk(canvas, &paint);
    }
}
