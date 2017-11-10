#include "bone.h"
#include "pointhelpers.h"
#include "canvas.h"

Bone::Bone() : BoundingBox(TYPE_BONE) {
    mRootPt = (new NonAnimatedMovablePoint(this, TYPE_BONE_POINT));
    mEndPt = (new NonAnimatedMovablePoint(this, TYPE_BONE_POINT));
}

void Bone::drawPixmapSk(SkCanvas *canvas) {
    SkPath path;
    qreal len = pointToLen(mRootPt->getRelativePos() -
                           mEndPt->getRelativePos());
    qreal width = len*0.25;

    SkPoint rootPos = QPointFToSkPoint(mRootPt->getRelativePos());
    SkPoint endPos = QPointFToSkPoint(mEndPt->getRelativePos());


    path.moveTo(rootPos);
    path.lineTo(endPos);
    path.transform(QMatrixToSkMatrix(getCombinedTransform()));
    SkPaint paintT;
    paintT.setColor(SK_ColorRED);
    paintT.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, paintT);

    paintT.setColor(SK_ColorBLACK);
    paintT.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paintT);
}

void Bone::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    Q_UNUSED(paint);
    drawPixmapSk(canvas);
}

void Bone::drawSelectedSk(SkCanvas *canvas,
                          const CanvasMode &currentCanvasMode,
                          const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mEndPt->drawSk(canvas, invScale);
            mRootPt->drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

MovablePoint *Bone::getPointAtAbsPos(
                                const QPointF &absPtPos,
                                const CanvasMode &currentCanvasMode,
                                const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = BoundingBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(pointToReturn == NULL) {
        if(mEndPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mEndPt;
        }
        if(mRootPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mRootPt;
        }
    }
    return pointToReturn;
}
