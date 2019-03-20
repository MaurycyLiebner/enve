#include "smartvectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "pointhelpers.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "MovablePoints/nodepoint.h"
#include "Animators/pathanimator.h"
#include "Animators/PathAnimators/smartpathcollectionhandler.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Animators/gradientpoints.h"
#include "Animators/effectanimators.h"
#include "Animators/transformanimator.h"
#include "MovablePoints/segment.h"

SmartVectorPath::SmartVectorPath() :
    PathBox(BoundingBoxType::TYPE_VECTOR_PATH),
    mHandler(mTransformAnimator.get(), this) {
    setName("Path");
    mPathAnimator = mHandler.getAnimator();
    ca_addChildAnimator(GetAsSPtr(mPathAnimator, Property));
    ca_moveChildBelow(mPathAnimator.data(), mEffectsAnimators.data());
}

bool SmartVectorPath::differenceInEditPathBetweenFrames(
        const int &frame1, const int &frame2) const {
    return mPathAnimator->prp_differencesBetweenRelFrames(frame1, frame2);
}

void SmartVectorPath::selectAllPoints(Canvas * const canvas) {
    mHandler.selectAllPoints(canvas);
}

void SmartVectorPath::applyCurrentTransformation() {
    mNReasonsNotToApplyUglyTransform++;
//    mPathAnimator->applyTransformToPoints(
//                mTransformAnimator->getCurrentTransformationMatrix());

    mTransformAnimator->reset();
    centerPivotPosition();
    mNReasonsNotToApplyUglyTransform--;
}

NormalSegment SmartVectorPath::getNormalSegment(
        const QPointF &absPos, const qreal &canvasScaleInv) {
    return mHandler.getNormalSegmentAtAbsPos(absPos, canvasScaleInv);
}

void SmartVectorPath::drawSelectedSk(SkCanvas *canvas,
                              const CanvasMode &currentCanvasMode,
                              const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        mHandler.drawPoints(canvas,
                            currentCanvasMode,
                            invScale,
                            toSkMatrix(getCombinedTransform()));
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mFillGradientPoints->drawGradientPointsSk(canvas, invScale);
            mStrokeGradientPoints->drawGradientPointsSk(canvas, invScale);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

MovablePoint *SmartVectorPath::getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv) {
    MovablePoint* pointToReturn = PathBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(!pointToReturn) {
        pointToReturn = mHandler.getPointAtAbsPos(absPtPos,
                                                  currentCanvasMode,
                                                  canvasScaleInv);
    }
    return pointToReturn;
}

void SmartVectorPath::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                   QList<stdptr<MovablePoint>>& list) {
    mHandler.selectAndAddContainedPointsToList(absRect, list);
}

SkPath SmartVectorPath::getPathAtRelFrameF(const qreal &relFrame) {
     return mHandler.getPathAtRelFrame(relFrame);
}

void SmartVectorPath::getMotionBlurProperties(QList<Property*> &list) const {
    PathBox::getMotionBlurProperties(list);
    list.append(mPathAnimator);
}
