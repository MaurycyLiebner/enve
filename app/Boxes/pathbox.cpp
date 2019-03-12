#include "Boxes/pathbox.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/gradientpoint.h"
#include "Animators/gradientpoints.h"
#include "skia/skiaincludes.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
#include "canvas.h"
#include "vectorpath.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "PropertyUpdaters/gradientpointsupdater.h"
#include "Animators/effectanimators.h"
#include "Animators/transformanimator.h"
#include "paintsettingsapplier.h"
#include "Animators/gradient.h"

PathBox::PathBox(const BoundingBoxType &type) :
    BoundingBox(type) {
    mPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)(false, false, this);
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mFillPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)(false, true, this);
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mOutlinePathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)(true, false, this);
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(NodePointUpdater)(this));

//    mPathEffectsAnimators->prp_setName("path effects");
//    mPathEffectsAnimators->prp_setOwnUpdater(
//                SPtrCreate(PixmapEffectUpdater)(this));

//    mOutlinePathEffectsAnimators->prp_setName("outline path effects");
//    mOutlinePathEffectsAnimators->prp_setOwnUpdater(
//                SPtrCreate(PixmapEffectUpdater)(this));

    mStrokeGradientPoints = SPtrCreate(GradientPoints)(this);
    mStrokeGradientPoints->prp_setOwnUpdater(
                SPtrCreate(GradientPointsUpdater)(false, this));

    mFillGradientPoints = SPtrCreate(GradientPoints)(this);
    mFillGradientPoints->prp_setOwnUpdater(
                SPtrCreate(GradientPointsUpdater)(true, this));

    mFillSettings = SPtrCreate(PaintSettings)(
                mFillGradientPoints.data(), this);
    mStrokeSettings = SPtrCreate(StrokeSettings)(
                mStrokeGradientPoints.data(), this);
    ca_addChildAnimator(mFillSettings);
    ca_addChildAnimator(mStrokeSettings);
    ca_moveChildAbove(mEffectsAnimators.data(),
                      mStrokeSettings.data());
    ca_addChildAnimator(mPathEffectsAnimators);
    ca_addChildAnimator(mFillPathEffectsAnimators);
    ca_addChildAnimator(mOutlinePathEffectsAnimators);
}

PathBox::~PathBox() {
    if(mFillSettings->getGradient())
        mFillSettings->getGradient()->removePath(this);
    if(mStrokeSettings->getGradient())
        mStrokeSettings->getGradient()->removePath(this);
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

    if(!mCurrentPathsOutdated) {
        currentEditPathCompatible =
                !differenceInEditPathBetweenFrames(
                    data->fRelFrame, mCurrentPathsFrame);
        if(currentEditPathCompatible) {
            currentPathCompatible =
                    !differenceInPathBetweenFrames(
                        data->fRelFrame, mCurrentPathsFrame);
            if(currentPathCompatible && !mCurrentOutlinePathOutdated) {
                currentOutlinePathCompatible =
                        !differenceInOutlinePathBetweenFrames(
                            data->fRelFrame, mCurrentPathsFrame);
                currentFillPathCompatible =
                        !differenceInFillPathBetweenFrames(
                            data->fRelFrame, mCurrentPathsFrame);
            }
        }
    }

    const auto pathData = GetAsPtr(data, PathBoxRenderData);
    if(currentEditPathCompatible) {
        pathData->fEditPath = mEditPathSk;
    } else {
        pathData->fEditPath = getPathAtRelFrameF(relFrame);
    }
    if(currentPathCompatible) {
        pathData->fPath = mPathSk;
    } else {
        pathData->fPath = pathData->fEditPath;
        if(getParentCanvas()->getPathEffectsVisible()) {
            // !!! reversed
            //mPathEffectsAnimators->filterPathForRelFrame(relFrame, &pathData->path);
            const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
            const qreal parentRelFrame =
                    mParentGroup->prp_absFrameToRelFrameF(absFrame);
            mParentGroup->filterPathForRelFrame(parentRelFrame, &pathData->fPath,
                                                data->fParentBox.data());
            // !!! reversed
            mPathEffectsAnimators->filterPathForRelFrame(relFrame, &pathData->fPath);
        }
    }

    if(currentOutlinePathCompatible) {
        pathData->fOutlinePath = mOutlinePathSk;
    } else {
        SkPath outline;
        if(mStrokeSettings->nonZeroLineWidth()) {
            SkPath outlineBase = pathData->fPath;
            mOutlinePathEffectsAnimators->filterPathForRelFrameBeforeThicknessF(
                        relFrame, &outlineBase);
            mParentGroup->filterOutlinePathBeforeThicknessForRelFrame(
                        relFrame, &outlineBase);
            SkStroke strokerSk;
            mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
            outline = SkPath();
            strokerSk.strokePath(outlineBase, &outline);
        } else {
            outline = SkPath();
        }
        if(getParentCanvas()->getPathEffectsVisible()) {
            mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, &outline);
            mParentGroup->filterOutlinePathForRelFrame(relFrame, &outline);
        }
        pathData->fOutlinePath = outline;
        //outline.addPath(pathData->fPath);
    }

    if(currentFillPathCompatible) {
        pathData->fFillPath = mFillPathSk;
    } else {
        pathData->fFillPath = pathData->fPath;
        mFillPathEffectsAnimators->filterPathForRelFrame(relFrame, &pathData->fPath);
        mParentGroup->filterFillPathForRelFrame(relFrame, &pathData->fPath);
    }

    if(currentOutlinePathCompatible && currentFillPathCompatible) {
        data->fRelBoundingRectSet = true;
        data->fRelBoundingRect = mRelBoundingRect;
    }

    UpdatePaintSettings &fillSettings = pathData->fPaintSettings;

    fillSettings.fPaintColor = mFillSettings->
            getColorAtRelFrame(relFrame);
    fillSettings.fPaintType = mFillSettings->getPaintType();
    const auto fillGrad = mFillSettings->getGradient();
    if(fillGrad) {
        fillSettings.updateGradient(
                    fillGrad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrameF(relFrame)),
                    mFillGradientPoints->getStartPointAtRelFrameF(relFrame),
                    mFillGradientPoints->getEndPointAtRelFrameF(relFrame),
                    mFillSettings->getGradientLinear());
    }

    UpdateStrokeSettings &strokeSettings = pathData->fStrokeSettings;
    const auto widthAnimator = mStrokeSettings->getStrokeWidthAnimator();
    strokeSettings.fOutlineWidth = widthAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
    const auto brushSettings = mStrokeSettings->getBrushSettings();
    if(brushSettings) {
        auto brush = brushSettings->getBrush();
        if(brush) {
            strokeSettings.fStrokeBrush = brush->createDuplicate();
            strokeSettings.fTimeCurve =
                    brushSettings->getTimeAnimator()->
                        getValueAtRelFrame(relFrame);

            strokeSettings.fWidthCurve =
                    brushSettings->getWidthAnimator()->
                        getValueAtRelFrame(relFrame)*strokeSettings.fOutlineWidth;
            strokeSettings.fPressureCurve =
                    brushSettings->getPressureAnimator()->
                        getValueAtRelFrame(relFrame);
        }
    }
    strokeSettings.fPaintColor = mStrokeSettings->
            getColorAtRelFrame(relFrame);
    strokeSettings.fPaintType = mStrokeSettings->getPaintType();
    const auto strokeGrad = mStrokeSettings->getGradient();
    if(strokeGrad) {
        strokeSettings.updateGradient(
                    strokeGrad->getQGradientStopsAtAbsFrame(
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
        if(!pointToReturn) {
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

void PathBox::addPathEffect(const qsptr<PathEffect>& effect) {
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

void PathBox::addFillPathEffect(const qsptr<PathEffect>& effect) {
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

void PathBox::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
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

void PathBox::removePathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mPathEffectsAnimators->hasChildAnimators()) {
        mPathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
}

void PathBox::removeFillPathEffect(const qsptr<PathEffect>& effect) {
    if(effect->hasReasonsNotToApplyUglyTransform()) {
        decReasonsNotToApplyUglyTransform();
    }
    mFillPathEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mFillPathEffectsAnimators->hasChildAnimators()) {
        mFillPathEffectsAnimators->SWT_hide();
    }

    clearAllCache();
}

void PathBox::removeOutlinePathEffect(const qsptr<PathEffect>& effect) {
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
    mStrokeGradientPoints->anim_setRecording(false);
    mStrokeGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                        mRelBoundingRect.bottomRight());
}

void PathBox::resetFillGradientPointsPos() {
    mFillGradientPoints->anim_setRecording(false);
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

void PathBox::setStrokeWidth(const qreal &strokeWidth) {
    mStrokeSettings->setCurrentStrokeWidth(strokeWidth);
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

SkPath PathBox::getPathWithThisOnlyEffectsAtRelFrameF(const qreal &relFrame) {
    SkPath path = getPathAtRelFrameF(relFrame);
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, &path);
    return path;
}


void PathBox::getMotionBlurProperties(QList<Property*> &list) const {
    BoundingBox::getMotionBlurProperties(list);
    list.append(mPathEffectsAnimators.get());
    list.append(mFillPathEffectsAnimators.get());
    list.append(mOutlinePathEffectsAnimators.get());
}

SkPath PathBox::getPathWithEffectsUntilGroupSumAtRelFrameF(const qreal &relFrame) {
    SkPath path = getPathAtRelFrameF(relFrame);
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, &path);
    if(!mParentGroup) return path;
    const qreal parentRelFrame = mParentGroup->prp_absFrameToRelFrameF(
                prp_relFrameToAbsFrameF(relFrame));
    mParentGroup->filterPathForRelFrameUntilGroupSum(parentRelFrame, &path);
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
    if(!fillSettings) {
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
    if(!strokeSettings) {
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
    if(!fillSettings) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        const PaintType paintType = fillSettings->getPaintType();
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

void PathBox::duplicateStrokeSettingsNotAnimatedFrom(
        StrokeSettings *strokeSettings) {
    if(!strokeSettings) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        const PaintType paintType = strokeSettings->getPaintType();
        mStrokeSettings->setPaintType(paintType);
        if(paintType == FLATPAINT || paintType == BRUSHPAINT) {
            mStrokeSettings->getColorAnimator()->qra_setCurrentValue(
                        strokeSettings->getCurrentColor());
        } else if(paintType == GRADIENTPAINT) {
            mStrokeSettings->setGradient(
                        strokeSettings->getGradient());
            mStrokeSettings->setGradientLinear(
                        strokeSettings->getGradientLinear());
        }
        mStrokeSettings->getStrokeWidthAnimator()->qra_setCurrentValue(
                    strokeSettings->getCurrentStrokeWidth());
    }
}

void PathBox::drawHoveredSk(SkCanvas *canvas,
                            const SkScalar &invScale) {
    drawHoveredPathSk(canvas, mPathSk, invScale);
}

void PathBox::applyPaintSetting(const PaintSettingsApplier &setting) {
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
    auto newPath = SPtrCreate(VectorPath)();
    if(SWT_isCircle()) {
        QPainterPath pathT;
        const auto circleT = GetAsPtr(this, Circle);
        pathT.addEllipse(QPointF(0, 0),
                         circleT->getCurrentXRadius(),
                         circleT->getCurrentYRadius());
        newPath->loadPathFromSkPath(QPainterPathToSkPath(pathT));
    } else {
        //newPath->loadPathFromSkPath(mEditPathSk);
        newPath->loadPathFromSkPath(mPathSk);
    }
    copyPathBoxDataTo(newPath.get());
    mParentGroup->addContainedBox(newPath);
    return newPath.get();
}

VectorPath *PathBox::strokeToVectorPathBox() {
    if(mOutlinePathSk.isEmpty()) return nullptr;
    auto newPath = SPtrCreate(VectorPath)();
    copyPathBoxDataTo(newPath.get());
    mParentGroup->addContainedBox(newPath);
    return newPath.get();
}

const SkPath &PathBox::getRelativePath() const { return mPathSk; }

void PathBox::updateFillDrawGradient() {
    const auto gradient = mFillSettings->getGradient();
    if(mFillSettings->getPaintType() == GRADIENTPAINT && gradient) {
        mFillGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                       gradient->getLastQGradientStopQColor());
        if(!mFillGradientPoints->enabled()) mFillGradientPoints->enable();
    } else if(mFillGradientPoints->enabled()) {
        mFillGradientPoints->disable();
    }
}

void PathBox::updateStrokeDrawGradient() {
    const auto gradient = mStrokeSettings->getGradient();
    if(mStrokeSettings->getPaintType() == GRADIENTPAINT && gradient) {
        mStrokeGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                         gradient->getLastQGradientStopQColor());

        if(!mStrokeGradientPoints->enabled()) mStrokeGradientPoints->enable();
    } else if(mStrokeGradientPoints->enabled()) {
        mStrokeGradientPoints->disable();
    }
}

void PathBox::updateDrawGradients() {
    updateFillDrawGradient();
    updateStrokeDrawGradient();
}

QRectF PathBox::getRelBoundingRectAtRelFrame(const qreal &relFrame) {
    const SkPath path = getPathAtRelFrameF(relFrame);
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
        BoundingBoxRenderData* renderData) {
    auto pathRenderData = GetAsPtr(renderData, PathBoxRenderData);
    mCurrentPathsFrame = renderData->fRelFrame;
    mEditPathSk = pathRenderData->fEditPath;
    mPathSk = pathRenderData->fPath;
    mOutlinePathSk = pathRenderData->fOutlinePath;
    mFillPathSk = pathRenderData->fFillPath;
    mCurrentPathsOutdated = false;
    mCurrentOutlinePathOutdated = false;
    BoundingBox::updateCurrentPreviewDataFromRenderData(renderData);
//    updateDialog_TEST();
}

bool PathBox::relPointInsidePath(const QPointF &relPos) const {
    const SkPoint relPosSk = qPointToSk(relPos);
    if(mSkRelBoundingRectPath.contains(relPosSk.x(), relPosSk.y()) ) {
        if(mFillPathSk.contains(relPosSk.x(), relPosSk.y())) {
            return true;
        }
        return mOutlinePathSk.contains(relPosSk.x(), relPosSk.y());
    } else {
        return false;
    }
}

void PathBox::setOutlineAffectedByScale(const bool &bT) {
    mOutlineAffectedByScale = bT;
    scheduleUpdate(Animator::USER_CHANGE);
}

PaintSettings *PathBox::getFillSettings() const {
    return mFillSettings.data();
}

StrokeSettings *PathBox::getStrokeSettings() const {
    return mStrokeSettings.data();
}
