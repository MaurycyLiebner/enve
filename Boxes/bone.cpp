#include "bone.h"
#include "pointhelpers.h"
#include "canvas.h"

Bone::Bone() : BoundingBox(TYPE_BONE) {
    prp_setName("Bone");
    mThisRootPt = (new BoneTipPoint(this, NULL, this));
    mEndPt = (new BoneTipPoint(NULL, this, this));
    mThisRootPt->setRelativePos(QPointF(0., 0.));
    mEndPt->setRelativePos(QPointF(0., -10.));
    mRootPt = mThisRootPt;
    mTransformAnimator->getPivotAnimator()->SWT_hide();
    mTransformAnimator->getOpacityAnimator()->SWT_hide();
}

bool Bone::relPointInsidePath(const QPointF &relPos) {
    return getCurrentPath().contains(relPos.x(), relPos.y());
    if(mSkRelBoundingRectPath.contains(relPos.x(), relPos.y()) ) {
        return getCurrentPath().contains(relPos.x(), relPos.y());
    } else {
        return false;
    }
}

void Bone::drawPixmapSk(SkCanvas *canvas) {
    SkPath path = getCurrentPath();
    path.transform(QMatrixToSkMatrix(getCombinedTransform()));
    SkPaint paintT;
    paintT.setColor(SK_ColorRED);
    paintT.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, paintT);

    paintT.setColor(SK_ColorBLACK);
    paintT.setStyle(SkPaint::kStroke_Style);
    paintT.setAntiAlias(true);
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

void Bone::tipMovedByRel(const QPointF &trans,
                         const QPointF &savedRelPos) {
    mEndPt->setRelativePos(savedRelPos + trans);
}

void Bone::tipMovedByAbs(const QPointF &trans,
                         const QPointF &savedRelPos) {
    tipMovedByRel(mSavedCombinedTransform.inverted().map(
                      mSavedCombinedTransform.map(savedRelPos) +
              trans) - savedRelPos, savedRelPos);
}

void Bone::rootMovedByRel(const QPointF &trans,
                         const QPointF &savedRelPos) {
    mRootPt->setRelativePos(savedRelPos + trans);
    mTransformAnimator->getPivotMovablePoint()->setRelativePos(
                savedRelPos + trans);
}

void Bone::rootMovedByAbs(const QPointF &trans,
                         const QPointF &savedRelPos) {
    rootMovedByRel(mSavedCombinedTransform.inverted().map(
                      mSavedCombinedTransform.map(savedRelPos) +
              trans) - savedRelPos, savedRelPos);
}


SkPath Bone::getCurrentPath() {
    SkPath path;
    QPointF rootPos;
    if(mRootPt == mThisRootPt) {
        rootPos = mRootPt->getRelativePos();
    } else {
        rootPos = mapAbsPosToRel(mRootPt->getAbsolutePos());
    }
    QPointF tipPos = mEndPt->getRelativePos();
    qreal len = pointToLen(rootPos - tipPos);
    qreal width = len*0.1;
    qreal angle = QLineF(tipPos, rootPos).angleTo(QLineF(QPointF(0., 0.),
                                                         QPointF(0., 10.)));


    path.moveTo(0.f, 0.f);
    path.lineTo(width, -width);
    path.lineTo(0.f, -len);
    path.lineTo(-width, -width);
    path.close();
    SkMatrix matr;
    matr.setRotate(angle);
    path.transform(matr);
    matr.reset();
    matr.setTranslate(rootPos.x(), rootPos.y());
    path.transform(matr);
    return path;
}

BoneTipPoint::BoneTipPoint(Bone *rootBone,
                           Bone *tipBone, Bone *parent) :
    NonAnimatedMovablePoint(parent,
                            TYPE_BONE_POINT){
    mRootBone = rootBone;
    mTipBone = tipBone;
}

void BoneTipPoint::moveByRel(const QPointF &relTranslatione) {
    if(mTipBone != NULL) {
        mTipBone->tipMovedByRel(relTranslatione,
                                mSavedRelPos);
    }
    if(mRootBone != NULL) {
        mRootBone->rootMovedByRel(relTranslatione,
                                  mSavedRelPos);
    }
}

void BoneTipPoint::moveByAbs(const QPointF &absTranslatione) {
    if(mTipBone != NULL) {
        mTipBone->tipMovedByAbs(absTranslatione,
                                mSavedRelPos);
    }
    if(mRootBone != NULL) {
        mRootBone->rootMovedByAbs(absTranslatione,
                                  mSavedRelPos);
    }
}

void BoneTipPoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(mTipBone != NULL) {
        mTipBone->saveCombinedTransform();
        mTipBone->startRotTransform();
    }
    if(mRootBone != NULL) {
        mRootBone->saveCombinedTransform();
        mRootBone->startRotTransform();
    }
}

void BoneTipPoint::finishTransform() {
    NonAnimatedMovablePoint::finishTransform();
    if(mTipBone != NULL) {
        mTipBone->finishTransform();
    }
    if(mRootBone != NULL) {
        mRootBone->finishTransform();
    }
}

void BoneTipPoint::setTipBone(Bone *tipBone) {
    mParent = tipBone;
    mTipBone = tipBone;
}

void BoneTipPoint::setRootBone(Bone *rootBone) {
    mRootBone = rootBone;
}
