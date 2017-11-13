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
                new NodePointUpdater(this));

    mFillPathEffectsAnimators =
            (new PathEffectAnimators(false, true, this))->
            ref<PathEffectAnimators>();
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setBlockedUpdater(
                new NodePointUpdater(this));

    mOutlinePathEffectsAnimators =
            (new PathEffectAnimators(true, false, this))->
            ref<PathEffectAnimators>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
                new NodePointUpdater(this));

//    mPathEffectsAnimators->prp_setName("path effects");
//    mPathEffectsAnimators->prp_setBlockedUpdater(
//                new PixmapEffectUpdater(this));

//    mOutlinePathEffectsAnimators->prp_setName("outline path effects");
//    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
//                new PixmapEffectUpdater(this));

    mStrokeGradientPoints =
            (new GradientPoints)->ref<GradientPoints>();
    mFillGradientPoints =
            (new GradientPoints)->ref<GradientPoints>();
    mFillSettings->setTargetPathBox(this);
    mStrokeSettings->setTargetPathBox(this);

    ca_addChildAnimator(mFillSettings.data());
    ca_addChildAnimator(mStrokeSettings.data());

    mFillGradientPoints->initialize(this);
    mStrokeGradientPoints->initialize(this);

    mFillGradientPoints->prp_setBlockedUpdater(
                new GradientPointsUpdater(true, this));
    mStrokeGradientPoints->prp_setBlockedUpdater(
                new GradientPointsUpdater(false, this));

    mFillSettings->setGradientPoints(mFillGradientPoints.data());
    mStrokeSettings->setGradientPoints(mStrokeGradientPoints.data());

    mStrokeSettings->setLineWidthUpdaterTarget(this);
    mFillSettings->setPaintPathTarget(this);
}

PathBox::~PathBox() {
    if(mFillSettings->getGradient() != NULL) {
        mFillSettings->getGradient()->removePath(this);
    }
    if(mStrokeSettings->getGradient() != NULL) {
        mStrokeSettings->getGradient()->removePath(this);
    }
}

Property *PathBox::ca_getFirstDescendantWithName(const QString &name) {
    Property *propT = BoundingBox::ca_getFirstDescendantWithName(name);
    if(propT != NULL) return propT;
    if(name == mOutlinePathEffectsAnimators->prp_getName()) {
        return mOutlinePathEffectsAnimators.data();
    }
    if(name == mFillPathEffectsAnimators->prp_getName()) {
        return mFillPathEffectsAnimators.data();
    }
    if(name == mPathEffectsAnimators->prp_getName()) {
        return mPathEffectsAnimators.data();
    }
    return NULL;
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

void PathBox::setupBoundingBoxRenderDataForRelFrame(
                            const int &relFrame,
                            BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                       data);
    PathBoxRenderData *pathData = (PathBoxRenderData*)data;
    pathData->editPath = getPathAtRelFrame(relFrame);
    pathData->path = pathData->editPath;
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, &pathData->path);
    //prp_thisRelFrameToParentRelFrame(relFrame);

    SkPath outline;
    if(mStrokeSettings->nonZeroLineWidth()) {
        SkPath outlineBase = pathData->path;
        mOutlinePathEffectsAnimators->filterPathForRelFrameBeforeThickness(relFrame, &outlineBase);
        SkStroke strokerSk;
        mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
        outline = SkPath();
        strokerSk.strokePath(outlineBase, &outline);
    } else {
        outline = SkPath();
    }
    mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, &outline);
    pathData->outlinePath = outline;
    outline.addPath(pathData->path);

    mFillPathEffectsAnimators->filterPathForRelFrame(relFrame, &pathData->path);

    UpdatePaintSettings *fillSettings = &pathData->paintSettings;

    fillSettings->paintColor = mFillSettings->
            getColorAtRelFrame(relFrame).qcol;
    fillSettings->paintType = mFillSettings->getPaintType();
    Gradient *grad = mFillSettings->getGradient();
    if(grad != NULL) {
        fillSettings->updateGradient(
                    grad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrame(relFrame)),
                    mFillGradientPoints->getStartPointAtRelFrame(relFrame),
                    mFillGradientPoints->getEndPointAtRelFrame(relFrame),
                    mFillSettings->getGradientLinear());
    }

    UpdateStrokeSettings *strokeSettings = &pathData->strokeSettings;
    strokeSettings->paintColor = mStrokeSettings->
            getColorAtRelFrame(relFrame).qcol;
    strokeSettings->paintType = mStrokeSettings->getPaintType();
    grad = mStrokeSettings->getGradient();
    if(grad != NULL) {
        strokeSettings->updateGradient(
                    grad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrame(relFrame)),
                    mStrokeGradientPoints->getStartPointAtRelFrame(relFrame),
                    mStrokeGradientPoints->getEndPointAtRelFrame(relFrame),
                    mStrokeSettings->getGradientLinear());
    }
}

MovablePoint *PathBox::getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints->qra_getPointAt(absPtPos,
                                                              canvasScaleInv);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints->qra_getPointAt(absPtPos,
                                                                canvasScaleInv);
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        MovablePoint *pivotMovable = mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return pivotMovable;
        }
    }
    return pointToReturn;
}

void PathBox::drawBoundingRectSk(SkCanvas *canvas,
                                 const qreal &invScale) {
    BoundingBox::drawBoundingRectSk(canvas, invScale);
    drawAsBoundingRectSk(canvas, mEditPathSk, invScale, false);
}

void PathBox::addPathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mPathEffectsAnimators.data());
    }
    mPathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mPathEffectsAnimators.data());

    clearAllCache();
}

void PathBox::addFillPathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mFillPathEffectsAnimators.data());
    }
    mFillPathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mFillPathEffectsAnimators.data());

    clearAllCache();
}

void PathBox::addOutlinePathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        incReasonsNotToApplyUglyTransform();
    }
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mOutlinePathEffectsAnimators.data());
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mOutlinePathEffectsAnimators.data());

    clearAllCache();
}

void PathBox::removePathEffect(PathEffect *effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        ca_removeChildAnimator(mPathEffectsAnimators.data());
    }

    clearAllCache();
}

void PathBox::removeFillPathEffect(PathEffect *effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mFillPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        ca_removeChildAnimator(mFillPathEffectsAnimators.data());
    }

    clearAllCache();
}

void PathBox::removeOutlinePathEffect(PathEffect *effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mOutlinePathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        ca_removeChildAnimator(mOutlinePathEffectsAnimators.data());
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
    scheduleUpdate();
}

void PathBox::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    mStrokeSettings->setJoinStyle(joinStyle);
    clearAllCache();
    scheduleUpdate();
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
    scheduleUpdate();
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

void PathBox::duplicateGradientPointsFrom(
        GradientPoints *fillGradientPoints,
        GradientPoints *strokeGradientPoints) {
    duplicateFillGradientPointsFrom(fillGradientPoints);
    duplicateStrokeGradientPointsFrom(strokeGradientPoints);
}

GradientPoints *PathBox::getStrokeGradientPoints() {
    return mStrokeGradientPoints.data();
}

GradientPoints *PathBox::getFillGradientPoints() {
    return mFillGradientPoints.data();
}

void PathBox::duplicateStrokeGradientPointsFrom(
        GradientPoints *strokeGradientPoints) {
    if(strokeGradientPoints != NULL) {
        strokeGradientPoints->makeDuplicate(mStrokeGradientPoints.data());
    }
}

void PathBox::duplicateFillGradientPointsFrom(
        GradientPoints *fillGradientPoints) {
    if(fillGradientPoints != NULL) {
        fillGradientPoints->makeDuplicate(mFillGradientPoints.data());
    }
}

void PathBox::duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                         StrokeSettings *strokeSettings) {
    duplicateFillSettingsFrom(fillSettings);
    duplicateStrokeSettingsFrom(strokeSettings);
}

void PathBox::duplicateFillSettingsFrom(PaintSettings *fillSettings) {
    if(fillSettings == NULL) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        fillSettings->makeDuplicate(mFillSettings.data());
    }
}

void PathBox::duplicateStrokeSettingsFrom(StrokeSettings *strokeSettings) {
    if(strokeSettings == NULL) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        strokeSettings->makeDuplicate(mStrokeSettings.data());
    }
}

void PathBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);

    PathBox *pathBoxTarget = ((PathBox*)targetBox);

    int effectsCount = mPathEffectsAnimators->ca_getNumberOfChildren();
    for(int i = 0; i < effectsCount; i++) {
        pathBoxTarget->addPathEffect(
                (PathEffect*)((PathEffect*)mPathEffectsAnimators->
                    ca_getChildAt(i))->makeDuplicate() );
    }

    int fillPathEffectsCount =
            mFillPathEffectsAnimators->ca_getNumberOfChildren();
    for(int i = 0; i < fillPathEffectsCount; i++) {
        pathBoxTarget->addFillPathEffect(
                (PathEffect*)((PathEffect*)mFillPathEffectsAnimators->
                    ca_getChildAt(i))->makeDuplicate() );
    }


    int outlineEffectsCount =
            mOutlinePathEffectsAnimators->ca_getNumberOfChildren();
    for(int i = 0; i < outlineEffectsCount; i++) {
        pathBoxTarget->addOutlinePathEffect(
                (PathEffect*)((PathEffect*)mOutlinePathEffectsAnimators->
                    ca_getChildAt(i))->makeDuplicate() );
    }

    pathBoxTarget->duplicatePaintSettingsFrom(mFillSettings.data(),
                                              mStrokeSettings.data());
    pathBoxTarget->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                               mStrokeGradientPoints.data());
}

void PathBox::drawHoveredSk(SkCanvas *canvas,
                            const SkScalar &invScale) {
    drawHoveredPathSk(canvas, mPathSk, invScale);
}

void PathBox::applyPaintSetting(const PaintSetting &setting) {
    setting.apply(this);
    scheduleUpdate();
}

void PathBox::setFillColorMode(const ColorMode &colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::setStrokeColorMode(const ColorMode &colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}
#include "circle.h"
VectorPath *PathBox::objectToPath() {
    VectorPath *newPath = new VectorPath();
    if(SWT_isCircle()) {
        QPainterPath pathT;
        Circle *circleT = (Circle*)this;
        pathT.addEllipse(QPointF(0., 0.),
                         circleT->getCurrentXRadius(),
                         circleT->getCurrentYRadius());
        newPath->loadPathFromSkPath(QPainterPathToSkPath(pathT));
    } else {
        newPath->loadPathFromSkPath(mPathSk);
    }
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mFillSettings.data(),
                                        mStrokeSettings.data());
    newPath->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                         mStrokeGradientPoints.data());
    mParentGroup->addContainedBox(newPath);
    return newPath;
}

VectorPath *PathBox::strokeToPath() {
    if(mOutlinePathSk.isEmpty()) return NULL;
    VectorPath *newPath = new VectorPath();
    newPath->loadPathFromSkPath(mOutlinePathSk);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mStrokeSettings.data(),
                                        NULL);
    newPath->duplicateGradientPointsFrom(mStrokeGradientPoints.data(),
                                         NULL);
    mParentGroup->addContainedBox(newPath);
    return newPath;
}

const SkPath &PathBox::getRelativePath() const { return mPathSk; }

void PathBox::updateFillDrawGradient() {
    if(mFillSettings->getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mFillSettings->getGradient();

        mFillGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                       gradient->getLastQGradientStopQColor());
        if(!mFillGradientPoints->enabled) {
            mFillGradientPoints->enable();
        }
    } else if(mFillGradientPoints->enabled) {
        mFillGradientPoints->disable();
    }
}

void PathBox::updateStrokeDrawGradient() {
    if(mStrokeSettings->getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings->getGradient();

        mStrokeGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                         gradient->getLastQGradientStopQColor() );

        if(!mStrokeGradientPoints->enabled) {
            mStrokeGradientPoints->enable();
        }
    } else if(mStrokeGradientPoints->enabled) {
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
        BoundingBoxRenderData *renderData) {
    PathBoxRenderData *pathRenderData =
            ((PathBoxRenderData*)renderData);
    mEditPathSk = pathRenderData->editPath;
    mPathSk = pathRenderData->path;
    mOutlinePathSk = pathRenderData->outlinePath;
    BoundingBox::updateCurrentPreviewDataFromRenderData(renderData);
}

bool PathBox::relPointInsidePath(const QPointF &relPos) {
    if(mSkRelBoundingRectPath.contains(relPos.x(), relPos.y()) ) {
        if(mPathSk.contains(relPos.x(), relPos.y())) {
            return true;
        }
        return mOutlinePathSk.contains(relPos.x(), relPos.y());
    } else {
        return false;
    }
}

void PathBox::setOutlineAffectedByScale(const bool &bT) {
    mOutlineAffectedByScale = bT;
    scheduleUpdate();
}

PaintSettings *PathBox::getFillSettings() {
    return mFillSettings.data();
}

StrokeSettings *PathBox::getStrokeSettings() {
    return mStrokeSettings.data();
}
