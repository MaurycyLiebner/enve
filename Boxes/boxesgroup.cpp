#include "Boxes/boxesgroup.h"
#include "undoredo.h"
#include <QApplication>
#include "mainwindow.h"
#include "ctrlpoint.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "BoxesList/boxscrollwidget.h"
#include "textbox.h"
#include "BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "canvaswindow.h"
#include "canvas.h"
#include "Boxes/particlebox.h"
#include "durationrectangle.h"
#include "linkbox.h"
#include "Animators/animatorupdater.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"

bool BoxesGroup::mCtrlsAlwaysVisible = false;

//bool zMoreThan(BoundingBox *box1, BoundingBox *box2)
//{
//    return box1->getZIndex() < box2->getZIndex();
//}

BoxesGroup::BoxesGroup() :
    BoundingBox(BoundingBoxType::TYPE_GROUP) {
    setName("Group");
    mFillStrokeSettingsWidget = getMainWindow()->getFillStrokeSettings();
    iniPathEffects();
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting) :
    BoundingBox(BoundingBoxType::TYPE_CANVAS) {
    mFillStrokeSettingsWidget = fillStrokeSetting;
    iniPathEffects();
}

//bool BoxesGroup::prp_nextRelFrameWithKey(const int &relFrame,
//                                         int &nextRelFrame) {
//    int thisMinNextFrame = BoundingBox::prp_nextRelFrameWithKey(relFrame);
//    return thisMinNextFrame;
//    int minNextAbsFrame = INT_MAX;
//    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
//        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
//        int boxNext = box->prp_nextRelFrameWithKey(boxRelFrame);
//        int absNext = box->prp_relFrameToAbsFrame(boxNext);
//        if(minNextAbsFrame > absNext) {
//            minNextAbsFrame = absNext;
//        }
//    }

//    return qMin(prp_absFrameToRelFrame(minNextAbsFrame), thisMinNextFrame);
//}

//int BoxesGroup::prp_prevRelFrameWithKey(const int &relFrame,
//                                        int &prevRelFrame) {
//    int thisMaxPrevFrame = BoundingBox::prp_nextRelFrameWithKey(relFrame);
//    return thisMaxPrevFrame;
//    int maxPrevAbsFrame = INT_MIN;
//    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
//        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
//        int boxPrev = box->prp_prevRelFrameWithKey(boxRelFrame);
//        int absPrev = box->prp_relFrameToAbsFrame(boxPrev);
//        if(maxPrevAbsFrame < absPrev) {
//            maxPrevAbsFrame = absPrev;
//        }
//    }
//    return qMax(maxPrevAbsFrame, thisMaxPrevFrame);
//}

void BoxesGroup::iniPathEffects() {
    mPathEffectsAnimators =
            (new PathEffectAnimators(false, false, this))->
            ref<PathEffectAnimators>();
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setBlockedUpdater(
                new GroupAllPathsUpdater(this));
    ca_addChildAnimator(mPathEffectsAnimators.data());
    mPathEffectsAnimators->SWT_hide();

    mFillPathEffectsAnimators =
            (new PathEffectAnimators(false, true, this))->
            ref<PathEffectAnimators>();
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setBlockedUpdater(
                new GroupAllPathsUpdater(this));
    ca_addChildAnimator(mFillPathEffectsAnimators.data());
    mFillPathEffectsAnimators->SWT_hide();

    mOutlinePathEffectsAnimators =
            (new PathEffectAnimators(true, false, this))->
            ref<PathEffectAnimators>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
                new GroupAllPathsUpdater(this));
    ca_addChildAnimator(mOutlinePathEffectsAnimators.data());
    mOutlinePathEffectsAnimators->SWT_hide();
}

void BoxesGroup::addPathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

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
    effect->setParentEffectAnimators(mPathEffectsAnimators.data());

    clearAllCache();
    updateAllChildPathBoxes();
}

void BoxesGroup::addFillPathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_show();
    }
    mFillPathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mFillPathEffectsAnimators.data());

    clearAllCache();
    updateAllChildPathBoxes();
}

void BoxesGroup::addOutlinePathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_show();
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mOutlinePathEffectsAnimators.data());

    clearAllCache();
    updateAllChildPathBoxes();
}

void BoxesGroup::removePathEffect(PathEffect *effect) {
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

    clearAllCache();
    updateAllChildPathBoxes();
}

void BoxesGroup::removeFillPathEffect(PathEffect *effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mFillPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
    updateAllChildPathBoxes();
}

void BoxesGroup::removeOutlinePathEffect(PathEffect *effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mOutlinePathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
    updateAllChildPathBoxes();
}

void BoxesGroup::filterPathForRelFrame(const int &relFrame,
                                       SkPath *srcDstPath,
                                       BoundingBox *box) {
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, srcDstPath,
                                                 qAbs(box->getCombinedTransform().m11()),
                                                 box == this);

    if(mParentGroup == nullptr) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterPathForRelFrame(parentRelFrame, srcDstPath, box);
}

void BoxesGroup::filterPathForRelFrameUntilGroupSum(const int &relFrame,
                                                    SkPath *srcDstPath,
                                                    BoundingBox *box) {
    mPathEffectsAnimators->filterPathForRelFrameUntilGroupSum(relFrame,
                                                              srcDstPath,
                                                              qAbs(box->getCombinedTransform().m11()));

    if(mParentGroup == nullptr) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterPathForRelFrameUntilGroupSum(parentRelFrame, srcDstPath, box);
}

void BoxesGroup::filterOutlinePathBeforeThicknessForRelFrame(
        const int &relFrame, SkPath *srcDstPath,
        BoundingBox *box) {
    mOutlinePathEffectsAnimators->filterPathForRelFrameBeforeThickness(relFrame,
                                                                       srcDstPath,
                                                                       qAbs(box->getCombinedTransform().m11()));
    if(mParentGroup == nullptr) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterOutlinePathBeforeThicknessForRelFrame(parentRelFrame,
                                                              srcDstPath, box);
}

void BoxesGroup::filterPathForRelFrameF(const qreal &relFrame,
                                       SkPath *srcDstPath,
                                       BoundingBox *box) {
    mPathEffectsAnimators->filterPathForRelFrameF(relFrame, srcDstPath,
                                                 box == this);

    if(mParentGroup == nullptr) return;
    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterPathForRelFrameF(parentRelFrame, srcDstPath, box);
}

void BoxesGroup::filterPathForRelFrameUntilGroupSumF(const qreal &relFrame,
                                                    SkPath *srcDstPath) {
    mPathEffectsAnimators->filterPathForRelFrameUntilGroupSumF(relFrame,
                                                              srcDstPath);

    if(mParentGroup == nullptr) return;
    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterPathForRelFrameUntilGroupSumF(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterOutlinePathBeforeThicknessForRelFrameF(
        const qreal &relFrame, SkPath *srcDstPath) {
    mOutlinePathEffectsAnimators->filterPathForRelFrameBeforeThicknessF(relFrame,
                                                                       srcDstPath);
    if(mParentGroup == nullptr) return;
    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterOutlinePathBeforeThicknessForRelFrameF(parentRelFrame,
                                                              srcDstPath);
}

bool BoxesGroup::isLastPathBox(PathBox *pathBox) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        BoundingBox *childAtI = mContainedBoxes.at(i).data();
        if(childAtI == pathBox) return true;
        if(childAtI->SWT_isPathBox()) return false;
    }
    return false;
}

void BoxesGroup::filterOutlinePathForRelFrame(const int &relFrame,
                                              SkPath *srcDstPath) {
    mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, srcDstPath);
    if(mParentGroup == nullptr) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterOutlinePathForRelFrame(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterFillPathForRelFrame(const int &relFrame,
                                           SkPath *srcDstPath) {
    mFillPathEffectsAnimators->filterPathForRelFrame(relFrame, srcDstPath);
    if(mParentGroup == nullptr) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterFillPathForRelFrame(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterOutlinePathForRelFrameF(const qreal &relFrame,
                                              SkPath *srcDstPath) {
    mOutlinePathEffectsAnimators->filterPathForRelFrameF(relFrame, srcDstPath);
    if(mParentGroup == nullptr) return;
    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterOutlinePathForRelFrameF(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterFillPathForRelFrameF(const qreal &relFrame,
                                           SkPath *srcDstPath) {
    mFillPathEffectsAnimators->filterPathForRelFrameF(relFrame, srcDstPath);
    if(mParentGroup == nullptr) return;
    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterFillPathForRelFrameF(parentRelFrame, srcDstPath);
}

bool BoxesGroup::enabledGroupPathSumEffectPresent() {
    foreach(PathEffect *effect, mGroupPathSumEffects) {
        if(effect->isVisible()) return true;
    }
    return false;
}

void BoxesGroup::prp_setParentFrameShift(const int &shift,
                                         ComplexAnimator *parentAnimator) {
    ComplexAnimator::prp_setParentFrameShift(shift, parentAnimator);
    int thisShift = prp_getFrameShift();
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->prp_setParentFrameShift(thisShift, this);
    }
}

void BoxesGroup::processSchedulers() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->processSchedulers();
    }
    BoundingBox::processSchedulers();
}

void BoxesGroup::addSchedulersToProcess() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->addSchedulersToProcess();
    }
    BoundingBox::addSchedulersToProcess();
}

void BoxesGroup::updateAllBoxes() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->updateAllBoxes();
    }
    scheduleUpdate();
}

void BoxesGroup::clearAllCache() {
    BoundingBox::clearAllCache();
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->clearAllCache();
    }
}

bool BoxesGroup::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                 const int &relFrame2) {
    bool differences =
            BoundingBox::prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2);
    if(differences) return true;
    int absFrame1 = prp_relFrameToAbsFrame(relFrame1);
    int absFrame2 = prp_relFrameToAbsFrame(relFrame2);
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        if(child->prp_differencesBetweenRelFrames(
                    child->prp_absFrameToRelFrame(absFrame1),
                    child->prp_absFrameToRelFrame(absFrame2))) {
            return true;
        }
    }
    return false;
}

void BoxesGroup::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                       int *lastIdentical,
                                                       const int &relFrame) {
    int fId;
    int lId;

    BoundingBox::prp_getFirstAndLastIdenticalRelFrame(&fId, &lId, relFrame);
    int absFrame = prp_relFrameToAbsFrame(relFrame);
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        if(fId > lId) {
            break;
        }
        int fIdT;
        int lIdT;
        child->prp_getFirstAndLastIdenticalRelFrame(
                    &fIdT, &lIdT,
                    child->prp_absFrameToRelFrame(absFrame));
        int fIdAbsT = child->prp_relFrameToAbsFrame(fIdT);
        int lIdAbsT = child->prp_relFrameToAbsFrame(lIdT);
        fIdT = prp_absFrameToRelFrame(fIdAbsT);
        lIdT = prp_absFrameToRelFrame(lIdAbsT);
        if(fIdT > fId) {
            fId = fIdT;
        }
        if(lIdT < lId) {
            lId = lIdT;
        }
    }

    if(lId > fId) {
        *firstIdentical = fId;
        *lastIdentical = lId;
    } else {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    }
}

void BoxesGroup::getFirstAndLastIdenticalForMotionBlur(int *firstIdentical,
                                                       int *lastIdentical,
                                                       const int &relFrame,
                                                       const bool &takeAncestorsIntoAccount) {
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            int fId = INT_MIN;
            int lId = INT_MAX;
            {
                int fId_ = INT_MIN;
                int lId_ = INT_MAX;

                QList<Property*> propertiesT;
                getMotionBlurProperties(&propertiesT);
                Q_FOREACH(Property *child, propertiesT) {
                    if(fId_ > lId_) {
                        break;
                    }
                    int fIdT_;
                    int lIdT_;
                    child->prp_getFirstAndLastIdenticalRelFrame(
                                &fIdT_, &lIdT_,
                                relFrame);
                    if(fIdT_ > fId_) {
                        fId_ = fIdT_;
                    }
                    if(lIdT_ < lId_) {
                        lId_ = lIdT_;
                    }
                }

                Q_FOREACH(const QSharedPointer<BoundingBox> &child,
                          mContainedBoxes) {
                    if(fId_ > lId_) {
                        break;
                    }
                    int fIdT_;
                    int lIdT_;
                    child->getFirstAndLastIdenticalForMotionBlur(
                                &fIdT_, &lIdT_,
                                relFrame, false);
                    if(fIdT_ > fId_) {
                        fId_ = fIdT_;
                    }
                    if(lIdT_ < lId_) {
                        lId_ = lIdT_;
                    }
                }

                if(lId_ > fId_) {
                    fId = fId_;
                    lId = lId_;
                } else {
                    fId = relFrame;
                    lId = relFrame;
                }
            }
            *firstIdentical = fId;
            *lastIdentical = lId;
            if(mDurationRectangle != nullptr) {
                if(fId < mDurationRectangle->getMinFrameAsRelFrame()) {
                    *firstIdentical = relFrame;
                }
                if(lId > mDurationRectangle->getMaxFrameAsRelFrame()) {
                    *lastIdentical = relFrame;
                }
            }
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                *firstIdentical = mDurationRectangle->getMaxFrameAsRelFrame() + 1;
                *lastIdentical = INT_MAX;
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                *firstIdentical = INT_MIN;
                *lastIdentical = mDurationRectangle->getMinFrameAsRelFrame() - 1;
            }
        }
    } else {
        *firstIdentical = INT_MIN;
        *lastIdentical = INT_MAX;
    }
    if(mParentGroup == nullptr || takeAncestorsIntoAccount) return;
    if(*firstIdentical == relFrame && *lastIdentical == relFrame) return;
    int parentFirst;
    int parentLast;
    int parentRel = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->BoundingBox::getFirstAndLastIdenticalForMotionBlur(
                  &parentFirst,
                  &parentLast,
                  parentRel);
    if(parentFirst > *firstIdentical) {
        *firstIdentical = parentFirst;
    }
    if(parentLast < *lastIdentical) {
        *lastIdentical = parentLast;
    }
}

BoxesGroup::~BoxesGroup() {
}

bool BoxesGroup::relPointInsidePath(const QPointF &relPos) {
    if(mRelBoundingRect.contains(relPos)) {
        QPointF absPos = mapRelPosToAbs(relPos);
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            if(box->absPointInsidePath(absPos)) {
                return true;
            }
        }
    }
    return false;
}

void BoxesGroup::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void BoxesGroup::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void BoxesGroup::setStrokeWidth(const qreal &strokeWidth, const bool &finish) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void BoxesGroup::startSelectedStrokeWidthTransform() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void BoxesGroup::startSelectedStrokeColorTransform() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void BoxesGroup::startSelectedFillColorTransform() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

void BoxesGroup::shiftAll(const int &shift) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(shift);
    } else {
        anim_shiftAllKeys(shift);
        foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            box->shiftAll(shift);
        }
    }
}

BoundingBox *BoxesGroup::createLink() {
    InternalLinkGroupBox *linkBox = new InternalLinkGroupBox(this);
    copyBoundingBoxDataTo(linkBox);
    return linkBox;
}

void BoxesGroup::setupBoundingBoxRenderDataForRelFrame(
                        const int &relFrame,
                        BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                       data);
    BoxesGroupRenderData *groupData = ((BoxesGroupRenderData*)data);
    groupData->childrenRenderData.clear();
    qreal childrenEffectsMargin = 0.;
    int absFrame = prp_relFrameToAbsFrame(relFrame);
    if(enabledGroupPathSumEffectPresent()) {
        int idT = 0;
        PathBox *lastPathBox = nullptr;
        foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            int boxRelFrame = box->prp_absFrameToRelFrame(absFrame);
            if(box->isRelFrameVisibleAndInVisibleDurationRect(boxRelFrame)) {
                BoundingBoxRenderData *boxRenderData =
                        box->getCurrentRenderData();
                if(boxRenderData == nullptr) {
                    continue;
                }
                if(box->SWT_isPathBox()) {
                    idT = groupData->childrenRenderData.count();
                    lastPathBox = (PathBox*)box.data();
                    continue;
                }
                boxRenderData->addDependent(data);
                groupData->childrenRenderData <<
                        boxRenderData->ref<BoundingBoxRenderData>();
                childrenEffectsMargin =
                        qMax(box->getEffectsMarginAtRelFrame(boxRelFrame),
                             childrenEffectsMargin);
            }
        }
        if(lastPathBox != nullptr) {
            int boxRelFrame = lastPathBox->prp_absFrameToRelFrame(absFrame);
            BoundingBoxRenderData *boxRenderData = new PathBoxRenderData(this);
            lastPathBox->setupBoundingBoxRenderDataForRelFrame(
                boxRelFrame, boxRenderData);
            boxRenderData->addScheduler();
            boxRenderData->addDependent(data);
            groupData->childrenRenderData.insert(idT,
                    boxRenderData->ref<BoundingBoxRenderData>());
            boxRenderData->parentBox = nullptr;
        }
    } else {
        foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            int boxRelFrame = box->prp_absFrameToRelFrame(absFrame);
            if(box->isRelFrameVisibleAndInVisibleDurationRect(boxRelFrame)) {
                BoundingBoxRenderData *boxRenderData =
                        box->getCurrentRenderData();
                if(boxRenderData == nullptr) {
                    continue;
                }
                boxRenderData->addDependent(data);
                groupData->childrenRenderData <<
                        boxRenderData->ref<BoundingBoxRenderData>();
                childrenEffectsMargin =
                        qMax(box->getEffectsMarginAtRelFrame(boxRelFrame),
                             childrenEffectsMargin);
            }
        }
    }

    data->effectsMargin += childrenEffectsMargin;
}

void BoxesGroup::setupBoundingBoxRenderDataForRelFrameF(
                        const qreal &relFrame,
                        BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
    BoxesGroupRenderData *groupData = ((BoxesGroupRenderData*)data);
    groupData->childrenRenderData.clear();
    qreal childrenEffectsMargin = 0.;
    qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    if(enabledGroupPathSumEffectPresent()) {
        int idT = 0;
        PathBox *lastPathBox = nullptr;
        foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
            if(box->isRelFrameFVisibleAndInVisibleDurationRect(boxRelFrame)) {
                BoundingBoxRenderData *boxRenderData = box->createRenderData();
                if(box->SWT_isPathBox()) {
                    idT = groupData->childrenRenderData.count();
                    lastPathBox = (PathBox*)box.data();
                    continue;
                }
                boxRenderData->parentIsTarget = false;
                boxRenderData->useCustomRelFrame = true;
                boxRenderData->customRelFrame = boxRelFrame;
                boxRenderData->addScheduler();
                boxRenderData->addDependent(data);
                groupData->childrenRenderData <<
                        boxRenderData->ref<BoundingBoxRenderData>();
                childrenEffectsMargin =
                        qMax(box->getEffectsMarginAtRelFrameF(boxRelFrame),
                             childrenEffectsMargin);
                boxRenderData->schedulerProccessed();
                mMainWindow->getCanvasWindow()->addUpdatableAwaitingUpdate(boxRenderData);
            }
        }
        if(lastPathBox != nullptr) {
            qreal boxRelFrame = lastPathBox->prp_absFrameToRelFrameF(absFrame);
            BoundingBoxRenderData *boxRenderData = new PathBoxRenderData(this);
            lastPathBox->setupBoundingBoxRenderDataForRelFrameF(
                boxRelFrame, boxRenderData);
            boxRenderData->addScheduler();
            boxRenderData->addDependent(data);
            groupData->childrenRenderData.insert(idT,
                    boxRenderData->ref<BoundingBoxRenderData>());
            boxRenderData->parentBox = nullptr;
        }
    } else {
        foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
            if(box->isRelFrameFVisibleAndInVisibleDurationRect(boxRelFrame)) {
                BoundingBoxRenderData *boxRenderData = box->createRenderData();
                boxRenderData->parentIsTarget = false;
                boxRenderData->useCustomRelFrame = true;
                boxRenderData->customRelFrame = boxRelFrame;
                boxRenderData->addScheduler();
                boxRenderData->addDependent(data);
                groupData->childrenRenderData <<
                        boxRenderData->ref<BoundingBoxRenderData>();
                childrenEffectsMargin =
                        qMax(box->getEffectsMarginAtRelFrameF(boxRelFrame),
                             childrenEffectsMargin);
                boxRenderData->schedulerProccessed();
                mMainWindow->getCanvasWindow()->addUpdatableAwaitingUpdate(boxRenderData);
            }
        }
    }

    data->effectsMargin += childrenEffectsMargin;
}


void BoxesGroup::drawPixmapSk(SkCanvas *canvas) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawPixmapSk(canvas);
    } else {
        SkPaint paint;
        paint.setAlpha(qRound(mTransformAnimator->getOpacity()*2.55));
        paint.setBlendMode(mBlendModeSk);
        canvas->saveLayer(nullptr, &paint);
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            //box->draw(p);
            box->drawPixmapSk(canvas);
        }
        canvas->restore();
    }
}

void BoxesGroup::drawSelectedSk(SkCanvas *canvas,
                                 const CanvasMode &currentCanvasMode,
                                 const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == MOVE_PATH && !mIsCurrentGroup) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

void BoxesGroup::setIsCurrentGroup(const bool &bT) {
    mIsCurrentGroup = bT;
    if(!bT) {
        if(mContainedBoxes.isEmpty() && mParentGroup != nullptr) {
            mParentGroup->removeContainedBox(this);
        }
    }
    setDescendantCurrentGroup(bT);
}

bool BoxesGroup::isCurrentGroup() {
    return mIsCurrentGroup;
}

bool BoxesGroup::isDescendantCurrentGroup() {
    return mIsDescendantCurrentGroup;
}

bool BoxesGroup::shouldPaintOnImage() {
    return !mIsDescendantCurrentGroup;
    return mEffectsAnimators->hasEffects() &&
           !mIsDescendantCurrentGroup;
}

void BoxesGroup::setDescendantCurrentGroup(const bool &bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) {
        scheduleUpdate();
    }
    if(mParentGroup == nullptr) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *BoxesGroup::getPathAtFromAllAncestors(const QPointF &absPos) {
    BoundingBox *boxAtPos = nullptr;
    //Q_FOREACHBoxInListInverted(mChildren) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const QSharedPointer<BoundingBox> &box = mContainedBoxes.at(i);
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            boxAtPos = box->getPathAtFromAllAncestors(absPos);
            if(boxAtPos != nullptr) {
                break;
            }
        }
    }
    return boxAtPos;
}

void BoxesGroup::ungroup() {
    //clearBoxesSelection();
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->applyTransformation(mTransformAnimator.data());
        removeContainedBox(box.data());
        mParentGroup->addContainedBox(box.data());
    }
    mParentGroup->removeContainedBox(this);
}

PaintSettings *BoxesGroup::getFillSettings() {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getFillSettings();
}

StrokeSettings *BoxesGroup::getStrokeSettings() {
    if(mContainedBoxes.isEmpty()) return nullptr;
    return mContainedBoxes.last()->getStrokeSettings();
}

void BoxesGroup::setCurrentFillStrokeSettingsFromBox(BoundingBox *box) {
    mFillStrokeSettingsWidget->setCurrentSettings(box->getFillSettings(),
                                                  box->getStrokeSettings());
}

void BoxesGroup::applyCurrentTransformation() {
    mNReasonsNotToApplyUglyTransform++;
    QPointF absPivot = getPivotAbsPos();
    qreal rotation = mTransformAnimator->rot();
    qreal scaleX = mTransformAnimator->xScale();
    qreal scaleY = mTransformAnimator->yScale();
    QPointF relTrans = mTransformAnimator->pos();
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
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

    mTransformAnimator->resetRotation(true);
    mTransformAnimator->resetScale(true);
    mTransformAnimator->resetTranslation(true);
    mNReasonsNotToApplyUglyTransform--;
}

void BoxesGroup::selectAllBoxesFromBoxesGroup() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        if(box->isSelected()) continue;
        getParentCanvas()->addBoxToSelection(box.data());
    }
}

void BoxesGroup::deselectAllBoxesFromBoxesGroup() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        if(box->isSelected()) {
            getParentCanvas()->removeBoxFromSelection(box.data());
        }
    }
}

BoundingBox *BoxesGroup::getBoxAt(const QPointF &absPos) {
    BoundingBox *boxAtPos = nullptr;

    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const QSharedPointer<BoundingBox> &box = mContainedBoxes.at(i);
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            if(box->absPointInsidePath(absPos)) {
                boxAtPos = box.data();
                break;
            }
        }
    }
    return boxAtPos;
}

void BoxesGroup::addContainedBoxesToSelection(const QRectF &rect) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            if(box->isContainedIn(rect) ) {
                getParentCanvas()->addBoxToSelection(box.data());
            }
        }
    }
}

void BoxesGroup::addContainedBox(BoundingBox *child) {
    //child->setParent(this);
    addContainedBoxToListAt(mContainedBoxes.count(), child);
}

void BoxesGroup::addContainedBoxToListAt(const int &index,
                                  BoundingBox *child,
                                  const bool &saveUndoRedo) {
    mContainedBoxes.insert(index, child->ref<BoundingBox>());
    child->setParentGroup(this);
    if(saveUndoRedo) {
//        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
    connect(child, SIGNAL(prp_absFrameRangeChanged(int,int)),
            this, SLOT(prp_updateAfterChangedAbsFrameRange(int,int)));
    updateContainedBoxIds(index, saveUndoRedo);

    //SWT_addChildAbstractionForTargetToAll(child);
    SWT_addChildAbstractionForTargetToAllAt(child,
                                            ca_mChildAnimators.count());
    child->prp_setAbsFrame(anim_mCurrentAbsFrame);

    child->prp_updateInfluenceRangeAfterChanged();

    foreach(BoundingBox *box, mLinkingBoxes) {
        ((InternalLinkGroupBox*)box)->addContainedBoxToListAt(index,
                                                       child->createLinkForLinkGroup(),
                                                       false);
    }
}

void BoxesGroup::updateContainedBoxIds(const int &firstId,
                                  const bool &saveUndoRedo) {
    updateContainedBoxIds(firstId, mContainedBoxes.length() - 1, saveUndoRedo);
}

void BoxesGroup::updateContainedBoxIds(const int &firstId,
                                  const int &lastId,
                                  const bool &saveUndoRedo) {
    for(int i = firstId; i <= lastId; i++) {
        mContainedBoxes.at(i)->setZListIndex(i, saveUndoRedo);
    }
}

void BoxesGroup::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->prp_setAbsFrame(frame);
    }
}

void BoxesGroup::removeContainedBoxFromList(const int &id,
                                     const bool &saveUndoRedo) {
    QSharedPointer<BoundingBox> box = mContainedBoxes.at(id);
    box->clearAllCache();
    if(box->isSelected()) {
        box->removeFromSelection();
    }
    disconnect(box.data(), 0, this, 0);
    if(saveUndoRedo) {
//        addUndoRedo(new RemoveChildFromListUndoRedo(this, id,
//                                                   box) );
    }
    mContainedBoxes.removeAt(id);

    if(box->SWT_isBoxesGroup()) {
        BoxesGroup *group = (BoxesGroup*)box.data();
        if(group->isCurrentGroup()) {
            mMainWindow->getCanvasWindow()->getCurrentCanvas()->
                    setCurrentBoxesGroup(group->getParentGroup());
        }
    }
    updateContainedBoxIds(id, saveUndoRedo);

    SWT_removeChildAbstractionForTargetFromAll(box.data());

    foreach(BoundingBox *box, mLinkingBoxes) {
        ((InternalLinkGroupBox*)box)->removeContainedBoxFromList(id, false);
    }
}

int BoxesGroup::getContainedBoxIndex(BoundingBox *child) {
    int index = -1;
    for(int i = 0; i < mContainedBoxes.count(); i++) {
        if(mContainedBoxes.at(i) == child) {
            index = i;
        }
    }
    return index;
}

void BoxesGroup::removeContainedBox(BoundingBox *child) {
    const int &index = getContainedBoxIndex(child);
    if(index < 0) {
        return;
    }
    child->removeFromSelection();
    removeContainedBoxFromList(index);
    if(mContainedBoxes.isEmpty() &&
       mParentGroup != nullptr) {
        mParentGroup->removeContainedBox(this);
    }
    //child->setParent(nullptr);
}


void BoxesGroup::increaseContainedBoxZInList(BoundingBox *child) {
    const int &index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) {
        return;
    }
    moveContainedBoxInList(child, index, index + 1);
}

void BoxesGroup::decreaseContainedBoxZInList(BoundingBox *child) {
    const int &index = getContainedBoxIndex(child);
    if(index == 0) {
        return;
    }
    moveContainedBoxInList(child, index, index - 1);
}

void BoxesGroup::bringContainedBoxToEndList(BoundingBox *child) {
    const int &index = getContainedBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) {
        return;
    }
    moveContainedBoxInList(child, index, mContainedBoxes.length() - 1);
}

void BoxesGroup::bringContainedBoxToFrontList(BoundingBox *child) {
    const int &index = getContainedBoxIndex(child);
    if(index == 0) {
        return;
    }
    moveContainedBoxInList(child, index, 0);
}

void BoxesGroup::moveContainedBoxInList(BoundingBox *child,
                                 const int &from, const int &to,
                                 const bool &saveUndoRedo) {
    mContainedBoxes.move(from, to);
    updateContainedBoxIds(qMin(from, to), qMax(from, to), saveUndoRedo);
    SWT_moveChildAbstractionForTargetToInAll(child, mContainedBoxes.count() - to - 1
                                                    + ca_mChildAnimators.count());
    if(saveUndoRedo) {
//        addUndoRedo(new MoveChildInListUndoRedo(child, from, to, this) );
    }

    scheduleUpdate();

    clearAllCache();
}

void BoxesGroup::moveContainedBoxBelow(BoundingBox *boxToMove,
                                BoundingBox *below) {
    const int &indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveContainedBoxInList(boxToMove,
                    indexFrom,
                    indexTo);
}

void BoxesGroup::moveContainedBoxAbove(BoundingBox *boxToMove,
                                BoundingBox *above) {
    const int &indexFrom = getContainedBoxIndex(boxToMove);
    int indexTo = getContainedBoxIndex(above);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveContainedBoxInList(boxToMove,
                    indexFrom,
                    indexTo);
}

#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void BoxesGroup::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    BoundingBox::SWT_addChildrenAbstractions(abstraction,
                                             visiblePartWidget);

    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const QSharedPointer<BoundingBox> &child = mContainedBoxes.at(i);
        abstraction->addChildAbstraction(
                    child->SWT_getAbstractionForWidget(visiblePartWidget));
    }
}

bool BoxesGroup::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool &parentSatisfies,
                                     const bool &parentMainTarget) {
    const SWT_Rule &rule = rules.rule;
    if(rule == SWT_Selected) {
        return BoundingBox::SWT_shouldBeVisible(rules,
                                                parentSatisfies,
                                                parentMainTarget) &&
                !isCurrentGroup();
    }
    return BoundingBox::SWT_shouldBeVisible(rules,
                                            parentSatisfies,
                                            parentMainTarget);
}
#include "PixmapEffects/fmt_filters.h"
void BoxesGroupRenderData::renderToImage() {
    if(renderedToImage) return;
    renderedToImage = true;
    QMatrix scale;
    scale.scale(resolution, resolution);
    QMatrix transformRes = transform*scale;
    //transformRes.scale(resolution, resolution);
    globalBoundingRect = transformRes.mapRect(relBoundingRect);
    foreach(const QRectF &rectT, otherGlobalRects) {
        globalBoundingRect = globalBoundingRect.united(rectT);
    }
    globalBoundingRect = globalBoundingRect.
            adjusted(-effectsMargin, -effectsMargin,
                     effectsMargin, effectsMargin);
    if(maxBoundsEnabled) {
        globalBoundingRect = globalBoundingRect.intersected(
                    scale.mapRect(maxBoundsRect));
    }
    QSizeF sizeF = globalBoundingRect.size();
    QPointF transF = globalBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(globalBoundingRect.left()/**resolution*/),
                    qRound(globalBoundingRect.top()/**resolution*/));
    globalBoundingRect.translate(-transF);
    SkImageInfo info = SkImageInfo::Make(ceil(sizeF.width()),
                                         ceil(sizeF.height()),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas *rasterCanvas = new SkCanvas(bitmap);//rasterSurface->getCanvas();
    rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas->translate(-globalBoundingRect.left(),
                            -globalBoundingRect.top());
    rasterCanvas->concat(QMatrixToSkMatrix(scale));

    drawSk(rasterCanvas);
    rasterCanvas->flush();
    delete rasterCanvas;

    drawPos = SkPoint::Make(qRound(globalBoundingRect.left()),
                            qRound(globalBoundingRect.top()));

    if(!pixmapEffects.isEmpty()) {
        SkPixmap pixmap;
        bitmap.peekPixels(&pixmap);
        fmt_filters::image img((uint8_t*)pixmap.writable_addr(),
                               pixmap.width(), pixmap.height());
        foreach(PixmapEffectRenderData *effect, pixmapEffects) {
            effect->applyEffectsSk(bitmap, img, resolution);
        }
        clearPixmapEffects();
    }

    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}
