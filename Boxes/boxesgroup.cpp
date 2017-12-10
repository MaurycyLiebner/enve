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
                                       SkPath *srcDstPath) {
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, srcDstPath);
    if(mParentGroup == NULL) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterPathForRelFrame(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterOutlinePathBeforeThicknessForRelFrame(
        const int &relFrame, SkPath *srcDstPath) {
    mOutlinePathEffectsAnimators->filterPathForRelFrameBeforeThickness(relFrame,
                                                                       srcDstPath);
    if(mParentGroup == NULL) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterOutlinePathBeforeThicknessForRelFrame(parentRelFrame,
                                                              srcDstPath);
}

void BoxesGroup::filterOutlinePathForRelFrame(const int &relFrame,
                                              SkPath *srcDstPath) {
    mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, srcDstPath);
    if(mParentGroup == NULL) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterOutlinePathForRelFrame(parentRelFrame, srcDstPath);
}

void BoxesGroup::filterFillPathForRelFrame(const int &relFrame,
                                           SkPath *srcDstPath) {
    mFillPathEffectsAnimators->filterPathForRelFrame(relFrame, srcDstPath);
    if(mParentGroup == NULL) return;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterFillPathForRelFrame(parentRelFrame, srcDstPath);
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
    foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        int boxRelFrame = box->prp_absFrameToRelFrame(absFrame);
        if(box->isRelFrameVisibleAndInVisibleDurationRect(boxRelFrame)) {
            BoundingBoxRenderData *boxRenderData =
                    box->getCurrentRenderData();
            if(boxRenderData == NULL) {
                continue;
            }
            if(!boxRenderData->finished()) {
                boxRenderData->addDependent(data);
            }
            groupData->childrenRenderData <<
                    boxRenderData->ref<BoundingBoxRenderData>();
            childrenEffectsMargin =
                    qMax(box->getEffectsMarginAtRelFrame(boxRelFrame),
                         childrenEffectsMargin);
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
        canvas->saveLayer(NULL, &paint);
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
        if(mContainedBoxes.isEmpty() && mParentGroup != NULL) {
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
    if(mParentGroup == NULL) return;
    mParentGroup->setDescendantCurrentGroup(bT);
}

BoundingBox *BoxesGroup::getPathAtFromAllAncestors(const QPointF &absPos) {
    BoundingBox *boxAtPos = NULL;
    //Q_FOREACHBoxInListInverted(mChildren) {
    for(int i = mContainedBoxes.count() - 1; i >= 0; i--) {
        const QSharedPointer<BoundingBox> &box = mContainedBoxes.at(i);
        if(box->isVisibleAndUnlocked() &&
            box->isVisibleAndInVisibleDurationRect()) {
            boxAtPos = box->getPathAtFromAllAncestors(absPos);
            if(boxAtPos != NULL) {
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
    if(mContainedBoxes.isEmpty()) return NULL;
    return mContainedBoxes.first()->getFillSettings();
}

StrokeSettings *BoxesGroup::getStrokeSettings() {
    if(mContainedBoxes.isEmpty()) return NULL;
    return mContainedBoxes.first()->getStrokeSettings();
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
    BoundingBox *boxAtPos = NULL;

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
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
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
    BoundingBox *box = mContainedBoxes.at(id).data();
    box->clearAllCache();
    if(box->isSelected()) {
        box->removeFromSelection();
    }
    disconnect(box, 0, this, 0);
    if(saveUndoRedo) {
        addUndoRedo(new RemoveChildFromListUndoRedo(this, id,
                                                   box) );
    }
    mContainedBoxes.removeAt(id);

    if(box->SWT_isBoxesGroup()) {
        BoxesGroup *group = (BoxesGroup*) box;
        if(group->isCurrentGroup()) {
            mMainWindow->getCanvasWindow()->getCurrentCanvas()->
                    setCurrentBoxesGroup(group->getParentGroup());
        }
    }
    updateContainedBoxIds(id, saveUndoRedo);

    SWT_removeChildAbstractionForTargetFromAll(box);

    foreach(BoundingBox *box, mLinkingBoxes) {
        ((InternalLinkGroupBox*)box)->removeContainedBoxFromList(id, false);
    }
}

int BoxesGroup::getChildBoxIndex(BoundingBox *child) {
    int index = -1;
    for(int i = 0; i < mContainedBoxes.count(); i++) {
        if(mContainedBoxes.at(i) == child) {
            index = i;
        }
    }
    return index;
}

void BoxesGroup::removeContainedBox(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index < 0) {
        return;
    }
    removeContainedBoxFromList(index);
    if(mContainedBoxes.isEmpty() &&
       mParentGroup != NULL) {
        mParentGroup->removeContainedBox(this);
    }
    //child->setParent(NULL);
}


void BoxesGroup::increaseContainedBoxZInList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) {
        return;
    }
    moveContainedBoxInList(child, index, index + 1);
}

void BoxesGroup::decreaseContainedBoxZInList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == 0) {
        return;
    }
    moveContainedBoxInList(child, index, index - 1);
}

void BoxesGroup::bringContainedBoxToEndList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == mContainedBoxes.count() - 1) {
        return;
    }
    moveContainedBoxInList(child, index, mContainedBoxes.length() - 1);
}

void BoxesGroup::bringContainedBoxToFrontList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
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
        addUndoRedo(new MoveChildInListUndoRedo(child, from, to, this) );
    }

    scheduleUpdate();

    clearAllCache();
}

void BoxesGroup::moveContainedBoxBelow(BoundingBox *boxToMove,
                                BoundingBox *below) {
    const int &indexFrom = getChildBoxIndex(boxToMove);
    int indexTo = getChildBoxIndex(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveContainedBoxInList(boxToMove,
                    indexFrom,
                    indexTo);
}

void BoxesGroup::moveContainedBoxAbove(BoundingBox *boxToMove,
                                BoundingBox *above) {
    const int &indexFrom = getChildBoxIndex(boxToMove);
    int indexTo = getChildBoxIndex(above);
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

    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
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
    QRectF globalBoundingRect =
            transformRes.mapRect(relBoundingRect).
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
