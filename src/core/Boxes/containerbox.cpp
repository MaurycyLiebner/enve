#include "containerbox.h"
#include "Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "canvas.h"
#include "internallinkgroupbox.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "PropertyUpdaters/groupallpathsupdater.h"
#include "textbox.h"
#include "Animators/rastereffectanimators.h"

ContainerBox::ContainerBox(const eBoxType type) :
    BoundingBox(type) {
    if(type == eBoxType::TYPE_GROUP) prp_setName("Group");
    else if(type == eBoxType::TYPE_LAYER) prp_setName("Layer");
    iniPathEffects();
}

bool ContainerBox::SWT_dropSupport(const QMimeData * const data) {
    return BoundingBox::SWT_dropSupport(data) ||
           mPathEffectsAnimators->SWT_dropSupport(data) ||
           eMimeData::sHasType<eBoxOrSound>(data);
}

bool ContainerBox::SWT_dropIntoSupport(const int index, const QMimeData * const data) {
    if(eMimeData::sHasType<eBoxOrSound>(data)) {
        return index >= ca_getNumberOfChildren();
    }
    return false;
}

bool ContainerBox::SWT_drop(const QMimeData * const data) {
    if(BoundingBox::SWT_drop(data)) return true;
    if(mPathEffectsAnimators->SWT_dropSupport(data))
        return mPathEffectsAnimators->SWT_drop(data);
    if(eMimeData::sHasType<eBoxOrSound>(data))
        return SWT_dropInto(ca_getNumberOfChildren(), data);
    return false;
}

bool ContainerBox::SWT_dropInto(const int index, const QMimeData * const data) {
    const auto eData = static_cast<const eMimeData*>(data);
    const auto bData = static_cast<const eDraggedObjects*>(eData);
    for(int i = 0; i < bData->count(); i++) {
        const auto iObj = bData->getObject<eBoxOrSound>(i);
        insertContained(index + i - ca_getNumberOfChildren(),
                        iObj->ref<eBoxOrSound>());
    }
    return true;
}

void ContainerBox::iniPathEffects() {
    mPathEffectsAnimators =
            enve::make_shared<PathEffectAnimators>();
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setOwnUpdater(
                enve::make_shared<GroupAllPathsUpdater>(this));
    ca_addChild(mPathEffectsAnimators);

    mFillPathEffectsAnimators =
            enve::make_shared<PathEffectAnimators>();
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setOwnUpdater(
                enve::make_shared<GroupAllPathsUpdater>(this));
    ca_addChild(mFillPathEffectsAnimators);

    mOutlineBasePathEffectsAnimators =
            enve::make_shared<PathEffectAnimators>();
    mOutlineBasePathEffectsAnimators->prp_setName("outline base effects");
    mOutlineBasePathEffectsAnimators->prp_setOwnUpdater(
                enve::make_shared<GroupAllPathsUpdater>(this));
    ca_addChild(mOutlineBasePathEffectsAnimators);

    mOutlinePathEffectsAnimators =
            enve::make_shared<PathEffectAnimators>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setOwnUpdater(
                enve::make_shared<GroupAllPathsUpdater>(this));
    ca_addChild(mOutlinePathEffectsAnimators);
}

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

const QList<BoundingBox*> &ContainerBox::getContainedBoxes() const {
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
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::addFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::addOutlineBasePathEffect(const qsptr<PathEffect>& effect) {
    mOutlineBasePathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->addChild(effect);
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::removePathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->removeChild(effect);
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::removeFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->removeChild(effect);
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::removeOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->removeChild(effect);
    updateAllChildPathBoxes(UpdateReason::userChange);
}

void ContainerBox::updateAllChildPathBoxes(const UpdateReason reason) {
    for(const auto& box : mContainedBoxes) {
        if(box->SWT_isPathBox()) {
            static_cast<PathBox*>(box)->setPathsOutdated();
            box->planScheduleUpdate(reason);
        } else if(box->SWT_isContainerBox()) {
            static_cast<ContainerBox*>(box)->updateAllChildPathBoxes(reason);
        }
    }
}

void ContainerBox::forcedMarginMeaningfulChange() {
    const auto thisMargin = mRasterEffectsAnimators->getMaxForcedMargin();
    const auto inheritedMargin =
            mParentGroup ? mParentGroup->mForcedMargin : QMargins();
    mForcedMargin.setTop(qMax(inheritedMargin.top(), thisMargin.top()));
    mForcedMargin.setLeft(qMax(inheritedMargin.left(), thisMargin.left()));
    mForcedMargin.setBottom(qMax(inheritedMargin.bottom(), thisMargin.bottom()));
    mForcedMargin.setRight(qMax(inheritedMargin.right(), thisMargin.right()));
    for(const auto& box : mContainedBoxes) {
        if(box->SWT_isContainerBox()) {
            const auto cont = static_cast<ContainerBox*>(box);
            cont->forcedMarginMeaningfulChange();
        } else box->planScheduleUpdate(UpdateReason::userChange);
    }
}

QRect ContainerBox::currentGlobalBounds() const {
    if(!mParentScene) return QRect();
    const auto sceneBounds = mParentScene->getCurrentBounds();
    return sceneBounds.adjusted(-mForcedMargin.left(),
                                -mForcedMargin.top(),
                                mForcedMargin.right(),
                                mForcedMargin.bottom());
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
}

void ContainerBox::filterOutlineBasePath(const qreal relFrame,
                                         SkPath * const srcDstPath) {
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

void ContainerBox::queChildScheduledTasks() {
    for(const auto &child : mContainedBoxes)
        child->queScheduledTasks();
}

void ContainerBox::queScheduledTasks() {
    queChildScheduledTasks();
    if(mSchedulePlanned && SWT_isGroupBox()) updateRelBoundingRect();
    BoundingBox::queScheduledTasks();
}

void ContainerBox::promoteToLayer() {
    if(!SWT_isGroupBox()) return;
    if(!SWT_isLinkBox()) mType = TYPE_LAYER;
    if(prp_mName.contains("Group")) {
        auto newName  = prp_mName;
        newName.replace("Group", "Layer");
        prp_setName(newName);
    }
    mRasterEffectsAnimators->SWT_enable();
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        static_cast<ContainerBox*>(box)->promoteToLayer();
    }
}

void ContainerBox::demoteToGroup() {
    if(!SWT_isLayerBox()) return;
    if(!SWT_isLinkBox()) mType = TYPE_GROUP;
    if(prp_mName.contains("Layer")) {
        auto newName  = prp_mName;
        newName.replace("Layer", "Group");
        prp_setName(newName);
    }
    mRasterEffectsAnimators->SWT_disable();
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        static_cast<ContainerBox*>(box)->demoteToGroup();
    }
}

void ContainerBox::updateAllBoxes(const UpdateReason reason) {
    for(const auto &child : mContainedBoxes) {
        child->updateAllBoxes(reason);
    }
    planScheduleUpdate(reason);
}

void ContainerBox::prp_afterFrameShiftChanged(const FrameRange &oldAbsRange,
                                              const FrameRange &newAbsRange) {
    ComplexAnimator::prp_afterFrameShiftChanged(oldAbsRange, newAbsRange);
    const int thisShift = prp_getTotalFrameShift();
    for(const auto &child : mContainedBoxes)
        child->prp_setInheritedFrameShift(thisShift, this);
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

void ContainerBox::updateRelBoundingRect() {
    SkPath boundingPaths;
    for(const auto &child : mContainedBoxes) {
        if(child->isVisibleAndInVisibleDurationRect()) {
            SkPath childPath;
            const auto childRel = child->getRelBoundingRect();
            childPath.addRect(toSkRect(childRel));

            const auto childRelTrans = child->getRelativeTransformAtCurrentFrame();
            childPath.transform(toSkMatrix(childRelTrans));

            boundingPaths.addPath(childPath);
        }
    }
    setRelBoundingRect(toQRectF(boundingPaths.computeTightBounds()));
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
            if(relFrame > mDurationRectangle->getMaxRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinRelFrame()) {
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
    if(mRelRect.contains(relPos)) {
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

void ContainerBox::updateContainedBoxes() {
    mContainedBoxes.clear();
    for(const auto& child : mContained) {
        if(child->SWT_isBoundingBox()) {
            mContainedBoxes << static_cast<BoundingBox*>(child.get());
        }
    }
}

bool ContainerBox::isDescendantCurrentGroup() const {
    return mIsDescendantCurrentGroup;
}

void ContainerBox::setDescendantCurrentGroup(const bool bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) planScheduleUpdate(UpdateReason::userChange);
    if(!mParentGroup) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *ContainerBox::getBoxAtFromAllDescendents(const QPointF &absPos) {
    BoundingBox* boxAtPos = nullptr;
    for(const auto& box : mContainedBoxes) {
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
    for(auto box : mContained) {
        removeContained(box);
        mParentGroup->addContained(box);
    }
    removeFromParent_k();
}

#include "patheffectsmenu.h"
void ContainerBox::setupCanvasMenu(PropertyMenu * const menu) {
    menu->addSection("Layer & Group");

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
    })->setDisabled(SWT_isGroupBox());

    BoundingBox::setupCanvasMenu(menu);
    PathEffectsMenu::addPathEffectsToActionMenu(menu);
}

void ContainerBox::drawContained(SkCanvas * const canvas) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto& box = mContainedBoxes.at(i);
        if(box->isVisibleAndInVisibleDurationRect())
            box->drawPixmapSk(canvas);
    }
}

void ContainerBox::drawPixmapSk(SkCanvas * const canvas) {
    if(SWT_isGroupBox()) return drawContained(canvas);
    if(mIsDescendantCurrentGroup) {
        SkPaint paint;
        const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
        paint.setAlpha(static_cast<U8CPU>(intAlpha));
        paint.setBlendMode(mBlendModeSk);
        canvas->saveLayer(nullptr, &paint);
        drawContained(canvas);
        canvas->restore();
    } else BoundingBox::drawPixmapSk(canvas);
}

qsptr<BoundingBox> ContainerBox::createLink() {
    return enve::make_shared<InternalLinkGroupBox>(this);
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
        const auto childGroup = static_cast<ContainerBox*>(child);
        const auto& descs = childGroup->getContainedBoxes();
        for(int i = descs.count() - 1; i >= 0; i--) {
            const auto& desc = descs.at(i);
            const qreal descRelFrame = desc->prp_absFrameToRelFrameF(absFrame);
            processChildData(desc, parentData, descRelFrame, absFrame);
        }
        return;
    }
    auto boxRenderData = child->getCurrentRenderData(childRelFrame);
    if(!boxRenderData) {
        boxRenderData = child->createRenderData();
        boxRenderData->fReason = parentData->fReason;
        boxRenderData->fRelFrame = childRelFrame;
        TaskScheduler::sGetInstance()->queCPUTask(boxRenderData);
    }
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;
}

stdsptr<BoxRenderData> ContainerBox::createRenderData() {
    return enve::make_shared<ContainerBoxRenderData>(this);
}

void ContainerBox::setupRenderData(const qreal relFrame,
                                   BoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto groupData = static_cast<ContainerBoxRenderData*>(data);
    groupData->fChildrenRenderData.clear();
    groupData->fOtherGlobalRects.clear();
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto& box = mContainedBoxes.at(i);
        const qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
        processChildData(box, groupData, boxRelFrame, absFrame);
    }
}

void ContainerBox::selectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) continue;
        mParentScene->addBoxToSelection(box);
    }
}

void ContainerBox::deselectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) {
            mParentScene->removeBoxFromSelection(box);
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

    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
           box->isVisibleAndInVisibleDurationRect()) {
            if(box->absPointInsidePath(absPos)) {
                boxAtPos = box;
                break;
            }
        }
    }
    return boxAtPos;
}

void ContainerBox::anim_setAbsFrame(const int frame) {
    BoundingBox::anim_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    for(const auto& cont : mContained)
        cont->anim_setAbsFrame(frame);
}

void ContainerBox::addContainedBoxesToSelection(const QRectF &rect) {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
                box->isVisibleAndInVisibleDurationRect()) {
            if(box->isContainedIn(rect)) {
                mParentScene->addBoxToSelection(box);
            }
        }
    }
}

void ContainerBox::addContained(const qsptr<eBoxOrSound>& child) {
    insertContained(0, child);
}

void ContainerBox::insertContained(const int id,
                                   const qsptr<eBoxOrSound>& child) {
    if(child->getParentGroup() == this) {
        const int cId = mContained.indexOf(child);
        moveContainedInList(child.get(), cId, (cId < id ? id - 1 : id));
        return;
    }
    child->removeFromParent_k();
    auto& connCtx = mContained.insertObj(id, child);
    updateContainedBoxes();
    child->setParentGroup(this);

    updateContainedIds(id);
    SWT_addChildAt(child.get(), containedIdToAbstractionId(id));

    if(child->SWT_isBoundingBox()) {
        connCtx << connect(child.data(), &Property::prp_absFrameRangeChanged,
                           this, &Property::prp_afterChangedAbsRange);
        const auto cBox = static_cast<BoundingBox*>(child.get());
        for(const auto& box : mLinkingBoxes) {
            const auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
            internalLinkGroup->insertContained(id, cBox->createLinkForLinkGroup());
        }
    }
    child->anim_setAbsFrame(anim_getCurrentAbsFrame());
    child->prp_afterWholeInfluenceRangeChanged();
}

void ContainerBox::updateContainedIds(const int firstId) {
    updateContainedIds(firstId, mContained.count() - 1);
}

void ContainerBox::updateContainedIds(const int firstId, const int lastId) {
    for(int i = firstId; i <= lastId; i++) mContained.at(i)->setZListIndex(i);
}

void ContainerBox::removeAllContained() {
    while(mContained.count() > 0) removeContained(mContained.last());
}

void ContainerBox::removeContainedFromList(const int id) {
    const auto child = mContained.takeObjAt(id);
    updateContainedBoxes();
    if(child->SWT_isContainerBox()) {
        const auto group = static_cast<ContainerBox*>(child.get());
        if(group->isCurrentGroup() && mParentScene) {
            mParentScene->setCurrentGroupParentAsCurrentGroup();
        }
    }

    SWT_removeChild(child.get());
    child->setParentGroup(nullptr);
    updateContainedIds(id);

    if(child->SWT_isBoundingBox()) {
        for(const auto& box : mLinkingBoxes) {
            const auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
            internalLinkGroup->removeContainedFromList(id);
        }
    }
}

int ContainerBox::getContainedIndex(eBoxOrSound * const child) {
    for(int i = 0; i < mContained.count(); i++) {
        if(mContained.at(i) == child) return i;
    }
    return -1;
}

bool ContainerBox::replaceContained(const qsptr<eBoxOrSound> &replaced,
                                    const qsptr<eBoxOrSound> &replacer) {
    const int id = getContainedIndex(replaced.get());
    if(id == -1) return false;
    removeContained(replaced);
    insertContained(id, replacer);
    return true;
}

void ContainerBox::removeContained(const qsptr<eBoxOrSound>& child) {
    const int index = getContainedIndex(child.get());
    if(index < 0) return;
    removeContainedFromList(index);
    //child->setParent(nullptr);
}

qsptr<eBoxOrSound> ContainerBox::takeContained_k(const int id) {
    const auto child = mContained.at(id);
    removeContained_k(child);
    return child;
}

void ContainerBox::removeContained_k(const qsptr<eBoxOrSound> &child) {
    removeContained(child);
    if(mContainedBoxes.isEmpty() && mParentGroup) {
        removeFromParent_k();
    }
}

void ContainerBox::increaseContainedZInList(eBoxOrSound * const child) {
    const int index = getContainedIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedInList(child, index, index + 1);
}

void ContainerBox::decreaseContainedZInList(eBoxOrSound * const child) {
    const int index = getContainedIndex(child);
    if(index == 0) return;
    moveContainedInList(child, index, index - 1);
}

void ContainerBox::bringContainedToEndList(eBoxOrSound * const child) {
    const int index = getContainedIndex(child);
    if(index == mContainedBoxes.count() - 1) return;
    moveContainedInList(child, index, mContainedBoxes.count() - 1);
}

void ContainerBox::bringContainedToFrontList(eBoxOrSound * const child) {
    const int index = getContainedIndex(child);
    if(index == 0) return;
    moveContainedInList(child, index, 0);
}

void ContainerBox::moveContainedInList(eBoxOrSound * const child, const int to) {
    const int from = getContainedIndex(child);
    if(from == -1) return;
    moveContainedInList(child, from, to);
}

void ContainerBox::moveContainedInList(eBoxOrSound * const child,
                                       const int from, const int to) {
    const int boundTo = qBound(0, to, mContained.count() - 1);
    mContained.moveObj(from, boundTo);
    updateContainedBoxes();
    updateContainedIds(qMin(from, boundTo), qMax(from, boundTo));
    SWT_moveChildTo(child, containedIdToAbstractionId(boundTo));
    planScheduleUpdate(UpdateReason::userChange);

    prp_afterWholeInfluenceRangeChanged();
}

void ContainerBox::moveContainedBelow(eBoxOrSound * const boxToMove,
                                      eBoxOrSound * const below) {
    const int indexFrom = getContainedIndex(boxToMove);
    int indexTo = getContainedIndex(below);
    if(indexFrom > indexTo) indexTo++;
    moveContainedInList(boxToMove, indexFrom, indexTo);
}

void ContainerBox::moveContainedAbove(eBoxOrSound * const boxToMove,
                                      eBoxOrSound * const above) {
    const int indexFrom = getContainedIndex(boxToMove);
    int indexTo = getContainedIndex(above);
    if(indexFrom < indexTo) indexTo--;
    moveContainedInList(boxToMove, indexFrom, indexTo);
}

#include "singlewidgetabstraction.h"
void ContainerBox::SWT_setupAbstraction(SWT_Abstraction* abstraction,
                                        const UpdateFuncs &updateFuncs,
                                        const int visiblePartWidgetId) {
    BoundingBox::SWT_setupAbstraction(abstraction, updateFuncs,
                                      visiblePartWidgetId);

    for(const auto& cont : mContained) {
        auto abs = cont->SWT_abstractionForWidget(updateFuncs, visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}

bool ContainerBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                       const bool parentSatisfies,
                                       const bool parentMainTarget) const {
    const SWT_BoxRule &rule = rules.fRule;
    const bool bbVisible = BoundingBox::SWT_shouldBeVisible(rules,
                                                            parentSatisfies,
                                                            parentMainTarget);
    if(rule == SWT_BR_SELECTED) return bbVisible && !isCurrentGroup();
    return bbVisible;
}

void ContainerBox::writeAllContained(QIODevice * const dst) {
    const int nCont = mContained.count();
    dst->write(rcConstChar(&nCont), sizeof(int));
    for(int i = nCont - 1; i >= 0; i--) {
        const auto &child = mContained.at(i);
        const bool isBox = child->SWT_isBoundingBox();
        dst->write(rcConstChar(&isBox), sizeof(bool));
        if(isBox) {
            const auto box = static_cast<BoundingBox*>(child.get());
            box->writeIdentifier(dst);
            box->writeBoundingBox(dst);
        } else {
            Q_ASSERT(child->SWT_isSingleSound());
            child->writeProperty(dst);
        }
        gWritePos(dst);
    }
}

void ContainerBox::writeBoundingBox(QIODevice * const dst) {
    BoundingBox::writeBoundingBox(dst);
    gWritePos(dst);
    writeAllContained(dst);
}

#include "smartvectorpath.h"
#include "imagebox.h"
#include "textbox.h"
#include "videobox.h"
#include "rectangle.h"
#include "circle.h"
#include "paintbox.h"
#include "imagesequencebox.h"
#include "internallinkcanvas.h"
#include "linkbox.h"
#include "customboxcreator.h"

qsptr<BoundingBox> readIdCreateBox(QIODevice * const src) {
    eBoxType type;
    src->read(rcChar(&type), sizeof(eBoxType));
    switch(type) {
        case(eBoxType::TYPE_VECTOR_PATH):
            return enve::make_shared<SmartVectorPath>();
        case(eBoxType::TYPE_IMAGE):
            return enve::make_shared<ImageBox>();
        case(eBoxType::TYPE_TEXT):
            return enve::make_shared<TextBox>();
        case(eBoxType::TYPE_VIDEO):
            return enve::make_shared<VideoBox>();
        case(eBoxType::TYPE_RECTANGLE):
            return enve::make_shared<Rectangle>();
        case(eBoxType::TYPE_CIRCLE):
            return enve::make_shared<Circle>();
        case(eBoxType::TYPE_LAYER):
            return enve::make_shared<ContainerBox>(TYPE_LAYER);
        case(eBoxType::TYPE_GROUP):
            return enve::make_shared<ContainerBox>(TYPE_GROUP);
        case(eBoxType::TYPE_PAINT):
            return enve::make_shared<PaintBox>();
        case(eBoxType::TYPE_IMAGESQUENCE):
            return enve::make_shared<ImageSequenceBox>();
        case(eBoxType::TYPE_INTERNAL_LINK):
            return enve::make_shared<InternalLinkBox>(nullptr);
        case(eBoxType::TYPE_INTERNAL_LINK_GROUP):
            return enve::make_shared<InternalLinkGroupBox>(nullptr);
        case(eBoxType::TYPE_EXTERNAL_LINK):
            return enve::make_shared<ExternalLinkBox>();
        case(eBoxType::TYPE_INTERNAL_LINK_CANVAS):
            return enve::make_shared<InternalLinkCanvas>(nullptr);
        case(eBoxType::TYPE_CUSTOM): {
            const auto id = CustomIdentifier::sRead(src);
            return CustomBoxCreator::sCreateForIdentifier(id);
        } default: RuntimeThrow("Invalid box type '" + std::to_string(type) + "'");
    }
}

void ContainerBox::readContained(QIODevice * const src) {
    bool isBox;
    src->read(rcChar(&isBox), sizeof(bool));
    if(isBox) {
        const auto box = readIdCreateBox(src);
        box->readBoundingBox(src);
        addContained(box);
    } else {
        const auto sound = enve::make_shared<SingleSound>();
        sound->readProperty(src);
        addContained(sound);
    }
    gReadPos(src);
}

void ContainerBox::readAllContained(QIODevice * const src) {
    int nCont;
    src->read(rcChar(&nCont), sizeof(int));
    for(int i = 0; i < nCont; i++) {
        try {
            readContained(src);
        } catch(...) {
            RuntimeThrow("Error reading contained " + QString::number(i));
        }
    }
}

void ContainerBox::readBoundingBox(QIODevice * const src) {
    BoundingBox::readBoundingBox(src);
    gReadPos(src, "Error reading ContainerBox basic properties");
    readAllContained(src);
}
