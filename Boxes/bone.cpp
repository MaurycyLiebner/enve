#include "bone.h"
#include "pointhelpers.h"
#include "canvas.h"

BonesBox::BonesBox() : BoundingBox(TYPE_BONES_BOX) {
    mMainBone = new Bone(getTransformAnimator());
    prp_setName("BonesBox");
}

bool BonesBox::relPointInsidePath(const QPointF &relPos) {
    return getCurrentPath().contains(relPos.x(), relPos.y());
    if(mSkRelBoundingRectPath.contains(relPos.x(), relPos.y()) ) {
        return getCurrentPath().contains(relPos.x(), relPos.y());
    } else {
        return false;
    }
}

void BonesBox::drawPixmapSk(SkCanvas *canvas) {
//    SkPath path = getCurrentPath();
//    path.transform(QMatrixToSkMatrix(getCombinedTransform()));
//    SkPaint paintT;
//    paintT.setColor(SK_ColorRED);
//    paintT.setStyle(SkPaint::kFill_Style);
//    canvas->drawPath(path, paintT);

//    paintT.setColor(SK_ColorBLACK);
//    paintT.setStyle(SkPaint::kStroke_Style);
//    paintT.setAntiAlias(true);
//    canvas->drawPath(path, paintT);
}

void BonesBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    Q_UNUSED(paint);
    drawPixmapSk(canvas);
}

void BonesBox::drawSelectedSk(SkCanvas *canvas,
                          const CanvasMode &currentCanvasMode,
                          const SkScalar &invScale) {
//    if(isVisibleAndInVisibleDurationRect()) {
//        canvas->save();
//        drawBoundingRectSk(canvas, invScale);
//        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
//            mEndPt->drawSk(canvas, invScale);
//            mRootPt->drawSk(canvas, invScale);
//        }
//        canvas->restore();
//    }
}

MovablePoint *BonesBox::getPointAtAbsPos(
                                const QPointF &absPtPos,
                                const CanvasMode &currentCanvasMode,
                                const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = BoundingBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
//    if(pointToReturn == NULL) {
//        if(mEndPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
//            return mEndPt;
//        }
//        if(mRootPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
//            return mRootPt;
//        }
//    }
    return pointToReturn;
}

SkPath BonesBox::getCurrentPath() {
    SkPath path;
//    QPointF rootPos;
//    if(mRootPt == mThisRootPt) {
//        rootPos = mRootPt->getRelativePos();
//    } else {
//        rootPos = mapAbsPosToRel(mRootPt->getAbsolutePos());
//    }
//    QPointF tipPos = mEndPt->getRelativePos();
//    qreal len = pointToLen(rootPos - tipPos);
//    qreal width = len*0.1;
//    qreal angle = QLineF(tipPos, rootPos).angleTo(QLineF(QPointF(0., 0.),
//                                                         QPointF(0., 10.)));


//    path.moveTo(0.f, 0.f);
//    path.lineTo(width, -width);
//    path.lineTo(0.f, -len);
//    path.lineTo(-width, -width);
//    path.close();
//    SkMatrix matr;
//    matr.setRotate(angle);
//    path.transform(matr);
//    matr.reset();
//    matr.setTranslate(rootPos.x(), rootPos.y());
//    path.transform(matr);
    return path;
}

