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
           eMimeData::sHasType<BoundingBox>(data);
}

bool ContainerBox::SWT_dropIntoSupport(const int index, const QMimeData * const data) {
    if(eMimeData::sHasType<BoundingBox>(data)) {
        return index >= ca_getNumberOfChildren();
    }
    return false;
}

bool ContainerBox::SWT_drop(const QMimeData * const data) {
    if(BoundingBox::SWT_drop(data)) return true;
    if(mPathEffectsAnimators->SWT_dropSupport(data))
        return mPathEffectsAnimators->SWT_drop(data);
    if(eMimeData::sHasType<BoundingBox>(data))
        return SWT_dropInto(ca_getNumberOfChildren(), data);
    return false;
}

bool ContainerBox::SWT_dropInto(const int index, const QMimeData * const data) {
    const auto eData = static_cast<const eMimeData*>(data);
    const auto bData = static_cast<const eDraggedObjects*>(eData);
    for(int i = 0; i < bData->count(); i++) {
        const auto iObj = bData->getObject<BoundingBox>(i);
        insertContainedBox(index + i - ca_getNumberOfChildren(),
                           iObj->ref<BoundingBox>());
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

const ConnContextObjList<qsptr<BoundingBox>> &ContainerBox::getContainedBoxes() const {
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
            static_cast<PathBox*>(box.get())->setPathsOutdated();
            box->planScheduleUpdate(reason);
        } else if(box->SWT_isContainerBox()) {
            static_cast<ContainerBox*>(box.get())->updateAllChildPathBoxes(reason);
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
    for(auto box : mContainedBoxes) {
        removeContainedBox(box);
        mParentGroup->addContainedBox(box);
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
            processChildData(desc.get(), parentData, descRelFrame, absFrame);
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
        processChildData(box.get(), groupData, boxRelFrame, absFrame);
    }
}

void ContainerBox::selectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) continue;
        mParentScene->addBoxToSelection(box.get());
    }
}

void ContainerBox::deselectAllBoxesFromBoxesGroup() {
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) {
            mParentScene->removeBoxFromSelection(box.get());
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
                boxAtPos = box.get();
                break;
            }
        }
    }
    return boxAtPos;
}

bool ContainerBox::unboundChildren() const {
    if(mParentGroup) {
        if(ContainerBox::SWT_isGroupBox())
            return mParentGroup->unboundChildren();
        return mRasterEffectsAnimators->unbound() ||
               mParentGroup->unboundChildren();
    }
    return false;
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
                mParentScene->addBoxToSelection(box.get());
            }
        }
    }
}

void ContainerBox::addContainedBox(const qsptr<BoundingBox>& child) {
    insertContainedBox(0, child);
}

void ContainerBox::insertContainedBox(const int id,
                                      const qsptr<BoundingBox>& child) {
    if(child->getParentGroup() == this) {
        const int cId = mContainedBoxes.indexOf(child);
        moveContainedBoxInList(child.get(), cId, (cId < id ? id - 1 : id));
        return;
    }
    child->removeFromParent_k();
    auto& connCtx = mContainedBoxes.insertObj(id, child);
    child->setParentGroup(this);
    connCtx << connect(child.data(), &BoundingBox::prp_absFrameRangeChanged,
                       this, &BoundingBox::prp_afterChangedAbsRange);
    updateContainedBoxIds(id);

    SWT_addChildAt(child.get(), boxIdToAbstractionId(id));
    child->anim_setAbsFrame(anim_getCurrentAbsFrame());

    child->prp_afterWholeInfluenceRangeChanged();

    for(const auto& box : mLinkingBoxes) {
        auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
        internalLinkGroup->insertContainedBox(
                    id, child->createLinkForLinkGroup());
    }
}

void ContainerBox::updateContainedBoxIds(const int firstId) {
    updateContainedBoxIds(firstId, mContainedBoxes.count() - 1);
}

void ContainerBox::updateContainedBoxIds(const int firstId, const int lastId) {
    for(int i = firstId; i <= lastId; i++) {
        mContainedBoxes.at(i)->setZListIndex(i);
    }
}

void ContainerBox::removeAllContainedBoxes() {
    while(mContainedBoxes.count() > 0) {
        removeContainedBox(mContainedBoxes.takeObjLast());
    }
}

void ContainerBox::removeContainedBoxFromList(const int id) {
    const auto box = mContainedBoxes.takeObjAt(id);
    if(box->SWT_isContainerBox()) {
        const auto group = static_cast<ContainerBox*>(box.get());
        if(group->isCurrentGroup() && mParentScene) {
            mParentScene->setCurrentGroupParentAsCurrentGroup();
        }
    }

    SWT_removeChild(box.get());
    box->setParentGroup(nullptr);
    updateContainedBoxIds(id);

    for(const auto& box : mLinkingBoxes) {
        const auto internalLinkGroup = box->ref<InternalLinkGroupBox>();
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
    insertContainedBox(id, replacer);
    return true;
}

void ContainerBox::removeContainedBox(const qsptr<BoundingBox>& child) {
    const int index = getContainedBoxIndex(child.get());
    if(index < 0) return;
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
    moveContainedBoxInList(child, index, mContainedBoxes.count() - 1);
}

void ContainerBox::bringContainedBoxToFrontList(BoundingBox * const child) {
    const int index = getContainedBoxIndex(child);
    if(index == 0) return;
    moveContainedBoxInList(child, index, 0);
}

void ContainerBox::moveContainedBoxInList(BoundingBox * const child, const int to) {
    const int from = getContainedBoxIndex(child);
    if(from == -1) return;
    moveContainedBoxInList(child, from, to);
}

void ContainerBox::moveContainedBoxInList(BoundingBox * const child,
                                          const int from, const int to) {
    const int boundTo = qBound(0, to, mContainedBoxes.count() - 1);
    mContainedBoxes.moveObj(from, boundTo);
    updateContainedBoxIds(qMin(from, boundTo), qMax(from, boundTo));
    SWT_moveChildTo(child, boxIdToAbstractionId(boundTo));
    planScheduleUpdate(UpdateReason::userChange);

    prp_afterWholeInfluenceRangeChanged();
}

void ContainerBox::moveContainedBoxBelow(BoundingBox * const boxToMove,
                                         BoundingBox * const below) {
    const int indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(below);
    if(indexFrom > indexTo) indexTo++;
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

void ContainerBox::moveContainedBoxAbove(BoundingBox * const boxToMove,
                                         BoundingBox * const above) {
    const int indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(above);
    if(indexFrom < indexTo) indexTo--;
    moveContainedBoxInList(boxToMove, indexFrom, indexTo);
}

#include "singlewidgetabstraction.h"
void ContainerBox::SWT_setupAbstraction(SWT_Abstraction* abstraction,
                                        const UpdateFuncs &updateFuncs,
                                        const int visiblePartWidgetId) {
    BoundingBox::SWT_setupAbstraction(abstraction, updateFuncs,
                                      visiblePartWidgetId);

    for(const auto& box : mContainedBoxes) {
        auto abs = box->SWT_abstractionForWidget(updateFuncs,
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
    const int nChildBoxes = mContainedBoxes.count();
    target->write(rcConstChar(&nChildBoxes), sizeof(int));
    for(const auto &child : mContainedBoxes) {
        child->writeIdentifier(target);
        child->writeBoundingBox(target);
    }
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

void ContainerBox::readChildBoxes(QIODevice * const src) {
    int nChildBoxes;
    src->read(rcChar(&nChildBoxes), sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        const auto box = readIdCreateBox(src);
        box->readBoundingBox(src);
        addContainedBox(box);
    }
}

void ContainerBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    readChildBoxes(target);
}
