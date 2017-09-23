#include "Boxes/boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include <QDebug>
#include "mainwindow.h"
#include "keysview.h"
#include "BoxesList/boxscrollwidget.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
#include "BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "durationrectangle.h"
#include "PixmapEffects/fmt_filters.h"
#include "Animators/animatorupdater.h"
#include "pointhelpers.h"
#include "skqtconversions.h"
#include "global.h"

QList<BoundingBox*> BoundingBox::mLoadedBoxes;
QList<FunctionWaitingForBoxLoad*> BoundingBox::mFunctionsWaitingForBoxLoad;

BoundingBox::BoundingBox(const BoundingBoxType &type) :
    ComplexAnimator(), Transformable() {
    mEffectsAnimators->prp_setName("effects");
    mEffectsAnimators->setParentBox(this);
    mEffectsAnimators->prp_setUpdater(new PixmapEffectUpdater(this));
    mEffectsAnimators->prp_blockUpdater();

    ca_addChildAnimator(mTransformAnimator.data());

    mType = type;

    mTransformAnimator->reset();
}

BoundingBox::~BoundingBox() {
}

void BoundingBox::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                      const int &maxFrame) {
    Property::prp_updateAfterChangedAbsFrameRange(minFrame,
                                                  maxFrame);
    if(anim_mCurrentAbsFrame >= minFrame) {
        if(anim_mCurrentAbsFrame <= maxFrame) {
            scheduleUpdate();
        }
    }
}

void BoundingBox::ca_childAnimatorIsRecordingChanged() {
    ComplexAnimator::ca_childAnimatorIsRecordingChanged();
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Animated);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_NotAnimated);
}

SingleWidgetAbstraction* BoundingBox::SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget) {
    Q_FOREACH(const std::shared_ptr<SingleWidgetAbstraction> &abs,
              mSWT_allAbstractions) {
        if(abs->getParentVisiblePartWidget() == visiblePartWidget) {
            return abs.get();
        }
    }
    return SWT_createAbstraction(visiblePartWidget);
}

#include "linkbox.h"
BoundingBox *BoundingBox::createLink() {
    InternalLinkBox *linkBox = new InternalLinkBox(this);
    BoundingBox::makeDuplicate(linkBox);
    return linkBox;
}

void BoundingBox::makeDuplicate(Property *property) {
    BoundingBox *targetBox = (BoundingBox*)property;
    targetBox->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    int effectsCount = mEffectsAnimators->ca_getNumberOfChildren();
    for(int i = 0; i < effectsCount; i++) {
        targetBox->addEffect(
                    (PixmapEffect*)((PixmapEffect*)mEffectsAnimators->
                           ca_getChildAt(i))->makeDuplicate() );
    }
}

Property *BoundingBox::makeDuplicate() {
    return createDuplicateWithSameParent();
}

BoundingBox *BoundingBox::createDuplicate() {
    BoundingBox *target = createNewDuplicate();
    makeDuplicate(target);
    return target;
}

BoundingBox *BoundingBox::createDuplicateWithSameParent() {
    BoundingBox *duplicate = createDuplicate();
    mParent->addChild(duplicate);
    return duplicate;
}

void BoundingBox::drawHoveredPathSk(SkCanvas *canvas,
                                    const SkPath &path,
                                    const qreal &invScale) {
    canvas->save();
    SkPath mappedPath = path;
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

bool BoundingBox::isAncestor(BoxesGroup *box) const {
    if(mParent == box) return true;
    if(mParent == NULL) return false;
    return mParent->isAncestor(box);
}

bool BoundingBox::isAncestor(BoundingBox *box) const {
    if(box->SWT_isBoxesGroup()) {
        return isAncestor((BoxesGroup*)box);
    }
    return false;
}

//#include <CImg.h>
//using namespace cimg_library;
void BoundingBox::applyEffectsSk(const SkBitmap &im,
                                 const qreal &scale) {
//    SkPixmap pixmap1;
//    im.peekPixels(&pixmap1);
//    CImg<unsigned char> cimg =
//           CImg<unsigned char>(
//                (unsigned char*)pixmap1.writable_addr(),
//                pixmap1.width(), pixmap1.height(),
//                1, 4, true);
//    cimg.blur(10.);
    if(mEffectsAnimators->hasChildAnimators()) {
        SkPixmap pixmap;
        im.peekPixels(&pixmap);
        fmt_filters::image img((uint8_t*)pixmap.writable_addr(),
                               im.width(), im.height());
        mEffectsAnimators->applyEffectsSk(im, img, scale);
    }
}

Canvas *BoundingBox::getParentCanvas() {
    if(mParent == NULL) return NULL;
    return mParent->getParentCanvas();
}

void BoundingBox::duplicateTransformAnimatorFrom(
        BoxTransformAnimator *source) {
    source->makeDuplicate(mTransformAnimator.data());
}

void BoundingBox::updateAllBoxes() {
    scheduleUpdate();
}

void BoundingBox::prp_updateInfluenceRangeAfterChanged() {
    int minAbsUpdateFrame;
    int maxAbsUpdateFrame;
    getVisibleAbsFrameRange(&minAbsUpdateFrame, &maxAbsUpdateFrame);
    prp_updateAfterChangedAbsFrameRange(minAbsUpdateFrame,
                                        maxAbsUpdateFrame);
}

void BoundingBox::clearAllCache() {
    prp_updateInfluenceRangeAfterChanged();
}

void BoundingBox::drawSelectedSk(SkCanvas *canvas,
                                 const CanvasMode &currentCanvasMode,
                                 const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

void BoundingBox::drawPixmapSk(SkCanvas *canvas) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();

        SkPaint paint;
        paint.setAlpha(qRound(mTransformAnimator->getOpacity()*2.55));
        paint.setBlendMode(mBlendModeSk);
        //paint.setFilterQuality(kHigh_SkFilterQuality);
        drawPixmapSk(canvas, &paint);
        canvas->restore();
    }
}

void BoundingBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint) {
    mDrawRenderContainer.drawSk(canvas, paint);
}

void BoundingBox::setBlendModeSk(const SkBlendMode &blendMode) {
    mBlendModeSk = blendMode;
    prp_updateInfluenceRangeAfterChanged();
}

QPainter::CompositionMode BoundingBox::getCompositionMode() {
    return mCompositionMode;
}

void BoundingBox::resetScale() {
    mTransformAnimator->resetScale(true);
}

void BoundingBox::resetTranslation() {
    mTransformAnimator->resetTranslation(true);
}

void BoundingBox::resetRotation() {
    mTransformAnimator->resetRotation(true);
}

void BoundingBox::prp_setAbsFrame(const int &frame) {
    int lastRelFrame = anim_mCurrentRelFrame;
    ComplexAnimator::prp_setAbsFrame(frame);
    if(mDurationRectangle != NULL) {
        int minDurRelFrame = mDurationRectangle->getMinFrameAsRelFrame();
        int maxDurRelFrame = mDurationRectangle->getMaxFrameAsRelFrame();
        bool isInVisRange = (anim_mCurrentRelFrame >= minDurRelFrame &&
                             anim_mCurrentRelFrame <= maxDurRelFrame);
        if(mUpdateDrawOnParentBox != isInVisRange) {
            if(mUpdateDrawOnParentBox) {
                mParent->scheduleUpdate();
            } else {
                scheduleUpdate();
            }
            mUpdateDrawOnParentBox = isInVisRange;
        }
    }
    if(prp_differencesBetweenRelFrames(lastRelFrame,
                                       anim_mCurrentRelFrame)) {
        scheduleUpdate();
    }
}

bool BoundingBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                  const int &relFrame2) {
    bool differences =
            ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1,
                                                             relFrame2);
    if(differences || mDurationRectangle == NULL) return differences;
    return mDurationRectangle->hasAnimationFrameRange();
}

void BoundingBox::setParent(BoxesGroup *parent) {
    mParent = parent;
    mTransformAnimator->setParentTransformAnimator(
                        mParent->getTransformAnimator());

    prp_setAbsFrame(mParent->anim_getCurrentAbsFrame());
    updateCombinedTransform();
}

BoxesGroup *BoundingBox::getParent() {
    return mParent;
}

void BoundingBox::disablePivotAutoAdjust() {
    mPivotAutoAdjust = false;
}

void BoundingBox::enablePivotAutoAdjust() {
    mPivotAutoAdjust = true;
}

void BoundingBox::setPivotRelPos(const QPointF &relPos,
                                 const bool &saveUndoRedo,
                                 const bool &pivotAutoAdjust) {
    mPivotAutoAdjust = pivotAutoAdjust;
    mTransformAnimator->
            setPivotWithoutChangingTransformation(relPos,
                                                  saveUndoRedo);//setPivot(relPos, saveUndoRedo);//setPivotWithoutChangingTransformation(relPos, saveUndoRedo);
    schedulePivotUpdate();
}

void BoundingBox::startPivotTransform() {
    mTransformAnimator->pivotTransformStarted();
}

void BoundingBox::finishPivotTransform() {
    mTransformAnimator->pivotTransformFinished();
}

void BoundingBox::setPivotAbsPos(const QPointF &absPos,
                                 const bool &saveUndoRedo,
                                 const bool &pivotChanged) {
    QPointF newPos = mapAbsPosToRel(absPos);
    setPivotRelPos(newPos, saveUndoRedo, pivotChanged);
    updateCombinedTransform();
}

QPointF BoundingBox::getPivotAbsPos() {
    return mTransformAnimator->getPivotAbs();
}

void BoundingBox::select() {
    mSelected = true;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

void BoundingBox::updateRelBoundingRectFromRenderData(
        BoundingBoxRenderData *renderData) {
    mRelBoundingRect = renderData->relBoundingRect;
    mRelBoundingRectSk = QRectFToSkRect(mRelBoundingRect);
    mSkRelBoundingRectPath = SkPath();
    mSkRelBoundingRectPath.addRect(mRelBoundingRectSk);

    if(mPivotAutoAdjust &&
       !mTransformAnimator->rotOrScaleOrPivotRecording()) {
        centerPivotPosition(false);
    }
}

void BoundingBox::updateCurrentPreviewDataFromRenderData(
        BoundingBoxRenderData *renderData) {
    updateRelBoundingRectFromRenderData(renderData);
}

void BoundingBox::scheduleUpdate() {
    Q_ASSERT(!mBlockedSchedule);
    if(!shouldScheduleUpdate()) return;
    if(mCurrentRenderData == NULL) {
        updateCurrentRenderData();
    } else {
        if(!mRedoUpdate) {
            mRedoUpdate = mCurrentRenderData->isAwaitingUpdate();
        }
        return;
    }
    mRedoUpdate = false;

    //mUpdateDrawOnParentBox = isVisibleAndInVisibleDurationRect();

    if(mParent != NULL) {
        mParent->scheduleUpdate();
    }
    mCurrentRenderData->addScheduler();

    emit scheduledUpdate();
}

void BoundingBox::nullifyCurrentRenderData() {
    mCurrentRenderData.reset();
}

void BoundingBox::deselect() {
    mSelected = false;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

bool BoundingBox::isContainedIn(const QRectF &absRect) {
    return absRect.contains(getCombinedTransform().mapRect(mRelBoundingRect));
}

BoundingBox *BoundingBox::getPathAtFromAllAncestors(const QPointF &absPos) {
    if(absPointInsidePath(absPos)) {
        return this;
    } else {
        return NULL;
    }
}

QPointF BoundingBox::mapAbsPosToRel(const QPointF &absPos) {
    return mTransformAnimator->mapAbsPosToRel(absPos);
}

PaintSettings *BoundingBox::getFillSettings() {
    return NULL;
}

StrokeSettings *BoundingBox::getStrokeSettings() {
    return NULL;
}

void BoundingBox::drawAsBoundingRectSk(SkCanvas *canvas,
                                       const SkPath &path,
                                       const SkScalar &invScale,
                                       const bool &dashes) {
    canvas->save();

    SkPaint paint;
    if(dashes) {
        SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
                                 MIN_WIDGET_HEIGHT*0.25f*invScale};
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
    }
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.5*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    SkPath mappedPath = path;
    mappedPath.transform(QMatrixToSkMatrix(getCombinedTransform()));
    canvas->drawPath(mappedPath, paint);
    paint.setStrokeWidth(0.75*invScale);
    paint.setColor(SK_ColorWHITE);
    canvas->drawPath(mappedPath, paint);

//    SkPaint paint;
//    SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
//                             MIN_WIDGET_HEIGHT*0.25f*invScale};
//    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
//    paint.setColor(SkColorSetARGBInline(125, 0, 0, 0));
//    paint.setStyle(SkPaint::kStroke_Style);
//    paint.setStrokeWidth(invScale);
//    SkPath mappedPath = path;
//    mappedPath.transform(QMatrixToSkMatrix(getCombinedTransform()));
//    canvas->drawPath(mappedPath, paint);

    canvas->restore();
}

void BoundingBox::drawBoundingRectSk(SkCanvas *canvas,
                                     const qreal &invScale) {
    drawAsBoundingRectSk(canvas,
                         mSkRelBoundingRectPath,
                         invScale,
                         true);
}

const SkPath &BoundingBox::getRelBoundingRectPath() {
    return mSkRelBoundingRectPath;
}

QMatrix BoundingBox::getCombinedTransform() const {
    return mTransformAnimator->getCombinedTransform();
}

QMatrix BoundingBox::getRelativeTransform() const {
    return mTransformAnimator->getRelativeTransform();
}

void BoundingBox::applyTransformation(BoxTransformAnimator *transAnimator) {
    Q_UNUSED(transAnimator);
}

void BoundingBox::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::scale(const qreal &scaleXBy,
                        const qreal &scaleYBy) {
    mTransformAnimator->scale(scaleXBy, scaleYBy);
}

void BoundingBox::rotateBy(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot);
}

void BoundingBox::rotateRelativeToSavedPivot(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot,
                                                  mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                            const qreal &scaleYBy) {
    mTransformAnimator->scaleRelativeToSavedValue(scaleXBy, scaleYBy,
                                                 mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal &scaleBy) {
    scaleRelativeToSavedPivot(scaleBy, scaleBy);
}

QPointF BoundingBox::mapRelPosToAbs(const QPointF &relPos) const {
    return mTransformAnimator->mapRelPosToAbs(relPos);
}

void BoundingBox::moveByAbs(const QPointF &trans) {
    mTransformAnimator->moveByAbs(mParent->getCombinedTransform(), trans);
//    QPointF by = mParent->mapAbsPosToRel(trans) -
//                 mParent->mapAbsPosToRel(QPointF(0., 0.));
// //    QPointF by = mapAbsPosToRel(
// //                trans - mapRelativeToAbsolute(QPointF(0., 0.)));

//    moveByRel(by);
}

void BoundingBox::moveByRel(const QPointF &trans) {
    mTransformAnimator->moveRelativeToSavedValue(trans.x(), trans.y());
}

void BoundingBox::setAbsolutePos(const QPointF &pos,
                                 const bool &saveUndoRedo) {
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo);
}

void BoundingBox::setRelativePos(const QPointF &relPos,
                                 const bool &saveUndoRedo) {
    mTransformAnimator->setPosition(relPos.x(), relPos.y(), saveUndoRedo);
}

void BoundingBox::saveTransformPivotAbsPos(const QPointF &absPivot) {
    mSavedTransformPivot =
            mParent->mapAbsPosToRel(absPivot) -
            mTransformAnimator->getPivot();
}

void BoundingBox::startPosTransform() {
    mTransformAnimator->startPosTransform();
}

void BoundingBox::startRotTransform() {
    mTransformAnimator->startRotTransform();
}

void BoundingBox::startScaleTransform() {
    mTransformAnimator->startScaleTransform();
}

void BoundingBox::startTransform() {
    mTransformAnimator->prp_startTransform();
}

void BoundingBox::finishTransform() {
    mTransformAnimator->prp_finishTransform();
    //updateCombinedTransform();
}

qreal BoundingBox::getEffectsMarginAtRelFrame(const int &relFrame) {
    return mEffectsAnimators->getEffectsMarginAtRelFrame(relFrame);
}

void BoundingBox::setupBoundingBoxRenderDataForRelFrame(
                        const int &relFrame,
                        BoundingBoxRenderData *data) {
    data->relFrame = relFrame;
    data->renderedToImage = false;
    data->relTransform = getRelativeTransformAtRelFrame(relFrame);
    data->transform = mTransformAnimator->getCombinedTransformMatrixAtRelFrame(relFrame);
    data->opacity = mTransformAnimator->getOpacityAtRelFrame(relFrame);
    data->resolution = getParentCanvas()->getResolutionFraction();
    data->effectsMargin =
            getEffectsMarginAtRelFrame(relFrame)*data->resolution + 2.;

    Canvas *parentCanvas = getParentCanvas();
    data->maxBoundsRect = parentCanvas->getMaxBoundsRect();

    data->pixmapEffects.clear();
    mEffectsAnimators->addEffectRenderDataToList(relFrame,
                                                 &data->pixmapEffects);
}

bool BoundingBox::relPointInsidePath(const QPointF &point) {
    return mRelBoundingRect.contains(point.toPoint());
}

bool BoundingBox::absPointInsidePath(const QPointF &absPoint) {
    return relPointInsidePath(mapAbsPosToRel(absPoint));
}

MovablePoint *BoundingBox::getPointAtAbsPos(const QPointF &absPtPos,
                                            const CanvasMode &currentCanvasMode,
                                            const qreal &canvasScaleInv) {
    if(currentCanvasMode == MOVE_PATH) {
        MovablePoint *pivotMovable = mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return pivotMovable;
        }
    }
    return NULL;
}

void BoundingBox::cancelTransform() {
    mTransformAnimator->prp_cancelTransform();
    //updateCombinedTransform();
}

void BoundingBox::moveUp() {
    mParent->increaseChildZInList(this);
}

void BoundingBox::moveDown() {
    mParent->decreaseChildZInList(this);
}

void BoundingBox::bringToFront() {
    mParent->bringChildToEndList(this);
}

void BoundingBox::bringToEnd() {
    mParent->bringChildToFrontList(this);
}

void BoundingBox::setZListIndex(const int &z,
                                const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new SetBoundingBoxZListIndexUnoRedo(mZListIndex, z, this));
    }
    mZListIndex = z;
}

int BoundingBox::getZIndex() {
    return mZListIndex;
}

QPointF BoundingBox::getAbsolutePos() {
    return QPointF(mTransformAnimator->getCombinedTransform().dx(),
                   mTransformAnimator->getCombinedTransform().dy());
}

void BoundingBox::updateRelativeTransformTmp() {
    updateCombinedTransformTmp();
    schedulePivotUpdate();
    scheduleUpdate();
    //updateCombinedTransform(replaceCache);
}

void BoundingBox::updateRelativeTransformAfterFrameChange() {
    updateCombinedTransformAfterFrameChange();
    schedulePivotUpdate();
}

void BoundingBox::updateCombinedTransformAfterFrameChange() {
    if(mParent == NULL) return;
    updateDrawRenderContainerTransform();


    updateAfterCombinedTransformationChangedAfterFrameChagne();
    scheduleUpdate();
}

void BoundingBox::updateDrawRenderContainerTransform() {
    mDrawRenderContainer.updatePaintTransformGivenNewCombinedTransform(
                mTransformAnimator->getCombinedTransform());
}

void BoundingBox::updateCombinedTransform() {
    if(mParent == NULL) return;
    updateDrawRenderContainerTransform();

    updateAfterCombinedTransformationChanged();
    scheduleUpdate();
}

void BoundingBox::updateCombinedTransformTmp() {
    updateCombinedTransform();
}

BoxTransformAnimator *BoundingBox::getTransformAnimator() {
    return mTransformAnimator.data();
}

void BoundingBox::selectionChangeTriggered(const bool &shiftPressed) {
    Canvas *parentCanvas = getParentCanvas();
    if(shiftPressed) {
        if(mSelected) {
            parentCanvas->removeBoxFromSelection(this);
        } else {
            parentCanvas->addBoxToSelection(this);
        }
    } else {
        parentCanvas->clearBoxesSelection();
        parentCanvas->addBoxToSelection(this);
    }
}

void BoundingBox::addEffect(PixmapEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(!mEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mEffectsAnimators.data());
    }
    mEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mEffectsAnimators.data());

    clearAllCache();
}

void BoundingBox::removeEffect(PixmapEffect *effect) {
    mEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mEffectsAnimators->hasChildAnimators()) {
        ca_removeChildAnimator(mEffectsAnimators.data());
    }

    clearAllCache();
}

int BoundingBox::prp_getParentFrameShift() const {
    if(mParent == NULL) {
        return 0;
    } else {
        return mParent->prp_getFrameShift();
    }
}

int BoundingBox::prp_getFrameShift() const {
    if(mDurationRectangle == NULL) {
        return prp_getParentFrameShift();
    } else {
        return mDurationRectangle->getFrameShift() +
                prp_getParentFrameShift();
    }
}

bool BoundingBox::hasDurationRectangle() {
    return mDurationRectangle != NULL;
}

void BoundingBox::createDurationRectangle() {
    DurationRectangle *durRect = new DurationRectangle(this);
    durRect->setMinFrame(0);
    Canvas *parentCanvas = getParentCanvas();
    if(parentCanvas != NULL) {
        durRect->setFramesDuration(getParentCanvas()->getFrameCount());
    }
    setDurationRectangle(durRect);
}

void BoundingBox::shiftAll(const int &shift) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(shift);
    } else {
        anim_shiftAllKeys(shift);
    }
}

void BoundingBox::setDurationRectangle(DurationRectangle *durationRect) {
    if(durationRect == mDurationRectangle) return;
    if(mDurationRectangle != NULL) {
        disconnect(mDurationRectangle, 0, this, 0);
    }
    DurationRectangle *oldDurRect = mDurationRectangle;
    mDurationRectangle = durationRect;
    updateAfterDurationRectangleShifted();
    if(durationRect == NULL) {
        shiftAll(oldDurRect->getFrameShift());
    }

    if(mDurationRectangle == NULL) return;
    connect(mDurationRectangle, SIGNAL(posChangedBy(int)),
            this, SLOT(updateAfterDurationRectangleShifted(int)));
    connect(mDurationRectangle, SIGNAL(rangeChanged()),
            this, SLOT(updateAfterDurationRectangleRangeChanged()));

    connect(mDurationRectangle, SIGNAL(minFrameChangedBy(int)),
            this, SLOT(updateAfterDurationMinFrameChangedBy(int)));
    connect(mDurationRectangle, SIGNAL(maxFrameChangedBy(int)),
            this, SLOT(updateAfterDurationMaxFrameChangedBy(int)));
}

void BoundingBox::updateAfterDurationRectangleShifted(const int &dFrame) {
    prp_setParentFrameShift(prp_mParentFrameShift);
    prp_setAbsFrame(anim_mCurrentAbsFrame);
    int minAbsUpdateFrame;
    int maxAbsUpdateFrame;
    getVisibleAbsFrameRange(&minAbsUpdateFrame, &maxAbsUpdateFrame);
    if(dFrame > 0) {
        prp_updateAfterChangedAbsFrameRange(minAbsUpdateFrame - dFrame,
                                            maxAbsUpdateFrame);
    } else {
        prp_updateAfterChangedAbsFrameRange(minAbsUpdateFrame,
                                            maxAbsUpdateFrame - dFrame);
    }
}

void BoundingBox::updateAfterDurationMinFrameChangedBy(const int &by) {
    int minAbsUpdateFrame;
    int maxAbsUpdateFrame;
    getVisibleAbsFrameRange(&minAbsUpdateFrame, &maxAbsUpdateFrame);
    if(by > 0) {
        prp_updateAfterChangedAbsFrameRange(minAbsUpdateFrame - by,
                                            minAbsUpdateFrame - 1);
    } else {
        prp_updateAfterChangedAbsFrameRange(minAbsUpdateFrame,
                                            minAbsUpdateFrame - by - 1);
    }
}

void BoundingBox::updateAfterDurationMaxFrameChangedBy(const int &by) {
    int minAbsUpdateFrame;
    int maxAbsUpdateFrame;
    getVisibleAbsFrameRange(&minAbsUpdateFrame, &maxAbsUpdateFrame);
    if(by > 0) {
        prp_updateAfterChangedAbsFrameRange(maxAbsUpdateFrame - by + 1,
                                            maxAbsUpdateFrame);
    } else {
        prp_updateAfterChangedAbsFrameRange(maxAbsUpdateFrame + 1,
                                            maxAbsUpdateFrame - by);
    }
}

DurationRectangleMovable *BoundingBox::anim_getRectangleMovableAtPos(
                            const qreal &relX,
                            const int &minViewedFrame,
                            const qreal &pixelsPerFrame) {
    if(mDurationRectangle == NULL) return NULL;
    return mDurationRectangle->getMovableAt(relX,
                                           pixelsPerFrame,
                                           minViewedFrame);
}

void BoundingBox::prp_drawKeys(QPainter *p,
                               const qreal &pixelsPerFrame,
                               const qreal &drawY,
                               const int &startFrame,
                               const int &endFrame) {
    if(mDurationRectangle != NULL) {
        p->save();
        p->translate(prp_getParentFrameShift()*pixelsPerFrame, 0);
        mDurationRectangle->draw(p, pixelsPerFrame,
                                drawY, startFrame);
        p->restore();
    }

    Animator::prp_drawKeys(p,
                           pixelsPerFrame, drawY,
                           startFrame, endFrame);
}

void BoundingBox::setName(const QString &name) {
    prp_mName = name;

    emit nameChanged(name);
}

QString BoundingBox::getName() {
    return prp_mName;
}

bool BoundingBox::isInVisibleDurationRect() {
    if(mDurationRectangle == NULL) return true;
    return anim_mCurrentRelFrame <= mDurationRectangle->getMaxFrameAsRelFrame() &&
           anim_mCurrentRelFrame >= mDurationRectangle->getMinFrameAsRelFrame();
}

bool BoundingBox::isVisibleAndInVisibleDurationRect() {
    return isInVisibleDurationRect() && mVisible;
}

bool BoundingBox::isRelFrameInVisibleDurationRect(const int &relFrame) {
    if(mDurationRectangle == NULL) return true;
    return relFrame <= mDurationRectangle->getMaxFrameAsRelFrame() &&
           relFrame >= mDurationRectangle->getMinFrameAsRelFrame();
}

bool BoundingBox::isRelFrameVisibleAndInVisibleDurationRect(
        const int &relFrame) {
    return isRelFrameInVisibleDurationRect(relFrame) && mVisible;
}

void BoundingBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                        int *lastIdentical,
                                                        const int &relFrame) {
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            ComplexAnimator::prp_getFirstAndLastIdenticalRelFrame(
                                        firstIdentical,
                                        lastIdentical,
                                        relFrame);
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
}

void BoundingBox::processSchedulers() {
    mBlockedSchedule = true;
    foreach(const std::shared_ptr<Updatable> &updatable, mSchedulers) {
        updatable->schedulerProccessed();
    }
}

void BoundingBox::addSchedulersToProcess() {
    foreach(const std::shared_ptr<Updatable> &updatable, mSchedulers) {
        MainWindow::getInstance()->addUpdateScheduler(updatable.get());
    }

    mSchedulers.clear();
    mBlockedSchedule = false;
}

void BoundingBox::addScheduler(Updatable *updatable) {
    mSchedulers << updatable->ref<Updatable>();
}

void BoundingBox::setVisibile(const bool &visible,
                              const bool &saveUndoRedo) {
    if(mVisible == visible) return;
    if(mSelected) {
        removeFromSelection();
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxVisibleUndoRedo(this, mVisible, visible));
    }
    mVisible = visible;

    clearAllCache();

    scheduleUpdate();

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Visible);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Invisible);
}

void BoundingBox::switchVisible() {
    setVisibile(!mVisible);
}

void BoundingBox::switchLocked() {
    setLocked(!mLocked);
}

void BoundingBox::hide()
{
    setVisibile(false);
}

void BoundingBox::show()
{
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() {
    return mVisible && !mLocked;
}

bool BoundingBox::isVisible()
{
    return mVisible;
}

bool BoundingBox::isLocked() {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(const bool &bt) {
    if(bt == mLocked) return;
    if(mSelected) {
        getParentCanvas()->removeBoxFromSelection(this);
    }
    mLocked = bt;
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Locked);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Unlocked);
}

bool BoundingBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &parentMainTarget) {
    const SWT_Rule &rule = rules.rule;
    bool satisfies;
    bool alwaysShowChildren = rules.alwaysShowChildren;
    if(rules.type == &SingleWidgetTarget::SWT_isSingleSound) return false;
    if(alwaysShowChildren) {
        if(rule == SWT_NoRule) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_Selected) {
            satisfies = isSelected() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Animated) {
            satisfies = isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_NotAnimated) {
            satisfies = !isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Visible) {
            satisfies = isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Invisible) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Locked) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Unlocked) {
            satisfies = !isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        }
    } else {
        if(rule == SWT_NoRule) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_Selected) {
            satisfies = isSelected();
        } else if(rule == SWT_Animated) {
            satisfies = isAnimated();
        } else if(rule == SWT_NotAnimated) {
            satisfies = !isAnimated();
        } else if(rule == SWT_Visible) {
            satisfies = isVisible() && parentSatisfies;
        } else if(rule == SWT_Invisible) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Locked) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Unlocked) {
            satisfies = !isLocked() && parentSatisfies;
        }
    }
    if(satisfies) {
        const QString &nameSearch = rules.searchString;
        if(!nameSearch.isEmpty()) {
            satisfies = prp_mName.contains(nameSearch);
        }
    }
    return satisfies;
}

void BoundingBox::SWT_addToContextMenu(
        QMenu *menu) {
    menu->addAction("Apply Transformation");
    menu->addAction("Create Link");
    menu->addAction("Center Pivot");
    menu->addAction("Copy");
    menu->addAction("Cut");
    menu->addAction("Duplicate");
    menu->addAction("Group");
    menu->addAction("Ungroup");
    menu->addAction("Delete");

    QMenu *effectsMenu = menu->addMenu("Effects");
    effectsMenu->addAction("Blur");
    effectsMenu->addAction("Shadow");
//            effectsMenu->addAction("Brush");
    effectsMenu->addAction("Lines");
    effectsMenu->addAction("Circles");
    effectsMenu->addAction("Swirl");
    effectsMenu->addAction("Oil");
    effectsMenu->addAction("Implode");
    effectsMenu->addAction("Desaturate");
}

void BoundingBox::removeFromParent() {
    mParent->removeChild(this);
}

void BoundingBox::removeFromSelection() {
    if(mSelected) {
        getParentCanvas()->removeBoxFromSelection(this);
    }
}

bool BoundingBox::SWT_handleContextMenuActionSelected(
        QAction *selectedAction) {
    if(selectedAction != NULL) {
        if(selectedAction->text() == "Delete") {
            mParent->removeChild(this);
        } else if(selectedAction->text() == "Apply Transformation") {
            applyCurrentTransformation();
        } else if(selectedAction->text() == "Create Link") {
            mParent->addChild(createLink());
        } else if(selectedAction->text() == "Group") {
            getParentCanvas()->groupSelectedBoxes();
            return true;
//        } else if(selectedAction->text() == "Ungroup") {
//            ungroupSelected();
//        } else if(selectedAction->text() == "Center Pivot") {
//            mCurrentBoxesGroup->centerPivotForSelected();
//        } else if(selectedAction->text() == "Blur") {
//            mCurrentBoxesGroup->applyBlurToSelected();
//        } else if(selectedAction->text() == "Shadow") {
//            mCurrentBoxesGroup->applyShadowToSelected();
//        } else if(selectedAction->text() == "Brush") {
//            mCurrentBoxesGroup->applyBrushEffectToSelected();
//        } else if(selectedAction->text() == "Lines") {
//            mCurrentBoxesGroup->applyLinesEffectToSelected();
//        } else if(selectedAction->text() == "Circles") {
//            mCurrentBoxesGroup->applyCirclesEffectToSelected();
//        } else if(selectedAction->text() == "Swirl") {
//            mCurrentBoxesGroup->applySwirlEffectToSelected();
//        } else if(selectedAction->text() == "Oil") {
//            mCurrentBoxesGroup->applyOilEffectToSelected();
//        } else if(selectedAction->text() == "Implode") {
//            mCurrentBoxesGroup->applyImplodeEffectToSelected();
//        } else if(selectedAction->text() == "Desaturate") {
//            mCurrentBoxesGroup->applyDesaturateEffectToSelected();
        }
    } else {

    }
    return false;
}

void BoundingBox::renderDataFinished(BoundingBoxRenderData *renderData) {
    if(mRedoUpdate) {
        scheduleUpdate();
    }
    mDrawRenderContainer.setVariablesFromRenderData(renderData);
    updateDrawRenderContainerTransform();
}

void BoundingBox::updateCurrentRenderData() {
    mCurrentRenderData = createRenderData()->ref<BoundingBoxRenderData>();
}

BoundingBoxRenderData *BoundingBox::getCurrentRenderData() {
    if(mCurrentRenderData == NULL) {
        return mDrawRenderContainer.getSrcRenderData();
    }
    return mCurrentRenderData.get();
}

void BoundingBox::getVisibleAbsFrameRange(int *minFrame, int *maxFrame) {
    if(mDurationRectangle == NULL) {
        *minFrame = INT_MIN;
        *maxFrame = INT_MAX;
    } else {
        *minFrame = mDurationRectangle->getMinFrameAsAbsFrame();
        *maxFrame = mDurationRectangle->getMaxFrameAsAbsFrame();
    }
}

BoundingBoxRenderData::BoundingBoxRenderData(BoundingBox *parentBoxT) {
    parentBox = parentBoxT->weakRef<BoundingBox>();
}

BoundingBoxRenderData::~BoundingBoxRenderData() {}

void BoundingBoxRenderData::updateRelBoundingRect() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == NULL) return;
    relBoundingRect = parentBoxT->getRelBoundingRectAtRelFrame(relFrame);
}

void BoundingBoxRenderData::drawRenderedImageForParent(SkCanvas *canvas) {
    canvas->save();
    canvas->scale(1.f/resolution, 1.f/resolution);
    renderToImage();
    SkPaint paint;
    paint.setAlpha(qRound(opacity*2.55));
    paint.setBlendMode(blendMode);
    canvas->drawImage(renderedImage,
                      drawPos.x(), drawPos.y(),
                      &paint);
    canvas->restore();
}

void BoundingBoxRenderData::renderToImage() {
    if(renderedToImage) return;
    renderedToImage = true;
    QMatrix scale;
    scale.scale(resolution, resolution);
    QMatrix transformRes = transform*scale;
    //transformRes.scale(resolution, resolution);
    QRectF allUglyBoundingRect =
            transformRes.mapRect(relBoundingRect).
            adjusted(-effectsMargin, -effectsMargin,
                     effectsMargin, effectsMargin);
    allUglyBoundingRect = allUglyBoundingRect.intersected(
                          scale.mapRect(maxBoundsRect));
    QSizeF sizeF = allUglyBoundingRect.size();
    QPointF transF = allUglyBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(allUglyBoundingRect.left()/**resolution*/),
                    qRound(allUglyBoundingRect.top()/**resolution*/));

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

    rasterCanvas->translate(-allUglyBoundingRect.left(),
                            -allUglyBoundingRect.top());

    allUglyBoundingRect.translate(-transF);

    rasterCanvas->translate(transF.x(), transF.y());
    rasterCanvas->concat(QMatrixToSkMatrix(transformRes));

    drawSk(rasterCanvas);
    rasterCanvas->flush();
    delete rasterCanvas;

    drawPos = SkPoint::Make(qRound(allUglyBoundingRect.left()),
                            qRound(allUglyBoundingRect.top()));

    if(!pixmapEffects.isEmpty()) {
        SkPixmap pixmap;
        bitmap.peekPixels(&pixmap);
        fmt_filters::image img((uint8_t*)pixmap.writable_addr(),
                               pixmap.width(), pixmap.height());
        foreach(PixmapEffectRenderData *effect, pixmapEffects) {
            effect->applyEffectsSk(bitmap, img, resolution);
        }
    }

    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}

void BoundingBoxRenderData::processUpdate() {
    renderToImage();
}

void BoundingBoxRenderData::beforeUpdate() {
    if(!mDataSet) {
        dataSet();
    }
    Updatable::beforeUpdate();
    //parentBox->setUpdateVars();
//    parentBox->setupBoundingBoxRenderDataForRelFrame(
//                parentBox->anim_getCurrentRelFrame(), this);
//    parentBox->updateCurrentPreviewDataFromRenderData(this);

    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == NULL) return;
    parentBoxT->nullifyCurrentRenderData();
}

void BoundingBoxRenderData::afterUpdate() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT != NULL) {
        parentBoxT->renderDataFinished(this);
    }
    Updatable::afterUpdate();
}

void BoundingBoxRenderData::schedulerProccessed() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT != NULL) {
        parentBoxT->setupBoundingBoxRenderDataForRelFrame(
                    parentBoxT->anim_getCurrentRelFrame(),
                    this);
    }
    mDataSet = false;
    dataSet();
    Updatable::schedulerProccessed();
}

void BoundingBoxRenderData::addSchedulerNow() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == NULL) return;
    parentBoxT->addScheduler(this);
}

void BoundingBoxRenderData::dataSet() {
    if(allDataReady()) {
        mDataSet = true;
        updateRelBoundingRect();
        BoundingBox *parentBoxT = parentBox.data();
        if(parentBoxT == NULL) return;
        parentBoxT->updateCurrentPreviewDataFromRenderData(this);
    }
}
