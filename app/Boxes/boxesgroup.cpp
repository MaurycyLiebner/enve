#include "Boxes/boxesgroup.h"
#include "undoredo.h"
#include <QApplication>
#include "GUI/mainwindow.h"
#include "MovablePoints/ctrlpoint.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "textbox.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "GUI/canvaswindow.h"
#include "canvas.h"
#include "Boxes/particlebox.h"
#include "durationrectangle.h"
#include "linkbox.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "PropertyUpdaters/groupallpathsupdater.h"
#include "Animators/transformanimator.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"

bool BoxesGroup::mCtrlsAlwaysVisible = false;

//bool zMoreThan(BoundingBox *box1, BoundingBox *box2)
//{
//    return box1->getZIndex() < box2->getZIndex();
//}

BoxesGroup::BoxesGroup(const BoundingBoxType &type) :
    BoundingBox(type) {
    setName("Group");
    iniPathEffects();
}

//bool BoxesGroup::anim_nextRelFrameWithKey(const int &relFrame,
//                                         int &nextRelFrame) {
//    int thisMinNextFrame = BoundingBox::anim_nextRelFrameWithKey(relFrame);
//    return thisMinNextFrame;
//    int minNextAbsFrame = FrameRange::EMAX;
//    for(const auto& box : mContainedBoxes) {
//        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
//        int boxNext = box->anim_nextRelFrameWithKey(boxRelFrame);
//        int absNext = box->prp_relFrameToAbsFrame(boxNext);
//        if(minNextAbsFrame > absNext) {
//            minNextAbsFrame = absNext;
//        }
//    }

//    return qMin(prp_absFrameToRelFrame(minNextAbsFrame), thisMinNextFrame);
//}

//int BoxesGroup::anim_prevRelFrameWithKey(const int &relFrame,
//                                        int &prevRelFrame) {
//    int thisMaxPrevFrame = BoundingBox::anim_nextRelFrameWithKey(relFrame);
//    return thisMaxPrevFrame;
//    int maxPrevAbsFrame = FrameRange::EMIN;
//    for(const auto& box : mContainedBoxes) {
//        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
//        int boxPrev = box->anim_prevRelFrameWithKey(boxRelFrame);
//        int absPrev = box->prp_relFrameToAbsFrame(boxPrev);
//        if(maxPrevAbsFrame < absPrev) {
//            maxPrevAbsFrame = absPrev;
//        }
//    }
//    return qMax(maxPrevAbsFrame, thisMaxPrevFrame);
//}

void BoxesGroup::iniPathEffects() {
    mPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)(false, false, this);
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mPathEffectsAnimators);
    mPathEffectsAnimators->SWT_hide();

    mFillPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)(false, true, this);
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mFillPathEffectsAnimators);
    mFillPathEffectsAnimators->SWT_hide();

    mOutlinePathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)(true, false, this);
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mOutlinePathEffectsAnimators);
    mOutlinePathEffectsAnimators->SWT_hide();
}

bool BoxesGroup::differenceInFillPathEffectsBetweenFrames(const int& relFrame1,
                                                          const int& relFrame2) const {
    return mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                      relFrame2);
}


bool BoxesGroup::differenceInOutlinePathEffectsBetweenFrames(const int& relFrame1,
                                                             const int& relFrame2) const {
    return mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                         relFrame2);
}

bool BoxesGroup::differenceInPathEffectsBetweenFrames(const int& relFrame1,
                                                      const int& relFrame2) const {
    return mPathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                  relFrame2);
}

void BoxesGroup::addPathEffect(const qsptr<PathEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));

    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_show();
    }
    if(effect->getEffectType() == GROUP_SUM_PATH_EFFECT) {
        mGroupPathSumEffects << effect;
    }
    mPathEffectsAnimators->ca_addChildAnimator(effect);

    prp_updateInfluenceRangeAfterChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void BoxesGroup::addFillPathEffect(const qsptr<PathEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_show();
    }
    mFillPathEffectsAnimators->ca_addChildAnimator(effect);

    prp_updateInfluenceRangeAfterChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void BoxesGroup::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_show();
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);

    prp_updateInfluenceRangeAfterChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void BoxesGroup::removePathEffect(const qsptr<PathEffect>& effect) {
    if(effect->getEffectType() == GROUP_SUM_PATH_EFFECT) {
        mGroupPathSumEffects.removeOne(effect);
    }
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_hide();
    }

    prp_updateInfluenceRangeAfterChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void BoxesGroup::removeFillPathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mFillPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_hide();
    }

    prp_updateInfluenceRangeAfterChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void BoxesGroup::removeOutlinePathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mOutlinePathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_hide();
    }

    prp_updateInfluenceRangeAfterChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void BoxesGroup::updateAllChildPathBoxes(const Animator::UpdateReason &reason) {
    for(const auto& box : mContainedBoxes) {
        if(box->SWT_isPathBox()) {
            GetAsPtr(box, PathBox)->setPathsOutdated();
            box->scheduleUpdate(reason);
        } else if(box->SWT_isBoxesGroup()) {
            GetAsPtr(box, BoxesGroup)->updateAllChildPathBoxes(reason);
        }
    }
}

void BoxesGroup::applyPathEffects(const qreal &relFrame,
                                  SkPath * const srcDstPath,
                                  BoundingBox * const box) {
    if(mParentGroup) {
        const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        const qreal parentRelFrame =
                mParentGroup->prp_absFrameToRelFrameF(absFrame);
        mParentGroup->applyPathEffects(parentRelFrame, srcDstPath, box);
    }
    mPathEffectsAnimators->apply(relFrame, srcDstPath);

//    if(!mParentGroup) return;
//    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
//                prp_relFrameToAbsFrameF(relFrame));
//    mParentGroup->apply(parentRelFrame, srcDstPath, box);
}

void BoxesGroup::filterOutlinePathBeforeThicknessForRelFrame(
        const qreal &relFrame, SkPath * const srcDstPath) {
    mOutlinePathEffectsAnimators->applyBeforeThickness(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->filterOutlinePathBeforeThicknessForRelFrame(parentRelFrame,
                                                              srcDstPath);
}

bool BoxesGroup::isLastPathBox(PathBox * const pathBox) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto childAtI = mContainedBoxes.at(i).data();
        if(childAtI == pathBox) return true;
        if(childAtI->SWT_isPathBox()) return false;
    }
    return false;
}

void BoxesGroup::filterOutlinePathForRelFrame(const qreal &relFrame,
                                              SkPath * const srcDstPath) {
    mOutlinePathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterOutlinePathForRelFrame(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterFillPathForRelFrame(const qreal &relFrame,
                                           SkPath * const srcDstPath) {
    mFillPathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterFillPathForRelFrame(parentRelFrame, srcDstPath);
}

void BoxesGroup::prp_setParentFrameShift(const int &shift,
                                         ComplexAnimator *parentAnimator) {
    ComplexAnimator::prp_setParentFrameShift(shift, parentAnimator);
    const int thisShift = prp_getFrameShift();
    for(const auto &child : mContainedBoxes) {
        child->prp_setParentFrameShift(thisShift, this);
    }
}

void BoxesGroup::scheduleWaitingTasks() {
    for(const auto &child : mContainedBoxes) {
        child->scheduleWaitingTasks();
    }
    BoundingBox::scheduleWaitingTasks();
}

void BoxesGroup::queScheduledTasks() {
    for(const auto &child : mContainedBoxes) {
        child->queScheduledTasks();
    }
    BoundingBox::queScheduledTasks();
}

void BoxesGroup::updateAllBoxes(const UpdateReason &reason) {
    for(const auto &child : mContainedBoxes) {
        child->updateAllBoxes(reason);
    }
    scheduleUpdate(reason);
}

QRectF BoxesGroup::getRelBoundingRectAtRelFrame(const qreal &relFrame) {
    SkPath boundingPaths;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(const auto &child : mContainedBoxes) {
        const qreal childRelFrame = child->prp_absFrameToRelFrameF(absFrame);
        if(child->isRelFrameVisibleAndInVisibleDurationRect(qRound(childRelFrame))) {
            SkPath childPath;
            childPath.addRect(
                        toSkRect(
                            child->getRelBoundingRectAtRelFrame(childRelFrame)));
            childPath.transform(
                        toSkMatrix(
                            child->getTransformAnimator()->
                            getRelativeTransformAtRelFrameF(childRelFrame)) );
            boundingPaths.addPath(childPath);
        }
    }
    return toQRectF(boundingPaths.computeTightBounds());
}

bool BoxesGroup::prp_differencesBetweenRelFramesIncludingInheritedExcludingContainedBoxes(
        const int &relFrame1, const int &relFrame2) {
    const auto idRange = BoundingBox::prp_getIdenticalRelFrameRange(relFrame1);
    const bool diffThis = !idRange.inRange(relFrame2);
    if(mParentGroup == nullptr || diffThis) return diffThis;
    const int absFrame1 = prp_relFrameToAbsFrame(relFrame1);
    const int absFrame2 = prp_relFrameToAbsFrame(relFrame2);
    const int parentRelFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int parentRelFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);

    const bool diffInherited =
            mParentGroup->prp_differencesBetweenRelFramesIncludingInheritedExcludingContainedBoxes(
                parentRelFrame1, parentRelFrame2);
    return diffThis || diffInherited;
}

FrameRange BoxesGroup::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    auto range = BoundingBox::prp_getIdenticalRelFrameRange(relFrame);
    const int absFrame = prp_relFrameToAbsFrame(relFrame);
    for(const auto &child : mContainedBoxes) {
        if(range.isUnary()) return range;
        auto childRange = child->prp_getIdenticalRelFrameRange(
                    child->prp_absFrameToRelFrame(absFrame));
        auto childAbsRange = child->prp_relRangeToAbsRange(childRange);
        auto childParentRange = prp_absRangeToRelRange(childAbsRange);
        range *= childParentRange;
    }

    return range;
}

FrameRange BoxesGroup::getFirstAndLastIdenticalForMotionBlur(
        const int &relFrame, const bool &takeAncestorsIntoAccount) {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            QList<Property*> propertiesT;
            getMotionBlurProperties(propertiesT);
            for(const auto& child : propertiesT) {
                if(range.isUnary()) return range;
                auto childRange = child->prp_getIdenticalRelFrameRange(relFrame);
                range *= childRange;
            }

            for(const auto &child : mContainedBoxes) {
                if(range.isUnary()) return range;
                auto childRange = child->getFirstAndLastIdenticalForMotionBlur(
                            relFrame, false);
                range *= childRange;
            }

            if(mDurationRectangle) {
                range *= mDurationRectangle->getRelFrameRange();
            }
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheLeft();
            }
        }
    }
    if(!mParentGroup || takeAncestorsIntoAccount) return range;
    if(range.isUnary()) return range;
    int parentRel = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    auto parentRange = mParentGroup->BoundingBox::getFirstAndLastIdenticalForMotionBlur(parentRel);
    return range*parentRange;
}

BoxesGroup::~BoxesGroup() {}

void BoxesGroup::anim_scaleTime(const int &pivotAbsFrame, const qreal &scale) {
    BoundingBox::anim_scaleTime(pivotAbsFrame, scale);

    for(const auto& box : mContainedBoxes) {
        box->anim_scaleTime(pivotAbsFrame, scale);
    }
}

bool BoxesGroup::relPointInsidePath(const QPointF &relPos) const {
    if(mRelBoundingRect.contains(relPos)) {
        const QPointF absPos = mapRelPosToAbs(relPos);
        for(const auto& box : mContainedBoxes) {
            if(box->absPointInsidePath(absPos)) {
                return true;
            }
        }
    }
    return false;
}

void BoxesGroup::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void BoxesGroup::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void BoxesGroup::setStrokeWidth(const qreal &strokeWidth) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeWidth(strokeWidth);
    }
}

void BoxesGroup::startSelectedStrokeWidthTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void BoxesGroup::startSelectedStrokeColorTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void BoxesGroup::startSelectedFillColorTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

void BoxesGroup::applyPaintSetting(const PaintSettingsApplier &setting) {
    for(const auto& box : mContainedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void BoxesGroup::setFillColorMode(const ColorMode &colorMode) {
    for(const auto& box :  mContainedBoxes) {
        box->setFillColorMode(colorMode);
    }
}

void BoxesGroup::setStrokeColorMode(const ColorMode &colorMode) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeColorMode(colorMode);
    }
}

void BoxesGroup::shiftAll(const int &shift) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(shift);
    } else {
        anim_shiftAllKeys(shift);
        for(const auto& box : mContainedBoxes) {
            box->shiftAll(shift);
        }
    }
}

const QList<qsptr<BoundingBox> > &BoxesGroup::getContainedBoxesList() const {
    return mContainedBoxes;
}

int BoxesGroup::getContainedBoxesCount() const {
    return mContainedBoxes.count();
}

qsptr<BoundingBox> BoxesGroup::createLink() {
    auto linkBox = SPtrCreate(InternalLinkGroupBox)(this);
    copyBoundingBoxDataTo(linkBox.get());
    return std::move(linkBox);
}

void processChildData(BoundingBox* child,
                      BoxesGroupRenderData* parentData,
                      const qreal& boxRelFrame) {
    auto boxRenderData =
            GetAsSPtr(child->getCurrentRenderData(qRound(boxRelFrame)),
                      BoundingBoxRenderData);
    if(boxRenderData) {
        if(boxRenderData->fCopied) {
            child->nullifyCurrentRenderData(boxRenderData->fRelFrame);
        }
    } else {
        boxRenderData = child->createRenderData();
        boxRenderData->fReason = parentData->fReason;
        //boxRenderData->parentIsTarget = false;
        boxRenderData->fUseCustomRelFrame = true;
        boxRenderData->fCustomRelFrame = boxRelFrame;
        boxRenderData->scheduleTask();
    }
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;
}

void BoxesGroup::setupBoundingBoxRenderDataForRelFrameF(
                        const qreal &relFrame,
                        BoundingBoxRenderData* data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
    const auto groupData = GetAsSPtr(data, BoxesGroupRenderData);
    groupData->fChildrenRenderData.clear();
    groupData->fOtherGlobalRects.clear();
    qreal childrenEffectsMargin = 0;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(const auto& box : mContainedBoxes) {
        const qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
        if(box->isRelFrameFVisibleAndInVisibleDurationRect(boxRelFrame)) {
            processChildData(box.data(), groupData.get(), boxRelFrame);

            childrenEffectsMargin =
                    qMax(box->getEffectsMarginAtRelFrameF(boxRelFrame),
                         childrenEffectsMargin);
        }
    }

    data->fEffectsMargin += childrenEffectsMargin;
}


void BoxesGroup::drawPixmapSk(SkCanvas * const canvas,
                              GrContext* const grContext) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawPixmapSk(canvas, grContext);
    } else {
        SkPaint paint;
        const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
        paint.setAlpha(static_cast<U8CPU>(intAlpha));
        paint.setBlendMode(mBlendModeSk);
        canvas->saveLayer(nullptr, &paint);
        for(const auto& box : mContainedBoxes) {
            //box->draw(p);
            box->drawPixmapSk(canvas, grContext);
        }
        canvas->restore();
    }
}

void BoxesGroup::setIsCurrentGroup_k(const bool &bT) {
    mIsCurrentGroup = bT;
    setDescendantCurrentGroup(bT);
    if(!bT) {
        if(mContainedBoxes.isEmpty() && mParentGroup) {
            removeFromParent_k();
        }
    }
}

bool BoxesGroup::isCurrentGroup() const {
    return mIsCurrentGroup;
}

bool BoxesGroup::isDescendantCurrentGroup() const {
    return mIsDescendantCurrentGroup;
}

bool BoxesGroup::shouldPaintOnImage() const {
    if(SWT_isLinkBox()) return true;
    if(mIsDescendantCurrentGroup) return false;
//    return !mIsDescendantCurrentGroup; !!!
    return mEffectsAnimators->hasEffects() ||
           mPathEffectsAnimators->hasEffects() ||
           mOutlinePathEffectsAnimators->hasEffects() ||
           mFillPathEffectsAnimators->hasEffects();
}

bool BoxesGroup::SWT_isBoxesGroup() const { return true; }

void BoxesGroup::setDescendantCurrentGroup(const bool &bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) scheduleUpdate(Animator::USER_CHANGE);
    if(!mParentGroup) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *BoxesGroup::getPathAtFromAllAncestors(const QPointF &absPos) {
    BoundingBox* boxAtPos = nullptr;
    //Q_FOREACHBoxInListInverted(mChildren) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto& box = mContainedBoxes.at(i);
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            boxAtPos = box->getPathAtFromAllAncestors(absPos);
            if(boxAtPos) break;
        }
    }
    return boxAtPos;
}

void BoxesGroup::ungroup_k() {
    //clearBoxesSelection();
    for(auto box : mContainedBoxes) {
        box->applyTransformation(mTransformAnimator.data());
        removeContainedBox(box);
        mParentGroup->addContainedBox(box);
    }
    removeFromParent_k();
}

FillSettingsAnimator *BoxesGroup::getFillSettings() const {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getFillSettings();
}

OutlineSettingsAnimator *BoxesGroup::getStrokeSettings() const {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getStrokeSettings();
}
void BoxesGroup::applyCurrentTransformation() {
    mNReasonsNotToApplyUglyTransform++;
    const QPointF absPivot = getPivotAbsPos();
    const qreal rotation = mTransformAnimator->rot();
    const qreal scaleX = mTransformAnimator->xScale();
    const qreal scaleY = mTransformAnimator->yScale();
    const QPointF relTrans = mTransformAnimator->pos();
    for(const auto& box : mContainedBoxes) {
        box->saveTransformPivotAbsPos(absPivot);
        box->startTransform();
        box->rotateRelativeToSavedPivot(rotation);
        box->finishTransform();
        box->startTransform();
        box->scaleRelativeToSavedPivot(scaleX, scaleY);
        box->finishTransform();
        box->startPosTransform();
        box->moveByRel(relTrans);
        box->finishTransform();
    }

    mTransformAnimator->resetRotation();
    mTransformAnimator->resetScale();
    mTransformAnimator->resetTranslation();
    mNReasonsNotToApplyUglyTransform--;
}

void BoxesGroup::selectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) continue;
        getParentCanvas()->addBoxToSelection(box.get());
    }
}

void BoxesGroup::deselectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) {
            getParentCanvas()->removeBoxFromSelection(box.get());
        }
    }
}

BoundingBox *BoxesGroup::getBoxAt(const QPointF &absPos) {
    BoundingBox* boxAtPos = nullptr;

    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto& box = mContainedBoxes.at(i);
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            if(box->absPointInsidePath(absPos)) {
                boxAtPos = box.get();
                break;
            }
        }
    }
    return boxAtPos;
}

void BoxesGroup::addContainedBoxesToSelection(const QRectF &rect) {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            if(box->isContainedIn(rect)) {
                getParentCanvas()->addBoxToSelection(box.get());
            }
        }
    }
}

void BoxesGroup::addContainedBox(const qsptr<BoundingBox>& child) {
    //child->setParent(this);
    addContainedBoxToListAt(mContainedBoxes.count(), child);
}

void BoxesGroup::addContainedBoxToListAt(
        const int &index,
        const qsptr<BoundingBox>& child) {
    mContainedBoxes.insert(index, GetAsSPtr(child, BoundingBox));
    child->setParentGroup(this);
    connect(child.data(),
            &BoundingBox::prp_absFrameRangeChanged,
            this, &BoundingBox::prp_updateAfterChangedAbsFrameRange);
    updateContainedBoxIds(index);

    //SWT_addChildAbstractionForTargetToAll(child);
    SWT_addChildAbstractionForTargetToAllAt(
                child.get(), boxIdToAbstractionId(index));
    child->anim_setAbsFrame(anim_getCurrentAbsFrame());

    child->prp_updateInfluenceRangeAfterChanged();

    for(const auto& box : mLinkingBoxes) {
        auto internalLinkGroup = GetAsSPtr(box, InternalLinkGroupBox);
        internalLinkGroup->addContainedBoxToListAt(
                    index, child->createLinkForLinkGroup());
    }
}

void BoxesGroup::updateContainedBoxIds(const int &firstId) {
    updateContainedBoxIds(firstId, mContainedBoxes.length() - 1);
}

void BoxesGroup::updateContainedBoxIds(const int &firstId,
                                       const int &lastId) {
    for(int i = firstId; i <= lastId; i++) {
        mContainedBoxes.at(i)->setZListIndex(i);
    }
}

void BoxesGroup::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    for(const auto& box : mContainedBoxes) {
        box->anim_setAbsFrame(frame);
    }
}

stdsptr<BoundingBoxRenderData> BoxesGroup::createRenderData() {
    return SPtrCreate(BoxesGroupRenderData)(this);
}

void BoxesGroup::removeContainedBoxFromList(const int &id) {
    auto box = mContainedBoxes.takeAt(id);
    if(box->SWT_isBoxesGroup()) {
        auto group = GetAsPtr(box, BoxesGroup);
        if(group->isCurrentGroup()) {
            emit group->setParentAsCurrentGroup();
        }
    }

    box->prp_updateInfluenceRangeAfterChanged();
    if(box->isSelected()) box->removeFromSelection();
    disconnect(box.data(), nullptr, this, nullptr);

    updateContainedBoxIds(id);

    SWT_removeChildAbstractionForTargetFromAll(box.get());
    box->setParentGroup(nullptr);

    for(const auto& box : mLinkingBoxes) {
        const auto internalLinkGroup = GetAsSPtr(box, InternalLinkGroupBox);
        internalLinkGroup->removeContainedBoxFromList(id);
    }
}

int BoxesGroup::getContainedBoxIndex(BoundingBox *child) {
    for(int i = 0; i < mContainedBoxes.count(); i++) {
        if(mContainedBoxes.at(i) == child) return i;
    }
    return -1;
}

void BoxesGroup::removeContainedBox(const qsptr<BoundingBox>& child) {
    const int &index = getContainedBoxIndex(child.get());
    if(index < 0) return;
    child->removeFromSelection();
    removeContainedBoxFromList(index);
    //child->setParent(nullptr);
}

void BoxesGroup::removeContainedBox_k(const qsptr<BoundingBox>& child) {
    removeContainedBox(child);
    if(mContainedBoxes.isEmpty() && mParentGroup) {
        removeFromParent_k();
    }
}

void BoxesGroup::increaseContainedBoxZInList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedBoxInList(child, index, index + 1);
}

void BoxesGroup::decreaseContainedBoxZInList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, index - 1);
}

void BoxesGroup::bringContainedBoxToEndList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedBoxInList(child, index, mContainedBoxes.length() - 1);
}

void BoxesGroup::bringContainedBoxToFrontList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, 0);
}

void BoxesGroup::moveContainedBoxInList(BoundingBox * const child,
                                        const int &to) {
    const int from = getContainedBoxIndex(child);
    if(from == -1) return;
    moveContainedBoxInList(child, from, to);
}

void BoxesGroup::moveContainedBoxInList(BoundingBox * const child,
                                        const int &from, const int &to) {
    mContainedBoxes.move(from, to);
    updateContainedBoxIds(qMin(from, to), qMax(from, to));
    SWT_moveChildAbstractionForTargetToInAll(child, boxIdToAbstractionId(to));
    scheduleUpdate(Animator::USER_CHANGE);

    prp_updateInfluenceRangeAfterChanged();
}

void BoxesGroup::moveContainedBoxBelow(BoundingBox * const boxToMove,
                                       BoundingBox * const below) {
    const int &indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

void BoxesGroup::moveContainedBoxAbove(BoundingBox * const boxToMove,
                                       BoundingBox * const above) {
    const int &indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(above);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

#include "singlewidgetabstraction.h"
void BoxesGroup::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction* abstraction,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartWidgetId) {
    BoundingBox::SWT_addChildrenAbstractions(abstraction, updateFuncs,
                                             visiblePartWidgetId);

    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto &child = mContainedBoxes.at(i);
        auto abs = child->SWT_getOrCreateAbstractionForWidget(updateFuncs,
                                                              visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}

bool BoxesGroup::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool &parentSatisfies,
                                     const bool &parentMainTarget) const {
    const SWT_BoxRule &rule = rules.fRule;
    if(rule == SWT_BR_SELECTED) {
        return BoundingBox::SWT_shouldBeVisible(rules,
                                                parentSatisfies,
                                                parentMainTarget) &&
                !isCurrentGroup();
    }
    return BoundingBox::SWT_shouldBeVisible(rules,
                                            parentSatisfies,
                                            parentMainTarget);
}
