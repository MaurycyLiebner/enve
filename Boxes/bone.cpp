#include "bone.h"
#include "pointhelpers.h"
#include "canvas.h"

BonesBox::BonesBox() : BoundingBox(TYPE_BONES_BOX) {
    prp_setName("Armature");
}

void BonesBox::drawHoveredSk(SkCanvas *canvas,
                            const SkScalar &invScale) {
    foreach(Bone *bone, mBones) {
        bone->drawHoveredPathSk(canvas, invScale);
    }
}

bool BonesBox::relPointInsidePath(const QPointF &relPos) {
    foreach(Bone *bone, mBones) {
        if(bone->getBoneAtRelPos(relPos) != NULL) { return true; }
    }
    return false;
}

void BonesBox::drawPixmapSk(SkCanvas *canvas) {
    foreach(Bone *bone, mBones) {
        bone->drawOnCanvas(canvas);
    }
}

void BonesBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    Q_UNUSED(paint);
    drawPixmapSk(canvas);
}

void BonesBox::drawSelectedSk(SkCanvas *canvas,
                          const CanvasMode &currentCanvasMode,
                          const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        BoundingBox::drawSelectedSk(canvas, currentCanvasMode, invScale);
        foreach(Bone *bone, mBones) {
            bone->drawSelectedSk(canvas, currentCanvasMode, invScale);
        }
        canvas->restore();
    }
}

MovablePoint *BonesBox::getPointAtAbsPos(
                                const QPointF &absPtPos,
                                const CanvasMode &currentCanvasMode,
                                const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = BoundingBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(pointToReturn == NULL) {
        foreach(Bone *bone, mBones) {
            pointToReturn = bone->getPointAtAbsPos(absPtPos,
                                                   canvasScaleInv);
            if(pointToReturn == NULL) continue;
            return pointToReturn;
        }
    }
    return pointToReturn;
}

Bone::Bone(BonesBox *boneBox) {
    prp_setName("bone");
    mTransformAnimator =
            (new BoneTransformAnimator(this))->ref<BoneTransformAnimator>();
    setParentBonesBox(boneBox);
    mTipPt = new BonePt(mTransformAnimator.data(), TYPE_BONE_POINT);
    mTipPt->setTipBone(this);
    mTipPt->setRelativePos(mRelRootPos);
    ca_addChildAnimator(mTransformAnimator.data());
}

Bone::Bone(Bone *parentBone) {
    prp_setName("bone");
    mTransformAnimator =
            (new BoneTransformAnimator(this))->ref<BoneTransformAnimator>();
    setParentBone(parentBone);
    mTipPt = new BonePt(mTransformAnimator.data(), TYPE_BONE_POINT);
    mTipPt->setTipBone(this);
    mTipPt->setRelativePos(mRelRootPos);
    ca_addChildAnimator(mTransformAnimator.data());
}

Bone *Bone::getBoneAtRelPos(const QPointF &relPos) {
    QPointF realRelPos = mTransformAnimator->mapFromParent(relPos);
    foreach(Bone *boneT, mChildBones) {
        Bone *boneTT = boneT->getBoneAtRelPos(realRelPos);
        if(boneTT != NULL) return boneTT;
    }
    //if(QRectF(mRelRootPos, mRelTipPos).contains(realRelPos)) {
        if(getCurrentRelPath().contains(realRelPos.x(), realRelPos.y())) {
            return this;
        }
    //}
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
    if(!mConnectedToParent) {
        QPointF rootAbs = mRootPt->getAbsolutePos();
        QPointF parentTipAbs = mParentBone->getTipPt()->getAbsolutePos();
        SkPaint paintT;
        paintT.setColor(SK_ColorBLACK);
        paintT.setStyle(SkPaint::kStroke_Style);
        paintT.setAntiAlias(true);
        paintT.setStrokeWidth(1.f);
        SkScalar intervals[] = {10, 10};
        paintT.setPathEffect(
                    SkDashPathEffect::Make(intervals,
                                           SK_ARRAY_COUNT(intervals), 1));
        canvas->drawLine(QPointFToSkPoint(rootAbs),
                         QPointFToSkPoint(parentTipAbs),
                         paintT);
    }
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
    return mTransformAnimator.data();
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
        mRootPt = new BonePt(mTransformAnimator.data(), TYPE_BONE_POINT);
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

MovablePoint *Bone::getPointAtAbsPos(const QPointF &absPtPos,
                                     const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = NULL;
    foreach(Bone *boneT, mChildBones) {
        pointToReturn = boneT->getPointAtAbsPos(absPtPos, canvasScaleInv);
        if(pointToReturn == NULL) continue;
        return pointToReturn;
    }
    if(pointToReturn == NULL) {
        if(mTipPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mTipPt;
        }
        if(mRootPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mRootPt;
        }
    }
    return pointToReturn;
}

void Bone::drawSelectedSk(SkCanvas *canvas,
                          const CanvasMode &currentCanvasMode,
                          const SkScalar &invScale) {
    canvas->save();
    if(mSelected) {
        SkPath path = getCurrentRelPath();
        path.transform(QMatrixToSkMatrix(
                           mTransformAnimator->getCombinedTransform()));
        SkPaint paintT;

        paintT.setColor(SK_ColorRED);
        paintT.setStyle(SkPaint::kStroke_Style);
        paintT.setAntiAlias(true);
        canvas->drawPath(path, paintT);
    }

    if(currentCanvasMode == CanvasMode::MOVE_POINT ||
       currentCanvasMode == CanvasMode::ADD_BONE) {
        mRootPt->drawSk(canvas, invScale);
        mTipPt->drawSk(canvas, invScale);
    }
    foreach(Bone *boneT, mChildBones) {
        boneT->drawSelectedSk(canvas, currentCanvasMode, invScale);
    }
    canvas->restore();
}

void Bone::selectAndAddContainedPointsToList(const QRectF &absRect,
                                             QList<MovablePoint *> *list) {
    foreach(Bone *boneT, mChildBones) {
        boneT->selectAndAddContainedPointsToList(absRect, list);
    }
    if(mTipPt->isContainedInRect(absRect)) {
        list->append(mTipPt);
        mTipPt->select();
    }
    if(mRootPt->getParentBone() == this) {
        if(mRootPt->isContainedInRect(absRect)) {
            list->append(mRootPt);
            mRootPt->select();
        }
    }
}

void Bone::setParentBone(Bone *parentBone) {
    if(parentBone == mParentBone) return;
    if(mParentBone != NULL) {
        mParentBone->removeChildBone(this);
    }
    if(mParentBonesBox != NULL) {
        mParentBonesBox->removeBone(this);
        mParentBonesBox = NULL;
    }

    mTransformAnimator->setParentTransformAnimator(
                parentBone->getTransformAnimator());
    mRootPt = parentBone->getTipPt();
    parentBone->addChildBone(this);
    setAbsRootPos(mRootPt->getAbsolutePos());
    mRootPt->addRootBone(this);
    mParentBone = parentBone;
}

void Bone::setParentBonesBox(BonesBox *bonesBox) {
    if(mParentBone != NULL) {
        mParentBone->removeChildBone(this);
        mParentBone = NULL;
        mRootPt->removeRootBone(this);
        mRootPt = NULL;
    }
    if(mParentBonesBox != NULL) {
        mParentBonesBox->removeBone(this);
    }
    mTransformAnimator->setParentTransformAnimator(
                bonesBox->getTransformAnimator());
    if(mRootPt == NULL) {
        mRootPt = new BonePt(mTransformAnimator.data(), TYPE_BONE_POINT);
        mRootPt->addRootBone(this);
    }
    mParentBonesBox = bonesBox;
    mParentBonesBox->addBone(this);
}

void Bone::drawHoveredPathSk(SkCanvas *canvas, const qreal &invScale) {
    canvas->save();
    SkPath mappedPath = getCurrentRelPath();
    mappedPath.transform(QMatrixToSkMatrix(
                             mTransformAnimator->getCombinedTransform()));
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2.*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(mappedPath, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(invScale);
    canvas->drawPath(mappedPath, paint);
    canvas->restore();
    foreach(Bone *bone, mChildBones) {
        bone->drawHoveredPathSk(canvas, invScale);
    }
}
