#include "bone.h"
#include "pointhelpers.h"
#include "canvas.h"

BonesBox::BonesBox() : BoundingBox(TYPE_BONES_BOX) {
    prp_setName("Armature");
}

void BonesBox::drawHoveredSk(SkCanvas *canvas,
                            const SkScalar &invScale) {
    for(const auto& bone : mBones) {
        bone->drawHoveredPathSk(canvas, invScale);
    }
}

bool BonesBox::relPointInsidePath(const QPointF &relPos) const {
    for(const auto& bone : mBones) {
        if(bone->getBoneAtRelPos(relPos)) { return true; }
    }
    return false;
}

bool BonesBox::SWT_isBonesBox() const { return true; }

void BonesBox::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) {
    for(const auto& bone : mBones) {
        bone->selectAndAddContainedPointsToList(absRect, list);
    }
}

void BonesBox::addBone(const qsptr<Bone>& bone) {
    mBones << bone;
    ca_addChildAnimator(bone);
    bone->setParentBonesBox(this);
}

void BonesBox::removeBone(const qsptr<Bone>& bone) {
    mBones.removeOne(bone);
    ca_removeChildAnimator(bone);
}

void BonesBox::drawPixmapSk(SkCanvas * const canvas,
                            GrContext* const grContext) {
    Q_UNUSED(grContext);
    for(const auto& bone : mBones) {
        bone->drawOnCanvas(canvas);
    }
}

void BonesBox::drawPixmapSk(SkCanvas * const canvas,
                            SkPaint * const paint,
                            GrContext* const grContext) {
    Q_UNUSED(paint);
    drawPixmapSk(canvas, grContext);
}

void BonesBox::drawCanvasControls(SkCanvas * const canvas,
                                  const CanvasMode &currentCanvasMode,
                                  const SkScalar &invScale) {
    BoundingBox::drawCanvasControls(canvas, currentCanvasMode, invScale);
    for(const auto& bone : mBones) {
        bone->drawOnCanvas(canvas, currentCanvasMode, invScale);
    }
}

MovablePoint *BonesBox::getPointAtAbsPos(
                                const QPointF &absPtPos,
                                const CanvasMode &currentCanvasMode,
                                const qreal &canvasScaleInv) {
    MovablePoint* pointToReturn =
            BoundingBox::getPointAtAbsPos(absPtPos,
                                          currentCanvasMode,
                                          canvasScaleInv);
    if(!pointToReturn) {
        for(const auto& bone : mBones) {
            pointToReturn = bone->getPointAtAbsPos(absPtPos,
                                                   canvasScaleInv);
            if(!pointToReturn) continue;
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
    for(const auto& bone : mBones) {
        Bone* boneT = bone->getBoneAtRelPos(relPos);
        if(boneT) { return boneT; }
    }
    return nullptr;
}

Bone::Bone() : ComplexAnimator("bone") {
    mTransformAnimator = SPtrCreate(BoneTransformAnimator)(this);
    mTipPt = SPtrCreate(BonePt)(mTransformAnimator.get());
    mTipPt->setTipBone(this);
    mTipPt->setRelativePos(mRelRootPos);
    ca_addChildAnimator(mTransformAnimator);
}

Bone *Bone::createBone(BonesBox *boneBox) {
    auto newBone = SPtrCreate(Bone)();
    boneBox->addBone(newBone);
    return newBone.get();
}

Bone *Bone::createBone(Bone *parentBone) {
    auto newBone = SPtrCreate(Bone)();
    parentBone->addChildBone(newBone);
    return newBone.get();
}

void Bone::deselect() {
    mSelected = false;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_SELECTED);
}

void Bone::select() {
    mSelected = true;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_SELECTED);
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

void Bone::cancelTransform() {
    mTransformAnimator->prp_cancelTransform();
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

void Bone::setAbsolutePos(const QPointF &pos) {
    setRelativePos(mParentBone->mapAbsPosToRel(pos));
}

void Bone::setRelativePos(const QPointF &relPos) {
    mTransformAnimator->setPosition(relPos.x(), relPos.y());
}

void Bone::saveTransformPivotAbsPos(const QPointF &absPivot) {
    if(!mParentBone) {
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

BonesBox *Bone::getParentBox() {
    if(!mParentBonesBox) {
        return mParentBone->getParentBox();
    }
    return GetAsPtr(mParentBonesBox, BonesBox);
}

Bone *Bone::getBoneAtRelPos(const QPointF &relPos) {
    QPointF realRelPos = mTransformAnimator->mapFromParent(relPos);
    for(const auto& boneT : mChildBones) {
        Bone* boneTT = boneT->getBoneAtRelPos(realRelPos);
        if(boneTT) return boneTT;
    }
    //if(QRectF(mRelRootPos, mRelTipPos).contains(realRelPos)) {
    if(getCurrentRelPath().contains(static_cast<SkScalar>(realRelPos.x()),
                                    static_cast<SkScalar>(realRelPos.y()))) {
            return this;
        }
    //}
    return nullptr;
}

SkPath Bone::getCurrentRelPath() {
    SkPath path;
    SkScalar len = static_cast<SkScalar>(pointToLen(mRelRootPos - mRelTipPos));
    SkScalar width = len*0.1f;
    QLineF tipRootLine = QLineF(mRelTipPos, mRelRootPos);
    QLineF verticalLine = QLineF(0., 0., 0., 10.);
    qreal angleDouble = tipRootLine.angleTo(verticalLine);
    SkScalar angle = static_cast<SkScalar>(angleDouble);

    path.moveTo(0.f, 0.f);
    path.lineTo(width, -width);
    path.lineTo(0.f, -len);
    path.lineTo(-width, -width);
    path.close();
    SkMatrix matr;
    matr.setRotate(angle);
    path.transform(matr);
    matr.reset();
    matr.setTranslate(static_cast<SkScalar>(mRelRootPos.x()),
                      static_cast<SkScalar>(mRelRootPos.y()));
    path.transform(matr);
    return path;
}

void Bone::drawOnCanvas(SkCanvas * const canvas) {
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
        canvas->drawLine(toSkPoint(rootAbs),
                         toSkPoint(parentTipAbs),
                         paintT);
    }
    SkPath path = getCurrentRelPath();
    path.transform(toSkMatrix(
                       mTransformAnimator->getTotalTransform()));
    SkPaint paintT;
    paintT.setColor(SK_ColorWHITE);
    paintT.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, paintT);

    paintT.setColor(SK_ColorBLACK);
    paintT.setStyle(SkPaint::kStroke_Style);
    paintT.setAntiAlias(true);
    canvas->drawPath(path, paintT);
    for(const auto& boneT : mChildBones) {
        boneT->drawOnCanvas(canvas);
    }
}

const QPointF &Bone::getRootRelPos() { return mRelRootPos; }

const QPointF &Bone::getTipRelPos() { return mRelTipPos; }

void Bone::setRelRootPos(const QPointF &pos) {
    mRelRootPos = pos;
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
    return mTransformAnimator.get();
}

bool Bone::isConnectedToParent() {
    return mConnectedToParent && mParentBone != nullptr;
}

void Bone::setConnectedToParent(const bool &bT) {
    if(bT == mConnectedToParent) return;
    mConnectedToParent = bT;
    if(!mParentBone) return;
    if(bT) {
        mRootPt = GetAsSPtr(mParentBone->getTipPt(), BonePt);
    } else {
        mRootPt = SPtrCreate(BonePt)(mTransformAnimator.get());
    }
    mRootPt->addRootBone(this);
}

Bone *Bone::getParentBone() {
    return mParentBone;
}

BonePt *Bone::getTipPt() {
    return mTipPt.get();
}

BonePt *Bone::getRootPt() {
    return mRootPt.get();
}

MovablePoint *Bone::getPointAtAbsPos(const QPointF &absPtPos,
                                       const qreal &canvasScaleInv) {
    MovablePoint* pointToReturn = nullptr;
    for(const auto& boneT : mChildBones) {
        pointToReturn = boneT->getPointAtAbsPos(absPtPos, canvasScaleInv);
        if(!pointToReturn) continue;
        return pointToReturn;
    }
    if(!pointToReturn) {
        if(mTipPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mTipPt.get();
        }
        if(mRootPt->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mRootPt.get();
        }
    }
    return pointToReturn;
}

void Bone::drawOnCanvas(SkCanvas * const canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale) {
    canvas->save();
    if(mSelected) {
        SkPath path = getCurrentRelPath();
        path.transform(toSkMatrix(
                           mTransformAnimator->getTotalTransform()));
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
    for(const auto& boneT : mChildBones) {
        boneT->drawOnCanvas(canvas, currentCanvasMode, invScale);
    }
    canvas->restore();
}

void Bone::addChildBone(const qsptr<Bone>& child) {
    mChildBones << child;
    ca_addChildAnimator(child);
    child->setParentBone(this);
}

void Bone::removeChildBone(const qsptr<Bone>& child) {
    if(mChildBones.removeOne(child)) {
        ca_removeChildAnimator(child);
    }
}

void Bone::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) {
    for(const auto& boneT : mChildBones) {
        boneT->selectAndAddContainedPointsToList(absRect, list);
    }
    if(mTipPt->isContainedInRect(absRect)) {
        list.append(mTipPt.get());
        mTipPt->select();
    }
    if(mRootPt->getParentBone() == this) {
        if(mRootPt->isContainedInRect(absRect)) {
            list.append(mRootPt.get());
            mRootPt->select();
        }
    }
}

void Bone::clearParentBonesBox() {
    if(mParentBonesBox) {
        mParentBonesBox->removeBone(ref<Bone>());
        mParentBonesBox = nullptr;
    }
}

void Bone::clearParentBone() {
    if(mParentBone) {
        mParentBone->removeChildBone(ref<Bone>());
        mParentBone = nullptr;
        mRootPt->removeRootBone(this);
        mRootPt = nullptr;
    }
}

void Bone::setParentBone(Bone *parentBone) {
    if(parentBone == mParentBone) return;
    clearCurrentParent();

    mTransformAnimator->setParentTransformAnimator(
                parentBone->getTransformAnimator());
    mRootPt = GetAsSPtr(parentBone->getTipPt(), BonePt);
    setAbsRootPos(mRootPt->getAbsolutePos());
    mRootPt->addRootBone(this);
    mParentBone = parentBone;
}

void Bone::setParentBonesBox(BonesBox *bonesBox) {
    if(bonesBox == mParentBonesBox) return;
    clearCurrentParent();

    mTransformAnimator->setParentTransformAnimator(
                bonesBox->getTransformAnimator());
    if(!mRootPt) {
        mRootPt = SPtrCreate(BonePt)(mTransformAnimator.get());
        mRootPt->addRootBone(this);
    }
    mParentBonesBox = bonesBox;
}

void Bone::drawHoveredOnlyThisPathSk(SkCanvas *canvas,
                                     const SkScalar &invScale) {
    canvas->save();
    SkPath mappedPath = getCurrentRelPath();
    mappedPath.transform(toSkMatrix(
                             mTransformAnimator->getTotalTransform()));
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2.f*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(mappedPath, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(invScale);
    canvas->drawPath(mappedPath, paint);
    canvas->restore();
}

void Bone::drawHoveredPathSk(SkCanvas *canvas, const SkScalar &invScale) {
    drawHoveredOnlyThisPathSk(canvas, invScale);
    for(const auto& bone : mChildBones) {
        bone->drawHoveredPathSk(canvas, invScale);
    }
}

BonePt::BonePt(BasicTransformAnimator* parent) :
    NonAnimatedMovablePoint(parent, TYPE_BONE_POINT, 7.5) {}

void BonePt::setRelativePos(const QPointF &relPos) {
    NonAnimatedMovablePoint::setRelativePos(relPos);
    if(mTipBone) {
        if(mParentBone == mTipBone) {
            mTipBone->setRelTipPos(relPos);
        } else {
            mTipBone->setAbsTipPos(
                        mParentBone->getTransformAnimator()->
                        mapRelPosToAbs(relPos));
        }
    }
    for(Bone *rootBone : mRootBones) {
        if(mParentBone == rootBone) {
            rootBone->setRelRootPos(relPos);
        } else {
            rootBone->setAbsRootPos(
                        mParentBone->getTransformAnimator()->
                        mapRelPosToAbs(relPos));
        }
    }
}

void BonePt::setTipBone(Bone *bone) {
    mTipBone = bone;
    if(bone) {
        setParentBone(bone);
    }
}

void BonePt::addRootBone(Bone *bone) {
    mRootBones << bone;
    if(!mTipBone) {
        setParentBone(bone);
    }
}

void BonePt::removeRootBone(Bone *bone) {
    mRootBones.removeOne(bone);
}

void BonePt::setParentBone(Bone *bone) {
    mParentBone = bone;
    if(bone) {
        mParentTransform_cv = mParentBone->getTransformAnimator();
    }
}

Bone *BonePt::getTipBone() {
    return mTipBone;
}

Bone *BonePt::getParentBone() {
    return mParentBone;
}
