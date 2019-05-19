#include "groupbox.h"
#include "durationrectangle.h"
#include "Animators/transformanimator.h"
#include "canvas.h"
#include "internallinkgroupbox.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "PropertyUpdaters/groupallpathsupdater.h"
#include "textbox.h"

GroupBox::GroupBox(const BoundingBoxType &type) : BoundingBox(type) {
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


void GroupBox::drawPixmapSk(SkCanvas * const canvas,
                            GrContext* const grContext) {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndInVisibleDurationRect())
            box->drawPixmapSk(canvas, grContext);
    }
}

void GroupBox::iniPathEffects() {
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


FillSettingsAnimator *GroupBox::getFillSettings() const {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getFillSettings();
}

OutlineSettingsAnimator *GroupBox::getStrokeSettings() const {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getStrokeSettings();
}

void GroupBox::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void GroupBox::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void GroupBox::setStrokeWidth(const qreal &strokeWidth) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeWidth(strokeWidth);
    }
}

void GroupBox::startSelectedStrokeWidthTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void GroupBox::startSelectedStrokeColorTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void GroupBox::startSelectedFillColorTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

void GroupBox::applyPaintSetting(const PaintSettingsApplier &setting) {
    for(const auto& box : mContainedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void GroupBox::setFillColorMode(const ColorMode &colorMode) {
    for(const auto& box :  mContainedBoxes) {
        box->setFillColorMode(colorMode);
    }
}

void GroupBox::setStrokeColorMode(const ColorMode &colorMode) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeColorMode(colorMode);
    }
}

const QList<qsptr<BoundingBox> > &GroupBox::getContainedBoxesList() const {
    return mContainedBoxes;
}

void GroupBox::anim_scaleTime(const int &pivotAbsFrame, const qreal &scale) {
    BoundingBox::anim_scaleTime(pivotAbsFrame, scale);

    for(const auto& box : mContainedBoxes) {
        box->anim_scaleTime(pivotAbsFrame, scale);
    }
}

bool GroupBox::differenceInFillPathEffectsBetweenFrames(const int& relFrame1,
                                                          const int& relFrame2) const {
    return mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                      relFrame2);
}


bool GroupBox::differenceInOutlinePathEffectsBetweenFrames(const int& relFrame1,
                                                             const int& relFrame2) const {
    return mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                         relFrame2);
}

bool GroupBox::differenceInPathEffectsBetweenFrames(const int& relFrame1,
                                                      const int& relFrame2) const {
    return mPathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                  relFrame2);
}

void GroupBox::addPathEffect(const qsptr<PathEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));

    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_show();
    }
    mPathEffectsAnimators->ca_addChildAnimator(effect);

    prp_afterWholeInfluenceRangeChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupBox::addFillPathEffect(const qsptr<PathEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_show();
    }
    mFillPathEffectsAnimators->ca_addChildAnimator(effect);

    prp_afterWholeInfluenceRangeChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupBox::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_show();
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);

    prp_afterWholeInfluenceRangeChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupBox::removePathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_hide();
    }

    prp_afterWholeInfluenceRangeChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupBox::removeFillPathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mFillPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_hide();
    }

    prp_afterWholeInfluenceRangeChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupBox::removeOutlinePathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mOutlinePathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_hide();
    }

    prp_afterWholeInfluenceRangeChanged();
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupBox::updateAllChildPathBoxes(const Animator::UpdateReason &reason) {
    for(const auto& box : mContainedBoxes) {
        if(box->SWT_isPathBox()) {
            GetAsPtr(box, PathBox)->setPathsOutdated();
            box->planScheduleUpdate(reason);
        } else if(box->SWT_isGroupBox()) {
            GetAsPtr(box, GroupBox)->updateAllChildPathBoxes(reason);
        }
    }
}

void GroupBox::applyPathEffects(const qreal &relFrame,
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

void GroupBox::filterOutlinePathBeforeThickness(
        const qreal &relFrame, SkPath * const srcDstPath) {
    mOutlinePathEffectsAnimators->applyBeforeThickness(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->filterOutlinePathBeforeThickness(parentRelFrame, srcDstPath);
}

void GroupBox::filterOutlinePath(const qreal &relFrame,
                                   SkPath * const srcDstPath) {
    mOutlinePathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterOutlinePath(parentRelFrame, srcDstPath);
}

void GroupBox::filterFillPath(const qreal &relFrame,
                                SkPath * const srcDstPath) {
    mFillPathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterFillPath(parentRelFrame, srcDstPath);
}

void GroupBox::scheduleWaitingTasks() {
    for(const auto &child : mContainedBoxes) {
        child->scheduleWaitingTasks();
    }
    BoundingBox::scheduleWaitingTasks();
}

void GroupBox::queScheduledTasks() {
    for(const auto &child : mContainedBoxes) {
        child->queScheduledTasks();
    }
    BoundingBox::queScheduledTasks();
}

void GroupBox::updateAllBoxes(const UpdateReason &reason) {
    for(const auto &child : mContainedBoxes) {
        child->updateAllBoxes(reason);
    }
    planScheduleUpdate(reason);
}

void GroupBox::prp_afterFrameShiftChanged() {
    ComplexAnimator::prp_afterFrameShiftChanged();
    const int thisShift = prp_getFrameShift();
    for(const auto &child : mContainedBoxes)
        child->prp_setParentFrameShift(thisShift, this);
}

void GroupBox::shiftAll(const int &shift) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(shift);
    } else {
        anim_shiftAllKeys(shift);
        for(const auto& box : mContainedBoxes) {
            box->shiftAll(shift);
        }
    }
}

QRectF GroupBox::getRelBoundingRect(const qreal &relFrame) {
    SkPath boundingPaths;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(const auto &child : mContainedBoxes) {
        const qreal childRelFrame = child->prp_absFrameToRelFrameF(absFrame);
        if(child->isVisibleAndInDurationRect(qRound(childRelFrame))) {
            SkPath childPath;
            const auto childRel = child->getRelBoundingRect(childRelFrame);
            childPath.addRect(toSkRect(childRel));

            const auto childRelTrans =
                    child->getRelativeTransformAtRelFrameF(childRelFrame);
            childPath.transform(toSkMatrix(childRelTrans));

            boundingPaths.addPath(childPath);
        }
    }
    return toQRectF(boundingPaths.computeTightBounds());
}


FrameRange GroupBox::prp_getIdenticalRelRange(const int &relFrame) const {
    auto range = BoundingBox::prp_getIdenticalRelRange(relFrame);
    const int absFrame = prp_relFrameToAbsFrame(relFrame);
    for(const auto &child : mContainedBoxes) {
        if(range.isUnary()) return range;
        auto childRange = child->prp_getIdenticalRelRange(
                    child->prp_absFrameToRelFrame(absFrame));
        auto childAbsRange = child->prp_relRangeToAbsRange(childRange);
        auto childParentRange = prp_absRangeToRelRange(childAbsRange);
        range *= childParentRange;
    }

    return range;
}

FrameRange GroupBox::getFirstAndLastIdenticalForMotionBlur(
        const int &relFrame, const bool &takeAncestorsIntoAccount) {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) {
        if(isFrameInDurationRect(relFrame)) {
            QList<Property*> propertiesT;
            getMotionBlurProperties(propertiesT);
            for(const auto& child : propertiesT) {
                if(range.isUnary()) return range;
                auto childRange = child->prp_getIdenticalRelRange(relFrame);
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


bool GroupBox::relPointInsidePath(const QPointF &relPos) const {
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

int GroupBox::getContainedBoxesCount() const {
    return mContainedBoxes.count();
}


void GroupBox::setIsCurrentGroup_k(const bool &bT) {
    mIsCurrentGroup = bT;
    setDescendantCurrentGroup(bT);
    if(!bT) {
        if(mContainedBoxes.isEmpty() && mParentGroup) {
            removeFromParent_k();
        }
    }
}

bool GroupBox::isCurrentGroup() const {
    return mIsCurrentGroup;
}

bool GroupBox::isDescendantCurrentGroup() const {
    return mIsDescendantCurrentGroup;
}


void GroupBox::setDescendantCurrentGroup(const bool &bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) planScheduleUpdate(Animator::USER_CHANGE);
    if(!mParentGroup) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *GroupBox::getBoxAtFromAllDescendents(const QPointF &absPos) {
    BoundingBox* boxAtPos = nullptr;
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto& box = mContainedBoxes.at(i);
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            boxAtPos = box->getBoxAtFromAllDescendents(absPos);
            if(boxAtPos) break;
        }
    }
    return boxAtPos;
}

void GroupBox::ungroup_k() {
    //clearBoxesSelection();
    for(auto box : mContainedBoxes) {
        removeContainedBox(box);
        mParentGroup->addContainedBox(box);
    }
    removeFromParent_k();
}


#include "typemenu.h"
void GroupBox::addActionsToMenu(BoxTypeMenu * const menu) {
    const auto ungroupAction = menu->addPlainAction<GroupBox>(
                "Ungroup", [](GroupBox * box) {
        box->ungroup_k();
    });
    ungroupAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);
    BoundingBox::addActionsToMenu(menu);
}

void GroupBox::selectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) continue;
        getParentCanvas()->addBoxToSelection(box.get());
    }
}

void GroupBox::deselectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) {
            getParentCanvas()->removeBoxFromSelection(box.get());
        }
    }
}

bool GroupBox::diffsAffectingContainedBoxes(
        const int &relFrame1, const int &relFrame2) {
    const auto idRange = BoundingBox::prp_getIdenticalRelRange(relFrame1);
    const bool diffThis = !idRange.inRange(relFrame2);
    if(mParentGroup == nullptr || diffThis) return diffThis;
    const int absFrame1 = prp_relFrameToAbsFrame(relFrame1);
    const int absFrame2 = prp_relFrameToAbsFrame(relFrame2);
    const int parentRelFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int parentRelFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);

    const bool diffInherited =
            mParentGroup->diffsAffectingContainedBoxes(
                parentRelFrame1, parentRelFrame2);
    return diffThis || diffInherited;
}

BoundingBox *GroupBox::getBoxAt(const QPointF &absPos) {
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

void GroupBox::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    for(const auto& box : mContainedBoxes) {
        box->anim_setAbsFrame(frame);
    }
}

void GroupBox::addContainedBoxesToSelection(const QRectF &rect) {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
                box->isVisibleAndInVisibleDurationRect()) {
            if(box->isContainedIn(rect)) {
                getParentCanvas()->addBoxToSelection(box.get());
            }
        }
    }
}

void GroupBox::addContainedBox(const qsptr<BoundingBox>& child) {
    //child->setParent(this);
    addContainedBoxToListAt(mContainedBoxes.count(), child);
}

void GroupBox::addContainedBoxToListAt(
        const int &index,
        const qsptr<BoundingBox>& child) {
    mContainedBoxes.insert(index, GetAsSPtr(child, BoundingBox));
    child->setParentGroup(this);
    connect(child.data(), &BoundingBox::prp_absFrameRangeChanged,
            this, &BoundingBox::prp_afterChangedAbsRange);
    updateContainedBoxIds(index);

    //SWT_addChildAbstractionForTargetToAll(child);
    SWT_addChildAbstractionForTargetToAllAt(
                child.get(), boxIdToAbstractionId(index));
    child->anim_setAbsFrame(anim_getCurrentAbsFrame());

    child->prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        auto internalLinkGroup = GetAsSPtr(box, InternalLinkGroupBox);
        internalLinkGroup->addContainedBoxToListAt(
                    index, child->createLinkForLinkGroup());
    }
}

void GroupBox::updateContainedBoxIds(const int &firstId) {
    updateContainedBoxIds(firstId, mContainedBoxes.length() - 1);
}

void GroupBox::updateContainedBoxIds(const int &firstId,
                                       const int &lastId) {
    for(int i = firstId; i <= lastId; i++) {
        mContainedBoxes.at(i)->setZListIndex(i);
    }
}


void GroupBox::removeAllContainedBoxes() {
    while(mContainedBoxes.count() > 0) {
        removeContainedBox(mContainedBoxes.takeLast());
    }
}

void GroupBox::removeContainedBoxFromList(const int &id) {
    auto box = mContainedBoxes.takeAt(id);
    if(box->SWT_isGroupBox()) {
        auto group = GetAsPtr(box, GroupBox);
        if(group->isCurrentGroup()) {
            emit group->setParentAsCurrentGroup();
        }
    }

    box->prp_afterWholeInfluenceRangeChanged();
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

int GroupBox::getContainedBoxIndex(BoundingBox *child) {
    for(int i = 0; i < mContainedBoxes.count(); i++) {
        if(mContainedBoxes.at(i) == child) return i;
    }
    return -1;
}

void GroupBox::removeContainedBox(const qsptr<BoundingBox>& child) {
    const int &index = getContainedBoxIndex(child.get());
    if(index < 0) return;
    child->removeFromSelection();
    removeContainedBoxFromList(index);
    //child->setParent(nullptr);
}

void GroupBox::removeContainedBox_k(const qsptr<BoundingBox>& child) {
    removeContainedBox(child);
    if(mContainedBoxes.isEmpty() && mParentGroup) {
        removeFromParent_k();
    }
}

void GroupBox::increaseContainedBoxZInList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedBoxInList(child, index, index + 1);
}

void GroupBox::decreaseContainedBoxZInList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, index - 1);
}

void GroupBox::bringContainedBoxToEndList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedBoxInList(child, index, mContainedBoxes.length() - 1);
}

void GroupBox::bringContainedBoxToFrontList(BoundingBox * const child) {
    const int &index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, 0);
}

void GroupBox::moveContainedBoxInList(BoundingBox * const child,
                                        const int &to) {
    const int from = getContainedBoxIndex(child);
    if(from == -1) return;
    moveContainedBoxInList(child, from, to);
}

void GroupBox::moveContainedBoxInList(BoundingBox * const child,
                                        const int &from, const int &to) {
    mContainedBoxes.move(from, to);
    updateContainedBoxIds(qMin(from, to), qMax(from, to));
    SWT_moveChildAbstractionForTargetToInAll(child, boxIdToAbstractionId(to));
    planScheduleUpdate(Animator::USER_CHANGE);

    prp_afterWholeInfluenceRangeChanged();
}

void GroupBox::moveContainedBoxBelow(BoundingBox * const boxToMove,
                                       BoundingBox * const below) {
    const int &indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

void GroupBox::moveContainedBoxAbove(BoundingBox * const boxToMove,
                                       BoundingBox * const above) {
    const int &indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(above);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

#include "singlewidgetabstraction.h"
void GroupBox::SWT_addChildrenAbstractions(
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

bool GroupBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
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

void processChildData(BoundingBox * const child,
                      LayerBoxRenderData * const parentData,
                      const qreal& childRelFrame,
                      const qreal& absFrame,
                      qreal& childrenEffectsMargin) {
    if(!child->isFrameFVisibleAndInDurationRect(childRelFrame)) return;
    if(child->SWT_isGroupBox() && !child->SWT_isLayerBox()) {
        const auto childGroup = GetAsPtr(child, GroupBox);
        const auto descs = childGroup->getContainedBoxesList();
        for(const auto& desc : descs) {
            processChildData(desc.get(), parentData,
                             desc->prp_absFrameToRelFrameF(absFrame),
                             absFrame, childrenEffectsMargin);
        }
        return;
    }
    auto boxRenderData =
            GetAsSPtr(child->getCurrentRenderData(qRound(childRelFrame)),
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
        boxRenderData->fCustomRelFrame = childRelFrame;
        boxRenderData->scheduleTask();
    }
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;

    childrenEffectsMargin =
            qMax(child->getEffectsMarginAtRelFrameF(childRelFrame),
                 childrenEffectsMargin);
}

void GroupBox::setupRenderData(const qreal &relFrame,
                               BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto groupData = GetAsPtr(data, LayerBoxRenderData);
    groupData->fChildrenRenderData.clear();
    groupData->fOtherGlobalRects.clear();
    qreal childrenEffectsMargin = 0;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(const auto& box : mContainedBoxes) {
        const qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
        processChildData(box.data(), groupData, boxRelFrame,
                         absFrame, childrenEffectsMargin);
    }

    data->fEffectsMargin += childrenEffectsMargin;
    if(!SWT_isLayerBox()) data->fOpacity = 0;
}

stdsptr<BoundingBoxRenderData> GroupBox::createRenderData() {
    return SPtrCreate(GroupBoxRenderData)(this);
}
