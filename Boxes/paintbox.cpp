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

bool PaintBox::prp_nextRelFrameWithKey(const int &relFrame,
                                       int &nextRelFrame) {
    int bbNext;
    bool bbHasNext = BoundingBox::prp_nextRelFrameWithKey(relFrame,
                                                          bbNext);
    if(mMainHandler != nullptr) {
        if(mMainHandler->prp_hasKeys()) {
            int clostestFrame;
            mMainHandler->anim_getClosestsKeyOccupiedRelFrame(relFrame,
                                                              clostestFrame);
            int paintMin = clostestFrame +
                    (relFrame - clostestFrame)/mFrameStep*mFrameStep;
            if(bbNext) paintMin = qMin(paintMin, bbNext);
            if(paintMin <= relFrame) paintMin += mFrameStep;
            nextRelFrame = paintMin;
            return true;
        }
    }
    nextRelFrame = bbNext;
    return bbHasNext;
}

bool PaintBox::prp_prevRelFrameWithKey(const int &relFrame,
                                       int &prevRelFrame) {
    int bbPrev;
    bool bbHasPrev = BoundingBox::prp_prevRelFrameWithKey(relFrame,
                                                          bbPrev);
    if(mMainHandler != nullptr) {
        if(mMainHandler->prp_hasKeys()) {
            int clostestFrame;
            mMainHandler->anim_getClosestsKeyOccupiedRelFrame(relFrame,
                                                              clostestFrame);
            int paintMin = clostestFrame +
                    (relFrame - clostestFrame)/mFrameStep*mFrameStep;
            if(bbHasPrev) paintMin = qMax(paintMin, bbPrev);
            if(paintMin >= relFrame) paintMin -= mFrameStep;
            prevRelFrame = paintMin;
            return true;
        }
    }
    prevRelFrame = bbPrev;
    return bbHasPrev;
}

const bool &PaintBox::isDraft() const {
    return mMainHandler->isDraft();
}

void PaintBox::setIsDraft(const bool &bT) {
    mMainHandler->setIsDraft(bT);
}

bool PaintBox::isSurfaceAnimated() {
    return mMainHandler->prp_hasKeys();
}

void PaintBox::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);

    if(mMainHandler == nullptr) return;
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
    return nullptr;
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
    if(mMainHandler == nullptr) {
        mMainHandler = new AnimatedSurface(mWidth, mHeight, 1., this);
        mMainHandler->setCurrentRelFrame(anim_mCurrentRelFrame);
        ca_addChildAnimator(mMainHandler);
    } else {
        mMainHandler->setSize(mWidth, mHeight);
    }
    if(mTemporaryHandler == nullptr) {
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
    if(dX < 0 && dY < 0) {
        mMainHandler->move(dX, dY);
    } else if(dX < 0) {
        mMainHandler->move(dX, 0);
    } else if(dY < 0) {
        mMainHandler->move(0, dY);
    }
    finishSizeSetup();
    if(dX > 0 && dY > 0) {
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
    canvas->saveLayer(nullptr, paint);
    BoundingBox::drawPixmapSk(canvas, nullptr);
    if(mTemporaryHandler != nullptr) {
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
    if(mTemporaryHandler != nullptr) {
        mTemporaryHandler->saveToTmp();
    }
    BoundingBox::processSchedulers();
}

void PaintBox::renderDataFinished(const std::shared_ptr<BoundingBoxRenderData>& renderData) {
    BoundingBox::renderDataFinished(renderData);
    if(mTemporaryHandler != nullptr) {
        mTemporaryHandler->clearTmp();
    }
}

void PaintBox::setupBoundingBoxRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& data) {
    if(mFinishSizeAndPosSetupScheduled) {
        mFinishSizeAndPosSetupScheduled = false;
        finishSizeAndPosSetup();
    } else if(mFinishSizeSetupScheduled) {
        mFinishSizeSetupScheduled = false;
        finishSizeSetup();
    }
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
    auto paintData = data->ref<PaintBoxRenderData>();
    if(mMainHandler == nullptr) return;
    mMainHandler->getTileDrawers(&paintData->tileDrawers);
    foreach(const TileSkDrawerCollection &drawer, paintData->tileDrawers) {
        foreach(TileSkDrawer *drawerT, drawer.drawers) {
            drawerT->addDependent(paintData);
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

std::shared_ptr<BoundingBoxRenderData> PaintBox::createRenderData() {
    return (new PaintBoxRenderData(this))->ref<BoundingBoxRenderData>();
}

bool PaintBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                               const int &relFrame2) {
    if(ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1, relFrame2)) {
        return true;
    }
    if(mMainHandler != nullptr) {
        if(mMainHandler->prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2)) {
            return true;
        }
    }
    return false;
}

void PaintBox::tabletMoveEvent(const qreal &xT,
                           const qreal &yT,
                           const ulong &time_stamp,
                           const qreal &pressure,
                           const bool &erase,
                           const Brush *brush) {
    QPointF relPos = mapAbsPosToRel(QPointF(xT, yT)) -
            mTopLeftPoint->getRelativePosAtRelFrame(
                        anim_mCurrentRelFrame);
    int seedT = rand() % 1000;
    srand(seedT);
    mMainHandler->tabletMoveEvent(relPos.x(), relPos.y(),
                              time_stamp, pressure,
                              erase, brush);
    qreal pRed, pGreen, pBlue, pAlpha;
    mMainHandler->getPickedUpRGBA(&pRed, &pGreen,
                                  &pBlue, &pAlpha);
    mTemporaryHandler->setPickedUpRGBA(pRed, pGreen,
                                       pBlue, pAlpha);
    srand(seedT);
    mTemporaryHandler->tabletMoveEvent(relPos.x(), relPos.y(),
                              time_stamp, pressure,
                              erase, brush);
}

void PaintBox::mouseMoveEvent(const qreal &xT,
                              const qreal &yT,
                              const ulong &time_stamp,
                              const bool &erase,
                              const Brush *brush) {
    tabletMoveEvent(xT, yT,
                    time_stamp, 1.0,
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
                                const Brush *brush) {
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

void PaintBox::mousePressEvent(const qreal &xT,
                               const qreal &yT,
                               const ulong &timestamp,
                               const qreal &pressure,
                               const Brush *brush) {
    tabletPressEvent(xT, yT, timestamp, pressure, false, brush);
}

void PaintBox::mouseReleaseEvent() {
    tabletReleaseEvent();
}

void PaintBoxRenderData::drawSk(SkCanvas *canvas) {
    //SkPaint paint;
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    canvas->translate(trans.x(), trans.y());
    foreach(const TileSkDrawerCollection &tile, tileDrawers) {
        tile.drawSk(canvas);
    }
}
