#include "containerbox.h"
#include "GUI/Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "canvas.h"
#include "internallinkgroupbox.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "PropertyUpdaters/groupallpathsupdater.h"
#include "textbox.h"
#include "Animators/gpueffectanimators.h"
#include "Animators/effectanimators.h"

ContainerBox::ContainerBox(const BoundingBoxType &type) :
    BoundingBox(type) {
    iniPathEffects();
}

void ContainerBox::iniPathEffects() {
    mPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mPathEffectsAnimators);

    mFillPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mFillPathEffectsAnimators);

    mOutlineBasePathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mOutlineBasePathEffectsAnimators->prp_setName("outline base effects");
    mOutlineBasePathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mOutlineBasePathEffectsAnimators);

    mOutlinePathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(GroupAllPathsUpdater)(this));
    ca_addChildAnimator(mOutlinePathEffectsAnimators);
}


//bool BoxesGroup::anim_nextRelFrameWithKey(const int relFrame,
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

//int BoxesGroup::anim_prevRelFrameWithKey(const int relFrame,
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

FillSettingsAnimator *ContainerBox::getFillSettings() const {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getFillSettings();
}

OutlineSettingsAnimator *ContainerBox::getStrokeSettings() const {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getStrokeSettings();
}

void ContainerBox::setStrokeCapStyle(const SkPaint::Cap capStyle) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void ContainerBox::setStrokeJoinStyle(const SkPaint::Join joinStyle) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void ContainerBox::strokeWidthAction(const QrealAction& action) {
    for(const auto& box : mContainedBoxes)
        box->strokeWidthAction(action);
}

void ContainerBox::startSelectedStrokeColorTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void ContainerBox::startSelectedFillColorTransform() {
    for(const auto& box : mContainedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

void ContainerBox::applyPaintSetting(const PaintSettingsApplier &setting) {
    for(const auto& box : mContainedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void ContainerBox::setFillColorMode(const ColorMode colorMode) {
    for(const auto& box :  mContainedBoxes) {
        box->setFillColorMode(colorMode);
    }
}

void ContainerBox::setStrokeColorMode(const ColorMode colorMode) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeColorMode(colorMode);
    }
}

const QList<qsptr<BoundingBox> > &ContainerBox::getContainedBoxesList() const {
    return mContainedBoxes;
}

void ContainerBox::anim_scaleTime(const int pivotAbsFrame, const qreal scale) {
    BoundingBox::anim_scaleTime(pivotAbsFrame, scale);

    for(const auto& box : mContainedBoxes) {
        box->anim_scaleTime(pivotAbsFrame, scale);
    }
}

bool ContainerBox::differenceInFillPathEffectsBetweenFrames(const int relFrame1,
                                                          const int relFrame2) const {
    return mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                      relFrame2);
}


bool ContainerBox::differenceInOutlinePathEffectsBetweenFrames(const int relFrame1,
                                                             const int relFrame2) const {
    return mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                         relFrame2);
}

bool ContainerBox::differenceInPathEffectsBetweenFrames(const int relFrame1,
                                                      const int relFrame2) const {
    return mPathEffectsAnimators->prp_differencesBetweenRelFrames(relFrame1,
                                                                  relFrame2);
}

void ContainerBox::addPathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::addFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::addOutlineBasePathEffect(const qsptr<PathEffect>& effect) {
    mOutlineBasePathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::removePathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->removeChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::removeFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->removeChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::removeOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->removeChild(effect);
    updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void ContainerBox::updateAllChildPathBoxes(const Animator::UpdateReason &reason) {
    for(const auto& box : mContainedBoxes) {
        if(box->SWT_isPathBox()) {
            GetAsPtr(box, PathBox)->setPathsOutdated();
            box->planScheduleUpdate(reason);
        } else if(box->SWT_isContainerBox()) {
            GetAsPtr(box, ContainerBox)->updateAllChildPathBoxes(reason);
        }
    }
}

void ContainerBox::applyPathEffects(const qreal relFrame,
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

void ContainerBox::filterOutlineBasePath(
        const qreal relFrame, SkPath * const srcDstPath) {
    mOutlineBasePathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->filterOutlineBasePath(parentRelFrame, srcDstPath);
}

void ContainerBox::filterOutlinePath(const qreal relFrame,
                                   SkPath * const srcDstPath) {
    mOutlinePathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterOutlinePath(parentRelFrame, srcDstPath);
}

void ContainerBox::filterFillPath(const qreal relFrame,
                                SkPath * const srcDstPath) {
    mFillPathEffectsAnimators->apply(relFrame, srcDstPath);
    if(!mParentGroup) return;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterFillPath(parentRelFrame, srcDstPath);
}

void ContainerBox::scheduleChildWaitingTasks() {
    for(const auto &child : mContainedBoxes) {
        child->scheduleWaitingTasks();
    }
}

void ContainerBox::scheduleWaitingTasks() {
    scheduleChildWaitingTasks();
    BoundingBox::scheduleWaitingTasks();
}

void ContainerBox::queChildScheduledTasks() {
    for(const auto &child : mContainedBoxes) {
        child->queScheduledTasks();
    }
}

void ContainerBox::queScheduledTasks() {
    BoundingBox::queScheduledTasks();
    queChildScheduledTasks();
}

void ContainerBox::promoteToLayer() {
    if(!SWT_isLinkBox()) mType = TYPE_LAYER;
    if(prp_mName.contains("Group")) {
        auto newName  = prp_mName;
        newName.replace("Group", "Layer");
        setName(newName);
    }
    mEffectsAnimators->SWT_enable();
    mGPUEffectsAnimators->SWT_enable();
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        GetAsPtr(box, ContainerBox)->promoteToLayer();
    }
}

void ContainerBox::demoteToGroup() {
    if(!SWT_isLinkBox()) mType = TYPE_GROUP;
    if(prp_mName.contains("Layer")) {
        auto newName  = prp_mName;
        newName.replace("Layer", "Group");
        setName(newName);
    }
    mEffectsAnimators->SWT_disable();
    mGPUEffectsAnimators->SWT_disable();
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        GetAsPtr(box, ContainerBox)->demoteToGroup();
    }
}

void ContainerBox::updateAllBoxes(const UpdateReason &reason) {
    for(const auto &child : mContainedBoxes) {
        child->updateAllBoxes(reason);
    }
    planScheduleUpdate(reason);
}

void ContainerBox::prp_afterFrameShiftChanged() {
    ComplexAnimator::prp_afterFrameShiftChanged();
    const int thisShift = prp_getFrameShift();
    for(const auto &child : mContainedBoxes)
        child->prp_setParentFrameShift(thisShift, this);
}

void ContainerBox::shiftAll(const int shift) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(shift);
    } else {
        anim_shiftAllKeys(shift);
        for(const auto& box : mContainedBoxes) {
            box->shiftAll(shift);
        }
    }
}

QRectF ContainerBox::getRelBoundingRect(const qreal relFrame) {
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


FrameRange ContainerBox::prp_getIdenticalRelRange(const int relFrame) const {
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

FrameRange ContainerBox::getFirstAndLastIdenticalForMotionBlur(
        const int relFrame, const bool takeAncestorsIntoAccount) {
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


bool ContainerBox::relPointInsidePath(const QPointF &relPos) const {
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

int ContainerBox::getContainedBoxesCount() const {
    return mContainedBoxes.count();
}

void ContainerBox::setIsCurrentGroup_k(const bool bT) {
    mIsCurrentGroup = bT;
    setDescendantCurrentGroup(bT);
    if(!bT) {
        if(mContainedBoxes.isEmpty() && mParentGroup) {
            removeFromParent_k();
        }
    }
}

bool ContainerBox::isCurrentGroup() const {
    return mIsCurrentGroup;
}

bool ContainerBox::isDescendantCurrentGroup() const {
    return mIsDescendantCurrentGroup;
}


void ContainerBox::setDescendantCurrentGroup(const bool bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) planScheduleUpdate(Animator::USER_CHANGE);
    if(!mParentGroup) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *ContainerBox::getBoxAtFromAllDescendents(const QPointF &absPos) {
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

void ContainerBox::ungroup_k() {
    //clearBoxesSelection();
    for(auto box : mContainedBoxes) {
        removeContainedBox(box);
        mParentGroup->addContainedBox(box);
    }
    removeFromParent_k();
}

#include "typemenu.h"
void ContainerBox::addActionsToMenu(BoxTypeMenu * const menu) {
    const auto ungroupAction = menu->addPlainAction<ContainerBox>(
                "Ungroup", [](ContainerBox * box) {
        box->ungroup_k();
    });
    ungroupAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    menu->addSeparator();

    menu->addPlainAction<ContainerBox>("Promote to Layer",
                                       [](ContainerBox * box) {
        box->promoteToLayer();
    })->setEnabled(SWT_isGroupBox());

    menu->addPlainAction<ContainerBox>("Demote to Group",
                                       [](ContainerBox * box) {
        box->demoteToGroup();
    })->setEnabled(!SWT_isGroupBox());

    BoundingBox::addActionsToMenu(menu);
}

void ContainerBox::drawPixmapSk(SkCanvas * const canvas,
                                GrContext* const grContext) {
    if(SWT_isGroupBox()) {
        for(const auto& box : mContainedBoxes) {
            if(box->isVisibleAndInVisibleDurationRect())
                box->drawPixmapSk(canvas, grContext);
        }
    } else {
        if(mIsDescendantCurrentGroup) {
            SkPaint paint;
            const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
            paint.setAlpha(static_cast<U8CPU>(intAlpha));
            paint.setBlendMode(mBlendModeSk);
            canvas->saveLayer(nullptr, &paint);
            for(const auto& box : mContainedBoxes) {
                if(box->isVisibleAndInVisibleDurationRect())
                    box->drawPixmapSk(canvas, grContext);
            }
            canvas->restore();
        } else {
            BoundingBox::drawPixmapSk(canvas, grContext);
        }
    }
}

qsptr<BoundingBox> ContainerBox::createLink() {
    return SPtrCreate(InternalLinkGroupBox)(this);
}

void ContainerBox::updateIfUsesProgram(
        const ShaderEffectProgram * const program) const {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndInVisibleDurationRect())
            box->updateIfUsesProgram(program);
    }
    BoundingBox::updateIfUsesProgram(program);
}

void processChildData(BoundingBox * const child,
                      ContainerBoxRenderData * const parentData,
                      const qreal childRelFrame,
                      const qreal absFrame) {
    if(!child->isFrameFVisibleAndInDurationRect(childRelFrame)) return;
    if(child->SWT_isGroupBox()) {
        const auto childGroup = GetAsPtr(child, ContainerBox);
        const auto descs = childGroup->getContainedBoxesList();
        for(const auto& desc : descs) {
            const qreal descRelFrame = desc->prp_absFrameToRelFrameF(absFrame);
            processChildData(desc.get(), parentData, descRelFrame, absFrame);
        }
        return;
    }
    auto boxRenderData = child->getCurrentRenderData(qRound(childRelFrame));
    if(!boxRenderData) {
        boxRenderData = child->createRenderData();
        boxRenderData->fReason = parentData->fReason;
        //boxRenderData->parentIsTarget = false;
        boxRenderData->fUseCustomRelFrame = true;
        boxRenderData->fCustomRelFrame = childRelFrame;
        boxRenderData->scheduleTask();
    }
    //qDebug() << boxRenderData->fBoxStateId;
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;
}

stdsptr<BoundingBoxRenderData> ContainerBox::createRenderData() {
    return SPtrCreate(ContainerBoxRenderData)(this);
}

void ContainerBox::setupRenderData(const qreal relFrame,
                                   BoundingBoxRenderData * const data) {
    if(SWT_isGroupBox()) {
        data->fOpacity = 0;
        if(data->fParentIsTarget && !data->nullifyBeforeProcessing()) {
            nullifyCurrentRenderData(data->fRelFrame);
        }
    } else setupLayerRenderData(relFrame, data);
}

void ContainerBox::setupLayerRenderData(const qreal relFrame,
                                        BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto groupData = GetAsPtr(data, ContainerBoxRenderData);
    groupData->fChildrenRenderData.clear();
    groupData->fOtherGlobalRects.clear();
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(const auto& box : mContainedBoxes) {
        const qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
        processChildData(box.data(), groupData, boxRelFrame, absFrame);
    }
}

void ContainerBox::selectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) continue;
        getParentCanvas()->addBoxToSelection(box.get());
    }
}

void ContainerBox::deselectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) {
            getParentCanvas()->removeBoxFromSelection(box.get());
        }
    }
}

bool ContainerBox::diffsAffectingContainedBoxes(
        const int relFrame1, const int relFrame2) {
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

BoundingBox *ContainerBox::getBoxAt(const QPointF &absPos) {
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

void ContainerBox::anim_setAbsFrame(const int frame) {
    BoundingBox::anim_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    for(const auto& box : mContainedBoxes) {
        box->anim_setAbsFrame(frame);
    }
}

void ContainerBox::addContainedBoxesToSelection(const QRectF &rect) {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
                box->isVisibleAndInVisibleDurationRect()) {
            if(box->isContainedIn(rect)) {
                getParentCanvas()->addBoxToSelection(box.get());
            }
        }
    }
}

void ContainerBox::addContainedBox(const qsptr<BoundingBox>& child) {
    //child->setParent(this);
    addContainedBoxToListAt(mContainedBoxes.count(), child);
}

void ContainerBox::addContainedBoxToListAt(
        const int index,
        const qsptr<BoundingBox>& child) {
    mContainedBoxes.insert(index, GetAsSPtr(child, BoundingBox));
    child->setParentGroup(this);
    connect(child.data(), &BoundingBox::prp_absFrameRangeChanged,
            this, &BoundingBox::prp_afterChangedAbsRange);
    updateContainedBoxIds(index);

    //SWT_addChild(child);
    SWT_addChildAt(
                child.get(), boxIdToAbstractionId(index));
    child->anim_setAbsFrame(anim_getCurrentAbsFrame());

    child->prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        auto internalLinkGroup = GetAsSPtr(box, InternalLinkGroupBox);
        internalLinkGroup->addContainedBoxToListAt(
                    index, child->createLinkForLinkGroup());
    }
}

void ContainerBox::updateContainedBoxIds(const int firstId) {
    updateContainedBoxIds(firstId, mContainedBoxes.length() - 1);
}

void ContainerBox::updateContainedBoxIds(const int firstId,
                                       const int lastId) {
    for(int i = firstId; i <= lastId; i++) {
        mContainedBoxes.at(i)->setZListIndex(i);
    }
}


void ContainerBox::removeAllContainedBoxes() {
    while(mContainedBoxes.count() > 0) {
        removeContainedBox(mContainedBoxes.takeLast());
    }
}

void ContainerBox::removeContainedBoxFromList(const int id) {
    auto box = mContainedBoxes.takeAt(id);
    if(box->SWT_isContainerBox()) {
        auto group = GetAsPtr(box, ContainerBox);
        if(group->isCurrentGroup()) {
            const auto parentCanvas = getParentCanvas();
            if(parentCanvas) {
                parentCanvas->setCurrentGroupParentAsCurrentGroup();
            }
        }
    }

    box->prp_afterWholeInfluenceRangeChanged();
    if(box->isSelected()) box->removeFromSelection();
    disconnect(box.data(), nullptr, this, nullptr);

    updateContainedBoxIds(id);

    SWT_removeChild(box.get());
    box->setParentGroup(nullptr);

    for(const auto& box : mLinkingBoxes) {
        const auto internalLinkGroup = GetAsSPtr(box, InternalLinkGroupBox);
        internalLinkGroup->removeContainedBoxFromList(id);
    }
}

int ContainerBox::getContainedBoxIndex(BoundingBox * const child) {
    for(int i = 0; i < mContainedBoxes.count(); i++) {
        if(mContainedBoxes.at(i) == child) return i;
    }
    return -1;
}

bool ContainerBox::replaceContainedBox(const qsptr<BoundingBox> &replaced,
                                       const qsptr<BoundingBox> &replacer) {
    const int id = getContainedBoxIndex(replaced.get());
    if(id == -1) return false;
    removeContainedBox(replaced);
    addContainedBoxToListAt(id, replacer);
    return true;
}

void ContainerBox::removeContainedBox(const qsptr<BoundingBox>& child) {
    const int index = getContainedBoxIndex(child.get());
    if(index < 0) return;
    child->removeFromSelection();
    removeContainedBoxFromList(index);
    //child->setParent(nullptr);
}

qsptr<BoundingBox> ContainerBox::takeContainedBox_k(const int id) {
    const auto child = mContainedBoxes.at(id);
    removeContainedBox_k(child);
    return child;
}

void ContainerBox::removeContainedBox_k(const qsptr<BoundingBox>& child) {
    removeContainedBox(child);
    if(mContainedBoxes.isEmpty() && mParentGroup) {
        removeFromParent_k();
    }
}

void ContainerBox::increaseContainedBoxZInList(BoundingBox * const child) {
    const int index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedBoxInList(child, index, index + 1);
}

void ContainerBox::decreaseContainedBoxZInList(BoundingBox * const child) {
    const int index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, index - 1);
}

void ContainerBox::bringContainedBoxToEndList(BoundingBox * const child) {
    const int index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedBoxInList(child, index, mContainedBoxes.length() - 1);
}

void ContainerBox::bringContainedBoxToFrontList(BoundingBox * const child) {
    const int index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, 0);
}

void ContainerBox::moveContainedBoxInList(BoundingBox * const child,
                                        const int to) {
    const int from = getContainedBoxIndex(child);
    if(from == -1) return;
    moveContainedBoxInList(child, from, to);
}

void ContainerBox::moveContainedBoxInList(BoundingBox * const child,
                                        const int from, const int to) {
    mContainedBoxes.move(from, to);
    updateContainedBoxIds(qMin(from, to), qMax(from, to));
    SWT_moveChildTo(child, boxIdToAbstractionId(to));
    planScheduleUpdate(Animator::USER_CHANGE);

    prp_afterWholeInfluenceRangeChanged();
}

void ContainerBox::moveContainedBoxBelow(BoundingBox * const boxToMove,
                                       BoundingBox * const below) {
    const int indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

void ContainerBox::moveContainedBoxAbove(BoundingBox * const boxToMove,
                                       BoundingBox * const above) {
    const int indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(above);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

#include "singlewidgetabstraction.h"
void ContainerBox::SWT_setupAbstraction(
        SWT_Abstraction* abstraction,
        const UpdateFuncs &updateFuncs,
        const int visiblePartWidgetId) {
    BoundingBox::SWT_setupAbstraction(abstraction, updateFuncs,
                                             visiblePartWidgetId);

    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto &child = mContainedBoxes.at(i);
        auto abs = child->SWT_abstractionForWidget(updateFuncs,
                                                              visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}

bool ContainerBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                       const bool parentSatisfies,
                                       const bool parentMainTarget) const {
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

void ContainerBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    int nChildBoxes = mContainedBoxes.count();
    target->write(rcConstChar(&nChildBoxes), sizeof(int));
    for(const auto &child : mContainedBoxes) {
        child->writeBoxType(target);
        child->writeBoundingBox(target);
    }
}

#include "smartvectorpath.h"
#include "imagebox.h"
#include "textbox.h"
#include "videobox.h"
#include "particlebox.h"
#include "rectangle.h"
#include "circle.h"
#include "paintbox.h"
#include "imagesequencebox.h"
#include "internallinkcanvas.h"
#include "linkbox.h"

void ContainerBox::readChildBoxes(QIODevice *target) {
    int nChildBoxes;
    target->read(rcChar(&nChildBoxes), sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        qsptr<BoundingBox> box;
        const auto boxType = BoundingBox::sReadBoxType(target);
        if(boxType == TYPE_VECTOR_PATH) {
            box = SPtrCreate(SmartVectorPath)();
        } else if(boxType == TYPE_IMAGE) {
            box = SPtrCreate(ImageBox)();
        } else if(boxType == TYPE_TEXT) {
            box = SPtrCreate(TextBox)();
        } else if(boxType == TYPE_VIDEO) {
            box = SPtrCreate(VideoBox)();
        } else if(boxType == TYPE_PARTICLES) {
            box = SPtrCreate(ParticleBox)();
        } else if(boxType == TYPE_RECTANGLE) {
            box = SPtrCreate(Rectangle)();
        } else if(boxType == TYPE_CIRCLE) {
            box = SPtrCreate(Circle)();
        } else if(boxType == TYPE_LAYER) {
            box = SPtrCreate(ContainerBox)(TYPE_LAYER);
        } else if(boxType == TYPE_GROUP) {
            box = SPtrCreate(ContainerBox)(TYPE_GROUP);
        } else if(boxType == TYPE_PAINT) {
            box = SPtrCreate(PaintBox)();
        } else if(boxType == TYPE_IMAGESQUENCE) {
            box = SPtrCreate(ImageSequenceBox)();
        } else if(boxType == TYPE_INTERNAL_LINK) {
            box = SPtrCreate(InternalLinkBox)(nullptr);
        } else if(boxType == TYPE_INTERNAL_LINK_GROUP) {
            box = SPtrCreate(InternalLinkGroupBox)(nullptr);
        } else if(boxType == TYPE_EXTERNAL_LINK) {
            box = SPtrCreate(ExternalLinkBox)();
        } else if(boxType == TYPE_INTERNAL_LINK_CANVAS) {
            box = SPtrCreate(InternalLinkCanvas)(nullptr);
        } else {
            RuntimeThrow("Invalid box type '" + std::to_string(boxType) + "'");
        }

        box->readBoundingBox(target);
        addContainedBox(box);
    }
}

void ContainerBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    readChildBoxes(target);
}
