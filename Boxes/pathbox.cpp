#include "Boxes/pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"
#include "gradientpoint.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"
#include "skiaincludes.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
#include "canvas.h"
#include "vectorpath.h"

PathBox::PathBox(const BoundingBoxType &type) :
    BoundingBox(type) {
    mPathEffectsAnimators =
            (new PathEffectAnimators(false, false, this))->
            ref<PathEffectAnimators>();
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setBlockedUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mFillPathEffectsAnimators =
            (new PathEffectAnimators(false, true, this))->
            ref<PathEffectAnimators>();
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setBlockedUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mOutlinePathEffectsAnimators =
            (new PathEffectAnimators(true, false, this))->
            ref<PathEffectAnimators>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
                SPtrCreate(NodePointUpdater)(this));

//    mPathEffectsAnimators->prp_setName("path effects");
//    mPathEffectsAnimators->prp_setBlockedUpdater(
//                SPtrCreate(PixmapEffectUpdater)(this));

//    mOutlinePathEffectsAnimators->prp_setName("outline path effects");
//    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
//                SPtrCreate(PixmapEffectUpdater)(this));

    mStrokeGradientPoints = SPtrCreate(GradientPoints)(this);
    mFillGradientPoints = SPtrCreate(GradientPoints)(this);
    mFillSettings = SPtrCreate(PaintSettings)(this);
    mStrokeSettings = SPtrCreate(StrokeSettings)(this);
    ca_addChildAnimator(mFillSettings);
    ca_addChildAnimator(mStrokeSettings);
    ca_moveChildAbove(mEffectsAnimators.data(),
                      mStrokeSettings.data(),
                      false);
    ca_addChildAnimator(mPathEffectsAnimators);
    ca_addChildAnimator(mFillPathEffectsAnimators);
    ca_addChildAnimator(mOutlinePathEffectsAnimators);

    mFillGradientPoints->prp_setBlockedUpdater(
                SPtrCreate(GradientPointsUpdater)(true, this));
    mStrokeGradientPoints->prp_setBlockedUpdater(
                SPtrCreate(GradientPointsUpdater)(false, this));

    mFillSettings->setGradientPoints(mFillGradientPoints.data());
    mStrokeSettings->setGradientPoints(mStrokeGradientPoints.data());

    mStrokeSettings->setLineWidthUpdaterTarget(this);
    mFillSettings->setPaintPathTarget(this);
}

PathBox::~PathBox() {
    if(mFillSettings->getGradient() != nullptr) {
        mFillSettings->getGradient()->removePath(this);
    }
    if(mStrokeSettings->getGradient() != nullptr) {
        mStrokeSettings->getGradient()->removePath(this);
    }
}

void PathBox::drawSelectedSk(SkCanvas *canvas,
                             const CanvasMode &currentCanvasMode,
                             const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mFillGradientPoints->drawGradientPointsSk(canvas, invScale);
            mStrokeGradientPoints->drawGradientPointsSk(canvas, invScale);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

void PathBox::setupBoundingBoxRenderDataForRelFrameF(
                            const qreal &relFrame,
                            BoundingBoxRenderData* data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame,
                                                        data);

    bool currentEditPathCompatible = false;
    bool currentPathCompatible = false;
    bool currentOutlinePathCompatible = false;
    bool currentFillPathCompatible = false;

    if(mCurrentPathsFrame != INT_MIN) {
        currentEditPathCompatible =
                !differenceInEditPathBetweenFrames(data->relFrame, mCurrentPathsFrame);
        if(currentEditPathCompatible) {
            currentPathCompatible =
                    !differenceInPathBetweenFrames(data->relFrame, mCurrentPathsFrame);
            if(currentPathCompatible) {
                currentOutlinePathCompatible =
                        !differenceInOutlinePathBetweenFrames(data->relFrame, mCurrentPathsFrame);
                currentFillPathCompatible =
                        !differenceInFillPathBetweenFrames(data->relFrame, mCurrentPathsFrame);
            }
        }
    }

    auto pathData = data->ref<PathBoxRenderData>();
    if(currentEditPathCompatible) {
        pathData->editPath = mEditPathSk;
    } else {
        pathData->editPath = getPathAtRelFrameF(relFrame);
    }
    if(currentPathCompatible) {
        pathData->path = mPathSk;
    } else {
        pathData->path = pathData->editPath;
        if(getParentCanvas()->getPathEffectsVisible()) {
            // !!! reversed
            //mPathEffectsAnimators->filterPathForRelFrameF(relFrame, &pathData->path);
            qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                        prp_relFrameToAbsFrameF(relFrame));
            mParentGroup->filterPathForRelFrameF(parentRelFrame, &pathData->path,
                                                data->parentBox.data());
            // !!! reversed
            mPathEffectsAnimators->filterPathForRelFrameF(relFrame, &pathData->path);
        }
    }

    if(currentOutlinePathCompatible) {
        pathData->outlinePath = mOutlinePathSk;
    } else {
        SkPath outline;
        if(mStrokeSettings->nonZeroLineWidth()) {
            SkPath outlineBase = pathData->path;
            mOutlinePathEffectsAnimators->filterPathForRelFrameBeforeThicknessF(
                        relFrame, &outlineBase);
            mParentGroup->filterOutlinePathBeforeThicknessForRelFrameF(
                        relFrame, &outlineBase);
            SkStroke strokerSk;
            mStrokeSettings->setStrokerSettingsForRelFrameSkF(relFrame, &strokerSk);
            outline = SkPath();
            strokerSk.strokePath(outlineBase, &outline);
        } else {
            outline = SkPath();
        }
        if(getParentCanvas()->getPathEffectsVisible()) {
            mOutlinePathEffectsAnimators->filterPathForRelFrameF(relFrame, &outline);
            mParentGroup->filterOutlinePathForRelFrameF(relFrame, &outline);
        }
        pathData->outlinePath = outline;
        outline.addPath(pathData->path);
    }

    if(currentFillPathCompatible) {
        pathData->fillPath = mFillPathSk;
    } else {
        pathData->fillPath = pathData->path;
        mFillPathEffectsAnimators->filterPathForRelFrameF(relFrame, &pathData->path);
        mParentGroup->filterFillPathForRelFrameF(relFrame, &pathData->path);
    }

    if(currentOutlinePathCompatible && currentFillPathCompatible) {
        data->relBoundingRectSet = true;
        data->relBoundingRect = mRelBoundingRect;
    }

    UpdatePaintSettings *fillSettings = &pathData->paintSettings;

    fillSettings->paintColor = mFillSettings->
            getColorAtRelFrameF(relFrame);
    fillSettings->paintType = mFillSettings->getPaintType();
    Gradient *grad = mFillSettings->getGradient();
    if(grad != nullptr) {
        fillSettings->updateGradient(
                    grad->getQGradientStopsAtAbsFrameF(
                        prp_relFrameToAbsFrameF(relFrame)),
                    mFillGradientPoints->getStartPointAtRelFrameF(relFrame),
                    mFillGradientPoints->getEndPointAtRelFrameF(relFrame),
                    mFillSettings->getGradientLinear());
    }

    UpdateStrokeSettings *strokeSettings = &pathData->strokeSettings;
    strokeSettings->paintColor = mStrokeSettings->
            getColorAtRelFrameF(relFrame);
    strokeSettings->paintType = mStrokeSettings->getPaintType();
    grad = mStrokeSettings->getGradient();
    if(grad != nullptr) {
        strokeSettings->updateGradient(
                    grad->getQGradientStopsAtAbsFrameF(
                        prp_relFrameToAbsFrameF(relFrame)),
                    mStrokeGradientPoints->getStartPointAtRelFrameF(relFrame),
                    mStrokeGradientPoints->getEndPointAtRelFrameF(relFrame),
                    mStrokeSettings->getGradientLinear());
    }
}

MovablePoint *PathBox::getPointAtAbsPos(const QPointF &absPtPos,
                                        const CanvasMode &currentCanvasMode,
                                        const qreal &canvasScaleInv) {
    MovablePoint* pointToReturn = nullptr;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints->qra_getPointAt(absPtPos,
                                                              canvasScaleInv);
        if(pointToReturn == nullptr) {
            pointToReturn = mFillGradientPoints->qra_getPointAt(absPtPos,
                                                                canvasScaleInv);
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        MovablePoint* pivotMovable =
                mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return pivotMovable;
        }
    }
    return pointToReturn;
}

void PathBox::drawBoundingRectSk(SkCanvas *canvas,
                                 const SkScalar &invScale) {
    BoundingBox::drawBoundingRectSk(canvas, invScale);
    drawAsBoundingRectSk(canvas, mEditPathSk, invScale, false);
}

void PathBox::addPathEffect(const PathEffectQSPtr& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));

    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_show();
    }
    mPathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mPathEffectsAnimators.data());

    clearAllCache();
}

void PathBox::addFillPathEffect(const PathEffectQSPtr& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_show();
    }
    mFillPathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mFillPathEffectsAnimators.data());

    clearAllCache();
}

void PathBox::addOutlinePathEffect(const PathEffectQSPtr& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_show();
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mOutlinePathEffectsAnimators.data());

    clearAllCache();
}

void PathBox::removePathEffect(const PathEffectQSPtr& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
}

void PathBox::removeFillPathEffect(const PathEffectQSPtr& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mFillPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
}

void PathBox::removeOutlinePathEffect(const PathEffectQSPtr& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mOutlinePathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        mOutlinePathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
}

void PathBox::resetStrokeGradientPointsPos() {
    mStrokeGradientPoints->prp_setRecording(false);
    mStrokeGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                        mRelBoundingRect.bottomRight());
}

void PathBox::resetFillGradientPointsPos() {
    mFillGradientPoints->prp_setRecording(false);
    mFillGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                      mRelBoundingRect.bottomRight());
}

void PathBox::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    mStrokeSettings->setCapStyle(capStyle);
    clearAllCache();
    scheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    mStrokeSettings->setJoinStyle(joinStyle);
    clearAllCache();
    scheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::setStrokeWidth(const qreal &strokeWidth, const bool &finish) {
    mStrokeSettings->setCurrentStrokeWidth(strokeWidth);
    if(finish) {
        mStrokeSettings->getStrokeWidthAnimator()->prp_finishTransform();
    }
    //scheduleOutlinePathUpdate();
}

void PathBox::setOutlineCompositionMode(
        const QPainter::CompositionMode &compositionMode) {
    mStrokeSettings->setOutlineCompositionMode(compositionMode);
    clearAllCache();
    scheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::startSelectedStrokeWidthTransform() {
    mStrokeSettings->getStrokeWidthAnimator()->prp_startTransform();
}

void PathBox::startSelectedStrokeColorTransform() {
    mStrokeSettings->getColorAnimator()->prp_startTransform();
}

void PathBox::startSelectedFillColorTransform() {
    mFillSettings->getColorAnimator()->prp_startTransform();
}


GradientPoints *PathBox::getStrokeGradientPoints() {
    return mStrokeGradientPoints.data();
}

SkPath PathBox::getPathWithThisOnlyEffectsAtRelFrame(const int &relFrame) {
    SkPath path = getPathAtRelFrame(relFrame);
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, &path);
    return path;
}

SkPath PathBox::getPathWithEffectsUntilGroupSumAtRelFrame(const int &relFrame) {
    SkPath path = getPathAtRelFrame(relFrame);
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, &path);
    if(mParentGroup == nullptr) return path;
    int parentRelFrame = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    mParentGroup->filterPathForRelFrameUntilGroupSum(parentRelFrame, &path, this);
    return path;
}

SkPath PathBox::getPathWithThisOnlyEffectsAtRelFrameF(const qreal &relFrame) {
    SkPath path = getPathAtRelFrameF(relFrame);
    mPathEffectsAnimators->filterPathForRelFrameF(relFrame, &path);
    return path;
}


void PathBox::getMotionBlurProperties(QList<Property *> &list) {
    BoundingBox::getMotionBlurProperties(list);
    list.append(mPathEffectsAnimators.get());
    list.append(mFillPathEffectsAnimators.get());
    list.append(mOutlinePathEffectsAnimators.get());
}

SkPath PathBox::getPathWithEffectsUntilGroupSumAtRelFrameF(const qreal &relFrame) {
    SkPath path = getPathAtRelFrameF(relFrame);
    mPathEffectsAnimators->filterPathForRelFrameF(relFrame, &path);
    if(mParentGroup == nullptr) return path;
    qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterPathForRelFrameUntilGroupSumF(parentRelFrame, &path);
    return path;
}


GradientPoints *PathBox::getFillGradientPoints() {
    return mFillGradientPoints.data();
}

void PathBox::duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                         StrokeSettings *strokeSettings) {
    duplicateFillSettingsFrom(fillSettings);
    duplicateStrokeSettingsFrom(strokeSettings);
}

void PathBox::duplicateFillSettingsFrom(PaintSettings *fillSettings) {
    if(fillSettings == nullptr) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        fillSettings->writeProperty(&buffer);
        if(buffer.reset()) {
            mFillSettings->readProperty(&buffer);
        }
        buffer.close();
    }
}

void PathBox::duplicateStrokeSettingsFrom(StrokeSettings *strokeSettings) {
    if(strokeSettings == nullptr) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        strokeSettings->writeProperty(&buffer);
        if(buffer.reset()) {
            mStrokeSettings->readProperty(&buffer);
        }
        buffer.close();
    }
}

void PathBox::duplicateFillSettingsNotAnimatedFrom(PaintSettings *fillSettings) {
    if(fillSettings == nullptr) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        PaintType paintType = fillSettings->getPaintType();
        mFillSettings->setPaintType(paintType);
        if(paintType == FLATPAINT) {
            mFillSettings->setCurrentColor(
                        fillSettings->getCurrentColor());
        } else if(paintType == GRADIENTPAINT) {
            mFillSettings->setGradient(
                        fillSettings->getGradient());
            mFillSettings->setGradientLinear(
                        fillSettings->getGradientLinear());
        }
    }
}

void PathBox::duplicateStrokeSettingsNotAnimatedFrom(StrokeSettings *strokeSettings) {
    if(strokeSettings == nullptr) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        PaintType paintType = strokeSettings->getPaintType();
        mStrokeSettings->setPaintType(paintType);
        if(paintType == FLATPAINT) {
            mStrokeSettings->getColorAnimator()->qra_setCurrentValue(
                        strokeSettings->getCurrentColor(), true, true);
        } else if(paintType == GRADIENTPAINT) {
            mStrokeSettings->setGradient(
                        strokeSettings->getGradient());
            mStrokeSettings->setGradientLinear(
                        strokeSettings->getGradientLinear());
        }
        mStrokeSettings->getStrokeWidthAnimator()->qra_setCurrentValue(
                    strokeSettings->getCurrentStrokeWidth(), true, true);
    }
}

void PathBox::drawHoveredSk(SkCanvas *canvas,
                            const SkScalar &invScale) {
    drawHoveredPathSk(canvas, mPathSk, invScale);
}

void PathBox::applyPaintSetting(const PaintSetting &setting) {
    setting.apply(this);
    scheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::setFillColorMode(const ColorMode &colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::setStrokeColorMode(const ColorMode &colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::copyPathBoxDataTo(PathBox *targetBox) {
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    PathBox::writeBoundingBox(&buffer);
    if(buffer.seek(sizeof(BoundingBoxType)) ) {
        targetBox->PathBox::readBoundingBox(&buffer);
    }
    buffer.close();
}

bool PathBox::differenceInPathBetweenFrames(const int &frame1, const int &frame2) const {
    return mPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2);
}

bool PathBox::differenceInOutlinePathBetweenFrames(const int &frame1, const int &frame2) const {
    if(mStrokeSettings->getLineWidthAnimator()->
       prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2);
}

bool PathBox::differenceInFillPathBetweenFrames(const int &frame1, const int &frame2) const {
    return mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2);
}

#include "circle.h"
VectorPath *PathBox::objectToVectorPathBox() {
    VectorPath *newPath = new VectorPath();
    if(SWT_isCircle()) {
        QPainterPath pathT;
        Circle *circleT = (Circle*)this;
        pathT.addEllipse(QPointF(0., 0.),
                         circleT->getCurrentXRadius(),
                         circleT->getCurrentYRadius());
        newPath->loadPathFromSkPath(QPainterPathToSkPath(pathT));
    } else {
        newPath->loadPathFromSkPath(mEditPathSk);
    }
    copyPathBoxDataTo(newPath);
    mParentGroup->addContainedBox(newPath);
    return newPath;
}

VectorPath *PathBox::strokeToVectorPathBox() {
    if(mOutlinePathSk.isEmpty()) return nullptr;
    VectorPath *newPath = new VectorPath();
    copyPathBoxDataTo(newPath);
    mParentGroup->addContainedBox(newPath);
    return newPath;
}

const SkPath &PathBox::getRelativePath() const { return mPathSk; }

void PathBox::updateFillDrawGradient() {
    if(mFillSettings->getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mFillSettings->getGradient();

        mFillGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                       gradient->getLastQGradientStopQColor());
        if(!mFillGradientPoints->enabled()) {
            mFillGradientPoints->enable();
        }
    } else if(mFillGradientPoints->enabled()) {
        mFillGradientPoints->disable();
    }
}

void PathBox::updateStrokeDrawGradient() {
    if(mStrokeSettings->getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings->getGradient();

        mStrokeGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                         gradient->getLastQGradientStopQColor() );

        if(!mStrokeGradientPoints->enabled()) {
            mStrokeGradientPoints->enable();
        }
    } else if(mStrokeGradientPoints->enabled()) {
        mStrokeGradientPoints->disable();
    }
}

void PathBox::updateDrawGradients() {
    updateFillDrawGradient();
    updateStrokeDrawGradient();
}

QRectF PathBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    SkPath path = getPathAtRelFrame(relFrame);
    SkPath outline;
    if(mStrokeSettings->nonZeroLineWidth()) {
        SkStroke strokerSk;
        mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
        outline = SkPath();
        strokerSk.strokePath(path, &outline);
    } else {
        outline = SkPath();
    }
    mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, &outline);
    outline.addPath(path);
    return SkRectToQRectF(outline.computeTightBounds());
}

void PathBox::updateCurrentPreviewDataFromRenderData(
        const BoundingBoxRenderDataSPtr& renderData) {
    auto pathRenderData = renderData->ref<PathBoxRenderData>();
    mCurrentPathsFrame = renderData->relFrame;
    mEditPathSk = pathRenderData->editPath;
    mPathSk = pathRenderData->path;
    mOutlinePathSk = pathRenderData->outlinePath;
    mFillPathSk = pathRenderData->fillPath;
    BoundingBox::updateCurrentPreviewDataFromRenderData(renderData);
}

bool PathBox::relPointInsidePath(const QPointF &relPos) {
    if(mSkRelBoundingRectPath.contains(relPos.x(), relPos.y()) ) {
        if(mFillPathSk.contains(relPos.x(), relPos.y())) {
            return true;
        }
        return mOutlinePathSk.contains(relPos.x(), relPos.y());
    } else {
        return false;
    }
}

void PathBox::setOutlineAffectedByScale(const bool &bT) {
    mOutlineAffectedByScale = bT;
    scheduleUpdate(Animator::USER_CHANGE);
}

PaintSettings *PathBox::getFillSettings() {
    return mFillSettings.data();
}

StrokeSettings *PathBox::getStrokeSettings() {
    return mStrokeSettings.data();
}
