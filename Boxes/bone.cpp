#include "bone.h"
#include "pointhelpers.h"
#include "canvas.h"

BonesBox::BonesBox() : BoundingBox(TYPE_BONES_BOX) {
    mMainBone = new Bone(getTransformAnimator());
    prp_setName("BonesBox");
}

bool BonesBox::relPointInsidePath(const QPointF &relPos) {
    return mMainBone->getBoneAtRelPos(relPos) != NULL;
}

void BonesBox::drawPixmapSk(SkCanvas *canvas) {
    mMainBone->drawOnCanvas(canvas);
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

Bone::Bone(BasicTransformAnimator *parentAnimator, Bone *parentBone) {
    mTransformAnimator = new BoneTransformAnimator(this);
    mTransformAnimator->setParentTransformAnimator(parentAnimator);
    mParentTransformAnimator = parentAnimator;
    mParentBone = parentBone;
    if(parentBone == NULL) {
        mRootPt = new BonePt(mTransformAnimator, TYPE_BONE_POINT);
    } else {
        mRootPt = parentBone->getTipPt();
    }
    mRootPt->addRootBone(this);
    mTipPt = new BonePt(mTransformAnimator, TYPE_BONE_POINT);
    mTipPt->setTipBone(this);
}

Bone *Bone::getBoneAtRelPos(const QPointF &relPos) {
    foreach(Bone *boneT, mChildBones) {
        Bone *boneTT = boneT->getBoneAtRelPos(relPos);
        if(boneTT != NULL) return boneTT;
    }
    if(QRectF(mRelRootPos, mRelTipPos).contains(relPos)) {
        if(getCurrentRelPath().contains(relPos.x(), relPos.y())) {
            return this;
        }
    }
    return NULL;
}

SkPath Bone::getCurrentRelPath() {
    SkPath path;
    qreal len = pointToLen(mRelRootPos - mRelTipPos);
    qreal width = len*0.1;
    qreal angle = QLineF(mRelTipPos, mRelRootPos).angleTo(QLineF(QPointF(0., 0.),
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
    matr.setTranslate(mRelRootPos.x(), mRelRootPos.y());
    path.transform(matr);
    return path;
}

void Bone::drawOnCanvas(SkCanvas *canvas) {
    SkPath path = getCurrentRelPath();
    path.transform(QMatrixToSkMatrix(
                       mTransformAnimator->getCombinedTransform()));
    SkPaint paintT;
    paintT.setColor(SK_ColorWHITE);
    paintT.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, paintT);

    paintT.setColor(SK_ColorBLACK);
    paintT.setStyle(SkPaint::kStroke_Style);
    paintT.setAntiAlias(true);
    canvas->drawPath(path, paintT);
    foreach(Bone *boneT, mChildBones) {
        boneT->drawOnCanvas(canvas);
    }
}

const QPointF &Bone::getRootRelPos() { return mRelRootPos; }

const QPointF &Bone::getTipRelPos() { return mRelTipPos; }

void Bone::setRelRootPos(const QPointF &pos) {
    mRelRootPos = pos;
    mTransformAnimator->prp_callUpdater();
}

void Bone::setRelTipPos(const QPointF &pos) {
    mRelTipPos = pos;
}

void Bone::setAbsRootPos(const QPointF &pos) {
    setRelRootPos(mTransformAnimator->mapAbsPosToRel(pos));
}

void Bone::setAbsTipPos(const QPointF &pos) {
    setRelTipPos(mTransformAnimator->mapAbsPosToRel(pos));
}

BasicTransformAnimator *Bone::getTransformAnimator() {
    return mTransformAnimator;
}

const bool &Bone::isConnectedToParent() {
    return mConnectedToParent && mParentBone != NULL;
}

void Bone::setConnectedToParent(const bool &bT) {
    if(bT == mConnectedToParent) return;
    mConnectedToParent = bT;
    if(mParentBone == NULL) return;
    if(bT) {
        delete mRootPt;
        mRootPt = mParentBone->getTipPt();
    } else {
        mRootPt = new BonePt(mTransformAnimator, TYPE_BONE_POINT);
    }
    mRootPt->addRootBone(this);
}

Bone *Bone::getParentBone() {
    return mParentBone;
}

BonePt *Bone::getTipPt() {
    return mTipPt;
}

BonePt *Bone::getRootPt() {
    return mRootPt;
}
