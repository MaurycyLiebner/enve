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
        if(bone->getBoneAtRelPos(relPos) != nullptr) { return true; }
    }
    return false;
}

bool BonesBox::SWT_isBonesBox() { return true; }

void BonesBox::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                 QList<MovablePoint *> *list) {
    foreach(Bone *bone, mBones) {
            bone->selectAndAddContainedPointsToList(absRect, list);
        }
}

void BonesBox::addBone(Bone *bone) {
    mBones << bone;
        ca_addChildAnimator(bone);
}

void BonesBox::removeBone(Bone *bone) {
    mBones.removeOne(bone);
        ca_removeChildAnimator(bone);
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
    if(pointToReturn == nullptr) {
        foreach(Bone *bone, mBones) {
            pointToReturn = bone->getPointAtAbsPos(absPtPos,
                                                   canvasScaleInv);
            if(pointToReturn == nullptr) continue;
            return pointToReturn;
        }
    }
    return pointToReturn;
}

BoundingBox *BonesBox::createNewDuplicate() { return nullptr; }

Bone *BonesBox::getBoneAtAbsPos(const QPointF &absPos) {
    return getBoneAtRelPos(mapAbsPosToRel(absPos));
}

Bone *BonesBox::getBoneAtRelPos(const QPointF &relPos) {
    foreach(Bone *bone, mBones) {
        Bone *boneT = bone->getBoneAtRelPos(relPos);
        if(boneT != nullptr) { return boneT; }
    }
    return nullptr;
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

void Bone::deselect() {
    mSelected = false;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

void Bone::select() {
    mSelected = true;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

const bool &Bone::isSelected() { return mSelected; }

void Bone::startPosTransform() {
    mTransformAnimator->startPosTransform();
}

void Bone::startRotTransform() {
    mTransformAnimator->startRotTransform();
}

void Bone::startScaleTransform() {
    mTransformAnimator->startScaleTransform();
}

void Bone::startTransform() {
    mTransformAnimator->prp_startTransform();
}

void Bone::finishTransform() {
    mTransformAnimator->prp_finishTransform();
}

void Bone::moveByAbs(const QPointF &trans) {
    mTransformAnimator->moveByAbs(trans);
    //    QPointF by = mParent->mapAbsPosToRel(trans) -
    //                 mParent->mapAbsPosToRel(QPointF(0., 0.));
    // //    QPointF by = mapAbsPosToRel(
    // //                trans - mapRelativeToAbsolute(QPointF(0., 0.)));

    //    moveByRel(by);
}

void Bone::moveByRel(const QPointF &trans) {
    mTransformAnimator->moveRelativeToSavedValue(trans.x(), trans.y());
}

void Bone::setAbsolutePos(const QPointF &pos, const bool &saveUndoRedo) {
    setRelativePos(mParentBone->mapAbsPosToRel(pos), saveUndoRedo);
}

void Bone::setRelativePos(const QPointF &relPos, const bool &saveUndoRedo) {
    mTransformAnimator->setPosition(relPos.x(), relPos.y(), saveUndoRedo);
}

void Bone::saveTransformPivotAbsPos(const QPointF &absPivot) {
    if(mParentBone == nullptr) {
        mSavedTransformPivot =
                mParentBonesBox->mapAbsPosToRel(absPivot);
    } else {
        mSavedTransformPivot =
                mParentBone->mapAbsPosToRel(absPivot);
    }
}

QPointF Bone::mapAbsPosToRel(const QPointF &absPos) {
    return mTransformAnimator->mapAbsPosToRel(absPos);
}

QPointF Bone::mapRelPosToAbs(const QPointF &relPos) {
    return mTransformAnimator->mapRelPosToAbs(relPos);
}

void Bone::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void Bone::scale(const qreal &scaleXBy, const qreal &scaleYBy) {
    mTransformAnimator->scale(scaleXBy, scaleYBy);
}

void Bone::rotateBy(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot);
}

void Bone::rotateRelativeToSavedPivot(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot,
                                                   mSavedTransformPivot);
}

void Bone::scaleRelativeToSavedPivot(const qreal &scaleXBy, const qreal &scaleYBy) {
    mTransformAnimator->scaleRelativeToSavedValue(scaleXBy, scaleYBy,
                                                  mSavedTransformPivot);
}

Bone *Bone::getBoneAtRelPos(const QPointF &relPos) {
    QPointF realRelPos = mTransformAnimator->mapFromParent(relPos);
    foreach(Bone *boneT, mChildBones) {
        Bone *boneTT = boneT->getBoneAtRelPos(realRelPos);
        if(boneTT != nullptr) return boneTT;
    }
    //if(QRectF(mRelRootPos, mRelTipPos).contains(realRelPos)) {
    if(getCurrentRelPath().contains(realRelPos.x(), realRelPos.y())) {
            return this;
        }
    //}
    return nullptr;
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
    return mConnectedToParent && mParentBone != nullptr;
}

void Bone::setConnectedToParent(const bool &bT) {
    if(bT == mConnectedToParent) return;
    mConnectedToParent = bT;
    if(mParentBone == nullptr) return;
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
    MovablePoint *pointToReturn = nullptr;
    foreach(Bone *boneT, mChildBones) {
        pointToReturn = boneT->getPointAtAbsPos(absPtPos, canvasScaleInv);
        if(pointToReturn == nullptr) continue;
        return pointToReturn;
    }
    if(pointToReturn == nullptr) {
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
        paintT.setStyle(SkPaint::kFill_Style);
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

void Bone::addChildBone(Bone *child) {
    mChildBones << child;
    ca_addChildAnimator(child);
}

void Bone::removeChildBone(Bone *child) {
    mChildBones.removeOne(child);
    ca_removeChildAnimator(child);
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
    if(mParentBone != nullptr) {
        mParentBone->removeChildBone(this);
    }
    if(mParentBonesBox != nullptr) {
        mParentBonesBox->removeBone(this);
        mParentBonesBox = nullptr;
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
    if(mParentBone != nullptr) {
        mParentBone->removeChildBone(this);
        mParentBone = nullptr;
        mRootPt->removeRootBone(this);
        mRootPt = nullptr;
    }
    if(mParentBonesBox != nullptr) {
        mParentBonesBox->removeBone(this);
    }
    mTransformAnimator->setParentTransformAnimator(
                bonesBox->getTransformAnimator());
    if(mRootPt == nullptr) {
        mRootPt = new BonePt(mTransformAnimator.data(), TYPE_BONE_POINT);
        mRootPt->addRootBone(this);
    }
    mParentBonesBox = bonesBox;
    mParentBonesBox->addBone(this);
}

void Bone::drawHoveredOnlyThisPathSk(SkCanvas *canvas, const qreal &invScale) {
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
}

void Bone::drawHoveredPathSk(SkCanvas *canvas, const qreal &invScale) {
    drawHoveredOnlyThisPathSk(canvas, invScale);
    foreach(Bone *bone, mChildBones) {
        bone->drawHoveredPathSk(canvas, invScale);
    }
}
