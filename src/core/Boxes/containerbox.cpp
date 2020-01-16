// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "containerbox.h"
#include "Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "canvas.h"
#include "internallinkgroupbox.h"
#include "PathEffects/patheffectcollection.h"
#include "PathEffects/patheffect.h"
#include "textbox.h"
#include "RasterEffects/rastereffectcollection.h"
#include "Sound/singlesound.h"
#include "actions.h"
#include "externallinkbox.h"

ContainerBox::ContainerBox(const eBoxType type) :
    BoxWithPathEffects(type) {
    if(type == eBoxType::group) prp_setName("Group");
    else if(type == eBoxType::layer) prp_setName("Layer");
    connect(mRasterEffectsAnimators.get(),
            &RasterEffectCollection::forcedMarginChanged,
            this, &ContainerBox::forcedMarginMeaningfulChange);
    iniPathEffects();
}

bool ContainerBox::SWT_dropSupport(const QMimeData * const data) {
    return BoxWithPathEffects::SWT_dropSupport(data) ||
           eMimeData::sHasType<eBoxOrSound>(data) ||
           data->hasUrls();
}

bool ContainerBox::SWT_dropIntoSupport(const int index, const QMimeData * const data) {
    if(eMimeData::sHasType<eBoxOrSound>(data)) {
        return index >= ca_getNumberOfChildren();
    }
    return data->hasUrls();
}

bool ContainerBox::SWT_drop(const QMimeData * const data) {
    if(BoxWithPathEffects::SWT_drop(data)) return true;
    if(eMimeData::sHasType<eBoxOrSound>(data) ||
       data->hasUrls())
        return SWT_dropInto(ca_getNumberOfChildren(), data);
    return false;
}

bool ContainerBox::SWT_dropInto(const int index, const QMimeData * const data) {
    if(eMimeData::sHasType<eBoxOrSound>(data)) {
        const auto eData = static_cast<const eMimeData*>(data);
        const auto bData = static_cast<const eDraggedObjects*>(eData);
        int dropId = index;
        for(int i = 0; i < bData->count(); i++) {
            const auto iObj = bData->getObject<eBoxOrSound>(i);
            if(iObj->SWT_isContainerBox()) {
                const auto box = static_cast<BoundingBox*>(iObj);
                if(box == this) continue;
                if(isAncestor(box)) continue;
            }
            insertContained((dropId++) - ca_getNumberOfChildren(),
                            iObj->ref<eBoxOrSound>());
        }
        return true;
    } else if(data->hasUrls()) {
        const auto urls = data->urls();
        int dropId = index;
        for(const auto& url : urls) {
            Actions::sInstance->importFile(url.path(), this,
                                           (dropId++) - ca_getNumberOfChildren());
        }
        return true;
    }
    return false;
}

void ContainerBox::iniPathEffects() {
    connect(mPathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
         updateAllChildPaths(reason, &PathBox::setPathsOutdated);
    });

    connect(mFillPathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
         updateAllChildPaths(reason, &PathBox::setFillPathOutdated);
    });

    connect(mOutlineBasePathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
         updateAllChildPaths(reason, &PathBox::setOutlinePathOutdated);
    });

    connect(mOutlinePathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
         updateAllChildPaths(reason, &PathBox::setOutlinePathOutdated);
    });
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

void ContainerBox::setStrokeBrush(SimpleBrushWrapper * const brush) {
    for(const auto& box : mContainedBoxes) {
        box->setStrokeBrush(brush);
    }
}

void ContainerBox::applyStrokeBrushWidthAction(const SegAction& action) {
    for(const auto& box : mContainedBoxes) {
        box->applyStrokeBrushWidthAction(action);
    }
}

void ContainerBox::applyStrokeBrushPressureAction(const SegAction& action) {
    for(const auto& box : mContainedBoxes) {
        box->applyStrokeBrushPressureAction(action);
    }
}

void ContainerBox::applyStrokeBrushSpacingAction(const SegAction& action) {
    for(const auto& box : mContainedBoxes) {
        box->applyStrokeBrushSpacingAction(action);
    }
}

void ContainerBox::applyStrokeBrushTimeAction(const SegAction& action) {
    for(const auto& box : mContainedBoxes) {
        box->applyStrokeBrushTimeAction(action);
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

void ContainerBox::updateAllChildPaths(const UpdateReason reason,
                                       void (PathBox::*func)(const UpdateReason)) {
    for(const auto& box : mContainedBoxes) {
        if(box->SWT_isPathBox()) {
            (static_cast<PathBox*>(box)->*func)(reason);
        } else if(box->SWT_isContainerBox()) {
            static_cast<ContainerBox*>(box)->updateAllChildPaths(reason, func);
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
        } else box->planUpdate(UpdateReason::userChange);
    }
}

QRect ContainerBox::currentGlobalBounds() const {
    const auto pScene = getParentScene();
    if(!pScene) return QRect();
    const auto sceneBounds = pScene->getCurrentBounds();
    return sceneBounds.adjusted(-mForcedMargin.left(),
                                -mForcedMargin.top(),
                                mForcedMargin.right(),
                                mForcedMargin.bottom());
}

void ContainerBox::queChildrenTasks() {
    for(const auto &child : mContainedBoxes)
        child->queTasks();
}

void ContainerBox::queTasks() {
    queChildrenTasks();
    if(getUpdatePlanned() && SWT_isGroupBox())
        updateRelBoundingRect();
    else BoundingBox::queTasks();
}

void ContainerBox::promoteToLayer() {
    if(!SWT_isGroupBox()) return;
    if(!SWT_isLinkBox()) mType = eBoxType::layer;
    if(prp_getName().contains("Group")) {
        auto newName  = prp_getName();
        newName.replace("Group", "Layer");
        prp_setName(newName);
    }
    mRasterEffectsAnimators->SWT_enable();
    prp_afterWholeInfluenceRangeChanged();

    const auto& linkingBoxes = getLinkingBoxes();
    for(const auto& box : linkingBoxes) {
        static_cast<ContainerBox*>(box)->promoteToLayer();
    }
}

void ContainerBox::demoteToGroup() {
    if(!SWT_isLayerBox()) return;
    if(!SWT_isLinkBox()) mType = eBoxType::group;
    if(prp_getName().contains("Layer")) {
        auto newName  = prp_getName();
        newName.replace("Layer", "Group");
        prp_setName(newName);
    }
    mRasterEffectsAnimators->SWT_disable();
    prp_afterWholeInfluenceRangeChanged();

    const auto& linkingBoxes = getLinkingBoxes();
    for(const auto& box : linkingBoxes) {
        static_cast<ContainerBox*>(box)->demoteToGroup();
    }
}

void ContainerBox::updateAllBoxes(const UpdateReason reason) {
    for(const auto &child : mContainedBoxes) {
        child->updateAllBoxes(reason);
    }
    planUpdate(reason);
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
    if(!bT) planUpdate(UpdateReason::userChange);
    if(!mParentGroup) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *ContainerBox::getBoxAtFromAllDescendents(const QPointF &absPos) {
    if(SWT_isLinkBox()) return nullptr;
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
    for(int i = mContained.count() - 1; i >= 0; i--) {
        auto box = mContained.at(i);
        removeContained(box);
        mParentGroup->addContained(box);
    }
    removeFromParent_k();
}

void ContainerBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<ContainerBox>()) return;
    menu->addedActionsForType<ContainerBox>();

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

    BoxWithPathEffects::setupCanvasMenu(menu);
}

void ContainerBox::drawContained(SkCanvas * const canvas,
                                 const SkFilterQuality filter) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const auto& box = mContainedBoxes.at(i);
        if(box->isVisibleAndInVisibleDurationRect())
            box->drawPixmapSk(canvas, filter);
    }
}

void ContainerBox::drawPixmapSk(SkCanvas * const canvas,
                                const SkFilterQuality filter) {
    if(SWT_isGroupBox()) return drawContained(canvas, filter);
    if(mIsDescendantCurrentGroup) {
        SkPaint paint;
        const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
        paint.setAlpha(static_cast<U8CPU>(intAlpha));
        paint.setBlendMode(getBlendMode());
        canvas->saveLayer(nullptr, &paint);
        drawContained(canvas, filter);
        canvas->restore();
    } else BoundingBox::drawPixmapSk(canvas, filter);
}

qsptr<BoundingBox> ContainerBox::createLink() {
    auto linkBox = enve::make_shared<InternalLinkGroupBox>(this);
    copyTransformationTo(linkBox.get());
    return std::move(linkBox);
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
    if(!boxRenderData) boxRenderData = child->queRender(childRelFrame);
    if(!boxRenderData) return;
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;
}

stdsptr<BoxRenderData> ContainerBox::createRenderData() {
    return enve::make_shared<ContainerBoxRenderData>(this);
}

void ContainerBox::setupRenderData(const qreal relFrame,
                                   BoxRenderData * const data,
                                   Canvas* const scene) {
    BoundingBox::setupRenderData(relFrame, data, scene);
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
    const auto pScene = getParentScene();
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) continue;
        pScene->addBoxToSelection(box);
    }
}

void ContainerBox::deselectAllBoxesFromBoxesGroup() {
    const auto pScene = getParentScene();
    for(const auto& box : mContainedBoxes) {
        if(box->isSelected()) {
            pScene->removeBoxFromSelection(box);
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
    const auto pScene = getParentScene();
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
                box->isVisibleAndInVisibleDurationRect()) {
            if(box->isContainedIn(rect)) {
                pScene->addBoxToSelection(box);
            }
        }
    }
}

void ContainerBox::addContained(const qsptr<eBoxOrSound>& child) {
    insertContained(0, child);
}

#include "Sound/esoundlink.h"
void ContainerBox::insertContained(const int id, const qsptr<eBoxOrSound>& child) {
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
    if(!SWT_isLinkBox())
        SWT_addChildAt(child.get(), containedIdToAbstractionId(id));

    if(child->SWT_isBoundingBox()) {
        connCtx << connect(child.data(), &Property::prp_absFrameRangeChanged,
                           this, &Property::prp_afterChangedAbsRange);
        const auto cBox = static_cast<BoundingBox*>(child.get());
        const auto& linkingBoxes = getLinkingBoxes();
        for(const auto& box : linkingBoxes) {
            const auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
            internalLinkGroup->insertContained(id, cBox->createLink());
        }
    } else /*if(child->SWT_isSound())*/ {
        const auto sound = static_cast<SingleSound*>(child.get());
        const auto& linkingBoxes = getLinkingBoxes();
        for(const auto& box : linkingBoxes) {
            const auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
            internalLinkGroup->insertContained(id, sound->createLink());
        }
    }
    child->anim_setAbsFrame(anim_getCurrentAbsFrame());
    child->prp_afterWholeInfluenceRangeChanged();

    {
        prp_pushUndoRedoName("Insert " + child->prp_getName());
        UndoRedo ur;
        ur.fUndo = [this, child]() {
            removeContained(child);
        };
        ur.fRedo = [this, id, child]() {
            insertContained(id, child);
        };
        prp_addUndoRedo(ur);
    }
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
        const auto pScene = getParentScene();
        if(group->isCurrentGroup() && pScene) {
            pScene->setCurrentGroupParentAsCurrentGroup();
        }
    }

    SWT_removeChild(child.get());
    child->setParentGroup(nullptr);
    updateContainedIds(id);

    const auto& linkingBoxes = getLinkingBoxes();
    for(const auto& box : linkingBoxes) {
        const auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
        internalLinkGroup->removeContainedFromList(id);
    }

    prp_afterWholeInfluenceRangeChanged();

    {
        prp_pushUndoRedoName("Remove " + child->prp_getName());
        UndoRedo ur;
        ur.fUndo = [this, id, child]() {
            insertContained(id, child);
        };
        ur.fRedo = [this, id]() {
            removeContainedFromList(id);
        };
        prp_addUndoRedo(ur);
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
    prp_pushUndoRedoName("Lower " + child->prp_getName() + " to Bottom");
    moveContainedInList(child, index, mContainedBoxes.count() - 1);
}

void ContainerBox::bringContainedToFrontList(eBoxOrSound * const child) {
    const int index = getContainedIndex(child);
    if(index == 0) return;
    prp_pushUndoRedoName("Raise " + child->prp_getName() + " to Top");
    moveContainedInList(child, index, 0);
}

void ContainerBox::moveContainedInList(eBoxOrSound * const child, const int to) {
    const int from = getContainedIndex(child);
    if(from == -1) return;
    moveContainedInList(child, from, to);
}

void ContainerBox::moveContainedInList(const int from, const int to) {
    const auto child = mContained.at(from).get();
    moveContainedInList(child, from, to);
}

void ContainerBox::moveContainedInList(eBoxOrSound * const child,
                                       const int from, const int to) {
    const int boundTo = qBound(0, to, mContained.count() - 1);
    mContained.moveObj(from, boundTo);
    updateContainedBoxes();
    updateContainedIds(qMin(from, boundTo), qMax(from, boundTo));
    SWT_moveChildTo(child, containedIdToAbstractionId(boundTo));
    planUpdate(UpdateReason::userChange);

    const auto& linkingBoxes = getLinkingBoxes();
    for(const auto& box : linkingBoxes) {
        const auto internalLinkGroup = static_cast<InternalLinkGroupBox*>(box);
        internalLinkGroup->moveContainedInList(from, to);
    }

    prp_afterWholeInfluenceRangeChanged();


    {
        prp_pushUndoRedoName("Change Z-Index");
        UndoRedo ur;
        qptr<eBoxOrSound> childQPtr = child;
        ur.fUndo = [this, from, to, childQPtr]() {
            if(!childQPtr) return;
            moveContainedInList(childQPtr.data(), to, from);
        };
        ur.fRedo = [this, from, to, childQPtr]() {
            if(!childQPtr) return;
            moveContainedInList(childQPtr.data(), from, to);
        };
        prp_addUndoRedo(ur);
    }
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
        abstraction->addChildAbstraction(abs->ref<SWT_Abstraction>());
    }
}

bool ContainerBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                       const bool parentSatisfies,
                                       const bool parentMainTarget) const {
    const SWT_BoxRule rule = rules.fRule;
    const bool bbVisible = BoundingBox::SWT_shouldBeVisible(rules,
                                                            parentSatisfies,
                                                            parentMainTarget);
    if(rule == SWT_BR_SELECTED) return bbVisible && !isCurrentGroup();
    return bbVisible;
}

void ContainerBox::writeAllContained(eWriteStream& dst) {
    const int nCont = mContained.count();
    dst << nCont;
    for(int i = nCont - 1; i >= 0; i--) {
        const auto futureId = dst.planFuturePos();
        const auto &child = mContained.at(i);
        const bool isBox = child->SWT_isBoundingBox();
        dst << isBox;
        if(isBox) {
            const auto box = static_cast<BoundingBox*>(child.get());
            box->writeIdentifier(dst);
            box->writeBoundingBox(dst);
        } else {
            Q_ASSERT(child->SWT_isSingleSound());
            child->prp_writeProperty(dst);
        }
        dst.assignFuturePos(futureId);
        dst.writeCheckpoint();
    }
}

void ContainerBox::writeBoundingBox(eWriteStream& dst) {
    BoundingBox::writeBoundingBox(dst);
    dst.writeCheckpoint();
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
#include "internallinkbox.h"
#include "customboxcreator.h"

qsptr<BoundingBox> readIdCreateBox(eReadStream& src) {
    eBoxType type;
    src.read(&type, sizeof(eBoxType));
    switch(type) {
        case(eBoxType::vectorPath):
            return enve::make_shared<SmartVectorPath>();
        case(eBoxType::image):
            return enve::make_shared<ImageBox>();
        case(eBoxType::text):
            return enve::make_shared<TextBox>();
        case(eBoxType::video):
            return enve::make_shared<VideoBox>();
        case(eBoxType::rectangle):
            return enve::make_shared<Rectangle>();
        case(eBoxType::circle):
            return enve::make_shared<Circle>();
        case(eBoxType::layer):
            return enve::make_shared<ContainerBox>(eBoxType::layer);
        case(eBoxType::group):
            return enve::make_shared<ContainerBox>(eBoxType::group);
        case(eBoxType::paint):
            return enve::make_shared<PaintBox>();
        case(eBoxType::imageSequence):
            return enve::make_shared<ImageSequenceBox>();
        case(eBoxType::internalLink):
            return enve::make_shared<InternalLinkBox>(nullptr);
        case(eBoxType::internalLinkGroup):
            return enve::make_shared<InternalLinkGroupBox>(nullptr);
        case(eBoxType::externalLink):
            return enve::make_shared<ExternalLinkBox>();
        case(eBoxType::internalLinkCanvas):
            return enve::make_shared<InternalLinkCanvas>(nullptr);
        case(eBoxType::custom): {
            const auto id = CustomIdentifier::sRead(src);
            return CustomBoxCreator::sCreateForIdentifier(id);
        } default: {
            const int typeId = static_cast<int>(type);
            RuntimeThrow("Invalid box type '" + std::to_string(typeId) + "'");
        }
    }
}

void ContainerBox::readContained(eReadStream& src) {
    bool isBox;
    src >> isBox;
    if(isBox) {
        const auto box = readIdCreateBox(src);
        box->readBoundingBox(src);
        addContained(box);
    } else {
        const auto sound = enve::make_shared<SingleSound>();
        sound->prp_readProperty(src);
        addContained(sound);
    }
    src.readCheckpoint("Error reading contained");
}

void ContainerBox::readAllContained(eReadStream& src) {
    int nCont;
    src >> nCont;
    for(int i = 0; i < nCont; i++) {
        const auto futurePos = src.readFuturePos();
        try {
            readContained(src);
        } catch(const std::exception& e) {
            src.seek(futurePos);
            gPrintExceptionCritical(e);
        }
    }
}

void ContainerBox::readBoundingBox(eReadStream& src) {
    BoundingBox::readBoundingBox(src);
    src.readCheckpoint("Error reading ContainerBox basic properties");
    readAllContained(src);
}
