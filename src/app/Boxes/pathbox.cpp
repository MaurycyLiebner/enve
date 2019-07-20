#include "Boxes/pathbox.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/gradientpoint.h"
#include "Animators/gradientpoints.h"
#include "skia/skiaincludes.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
#include "canvas.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "PropertyUpdaters/gradientpointsupdater.h"
#include "Animators/effectanimators.h"
#include "Animators/transformanimator.h"
#include "paintsettingsapplier.h"
#include "Animators/gradient.h"
#include "Animators/gpueffectanimators.h"
#include "Animators/outlinesettingsanimator.h"

PathBox::PathBox(const BoundingBoxType &type) :
    BoundingBox(type) {
    mPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mFillPathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mFillPathEffectsAnimators->prp_setName("fill effects");
    mFillPathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mOutlineBasePathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
    mOutlineBasePathEffectsAnimators->prp_setName("outline base effects");
    mOutlineBasePathEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(NodePointUpdater)(this));

    mOutlinePathEffectsAnimators =
            SPtrCreate(PathEffectAnimators)();
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

    mFillSettings = SPtrCreate(FillSettingsAnimator)(
                mFillGradientPoints.data(), this);
    mStrokeSettings = SPtrCreate(OutlineSettingsAnimator)(
                mStrokeGradientPoints.data(), this);
    ca_addChildAnimator(mFillSettings);
    ca_addChildAnimator(mStrokeSettings);

    ca_addChildAnimator(mPathEffectsAnimators);
    ca_addChildAnimator(mFillPathEffectsAnimators);
    ca_addChildAnimator(mOutlineBasePathEffectsAnimators);
    ca_addChildAnimator(mOutlinePathEffectsAnimators);

    ca_moveChildBelow(mGPUEffectsAnimators.data(),
                      mOutlinePathEffectsAnimators.data());
    ca_moveChildBelow(mEffectsAnimators.data(),
                      mGPUEffectsAnimators.data());
}

PathBox::~PathBox() {
    if(mFillSettings->getGradient())
        mFillSettings->getGradient()->removePath(this);
    if(mStrokeSettings->getGradient())
        mStrokeSettings->getGradient()->removePath(this);
}

void PathBox::setPathEffectsEnabled(const bool enable) {
    mPathEffectsAnimators->SWT_setEnabled(enable);
    mPathEffectsAnimators->SWT_setVisible(
                mPathEffectsAnimators->hasChildAnimators() || enable);
}

bool PathBox::getPathEffectsEnabled() const {
    return mPathEffectsAnimators->SWT_isEnabled();
}

void PathBox::setFillEffectsEnabled(const bool enable) {
    mFillPathEffectsAnimators->SWT_setEnabled(enable);
    mFillPathEffectsAnimators->SWT_setVisible(
                mFillPathEffectsAnimators->hasChildAnimators() || enable);
}

bool PathBox::getFillEffectsEnabled() const {
    return mFillPathEffectsAnimators->SWT_isEnabled();
}

void PathBox::setOutlineBaseEffectsEnabled(const bool enable) {
    mOutlinePathEffectsAnimators->SWT_setEnabled(enable);
    mOutlinePathEffectsAnimators->SWT_setVisible(
                mOutlinePathEffectsAnimators->hasChildAnimators() || enable);
}

bool PathBox::getOutlineBaseEffectsEnabled() const {
    return mOutlinePathEffectsAnimators->SWT_isEnabled();
}

void PathBox::setOutlineEffectsEnabled(const bool enable) {
    mOutlinePathEffectsAnimators->SWT_setEnabled(enable);
    mOutlinePathEffectsAnimators->SWT_setVisible(
                mOutlinePathEffectsAnimators->hasChildAnimators() || enable);
}

bool PathBox::getOutlineEffectsEnabled() const {
    return mOutlinePathEffectsAnimators->SWT_isEnabled();
}

void PathBox::setParentGroup(ContainerBox * const parent) {
    setPathsOutdated();
    BoundingBox::setParentGroup(parent);
}

void PathBox::setupRenderData(const qreal relFrame,
                              BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);

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
            mPathEffectsAnimators->apply(relFrame, &pathData->fPath);
            const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
            const qreal parentRelFrame =
                    mParentGroup->prp_absFrameToRelFrameF(absFrame);
            mParentGroup->applyPathEffects(parentRelFrame, &pathData->fPath,
                                                data->fParentBox);
        }
    }

    if(currentOutlinePathCompatible) {
        pathData->fOutlinePath = mOutlinePathSk;
    } else {
        SkPath outline;
        if(mStrokeSettings->nonZeroLineWidth()) {
            SkPath outlineBase = pathData->fPath;
            mOutlineBasePathEffectsAnimators->apply(
                        relFrame, &outlineBase);
            mParentGroup->filterOutlineBasePath(
                        relFrame, &outlineBase);
            SkStroke strokerSk;
            mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
            strokerSk.strokePath(outlineBase, &outline);
        }
        if(getParentCanvas()->getPathEffectsVisible()) {
            mOutlinePathEffectsAnimators->apply(relFrame, &outline);
            mParentGroup->filterOutlinePath(relFrame, &outline);
        }
        pathData->fOutlinePath = outline;
        //outline.addPath(pathData->fPath);
    }

    if(currentFillPathCompatible) {
        pathData->fFillPath = mFillPathSk;
    } else {
        pathData->fFillPath = pathData->fPath;
        mFillPathEffectsAnimators->apply(relFrame, &pathData->fPath);
        mParentGroup->filterFillPath(relFrame, &pathData->fPath);
    }

    if(currentOutlinePathCompatible && currentFillPathCompatible) {
        data->fRelBoundingRectSet = true;
        data->fRelBoundingRect = mRelBoundingRect;
    }

    UpdatePaintSettings &fillSettings = pathData->fPaintSettings;

    fillSettings.fPaintColor = mFillSettings->getColor(relFrame);
    fillSettings.fPaintType = mFillSettings->getPaintType();
    const auto fillGrad = mFillSettings->getGradient();
    if(fillGrad) {
        fillSettings.updateGradient(
                    fillGrad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrameF(relFrame)),
                    mFillGradientPoints->getStartPointAtRelFrameF(relFrame),
                    mFillGradientPoints->getEndPointAtRelFrameF(relFrame),
                    mFillSettings->getGradientType());
    }

    UpdateStrokeSettings &strokeSettings = pathData->fStrokeSettings;
    const auto widthAnimator = mStrokeSettings->getStrokeWidthAnimator();
    strokeSettings.fOutlineWidth = widthAnimator->getEffectiveValue(relFrame);

    strokeSettings.fPaintColor = mStrokeSettings->
            getColor(relFrame);
    strokeSettings.fPaintType = mStrokeSettings->getPaintType();
    const auto strokeGrad = mStrokeSettings->getGradient();
    if(strokeGrad) {
        strokeSettings.updateGradient(
                    strokeGrad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrameF(relFrame)),
                    mStrokeGradientPoints->getStartPointAtRelFrameF(relFrame),
                    mStrokeGradientPoints->getEndPointAtRelFrameF(relFrame),
                    mStrokeSettings->getGradientType());
    }
}

void PathBox::addPathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->addChild(effect);
}

void PathBox::addFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->addChild(effect);
}

void PathBox::addOutlineBasePathEffect(const qsptr<PathEffect>& effect) {
    mOutlineBasePathEffectsAnimators->addChild(effect);
}

void PathBox::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->addChild(effect);
}

void PathBox::removePathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->removeChild(effect);
}

void PathBox::removeFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->removeChild(effect);
}

void PathBox::removeOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->removeChild(effect);
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

void PathBox::setStrokeCapStyle(const SkPaint::Cap capStyle) {
    mStrokeSettings->setCapStyle(capStyle);
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::setStrokeJoinStyle(const SkPaint::Join joinStyle) {
    mStrokeSettings->setJoinStyle(joinStyle);
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::setOutlineCompositionMode(
        const QPainter::CompositionMode &compositionMode) {
    mStrokeSettings->setOutlineCompositionMode(compositionMode);
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(Animator::USER_CHANGE);
}

void PathBox::strokeWidthAction(const QrealAction& action) {
    mStrokeSettings->strokeWidthAction(action);
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

SkPath PathBox::getPathWithThisOnlyEffectsAtRelFrameF(const qreal relFrame) {
    SkPath path = getPathAtRelFrameF(relFrame);
    mPathEffectsAnimators->apply(relFrame, &path);
    return path;
}

void PathBox::getMotionBlurProperties(QList<Property*> &list) const {
    BoundingBox::getMotionBlurProperties(list);
    list.append(mPathEffectsAnimators.get());
    list.append(mFillPathEffectsAnimators.get());
    list.append(mOutlinePathEffectsAnimators.get());
}

GradientPoints *PathBox::getFillGradientPoints() {
    return mFillGradientPoints.data();
}

void PathBox::duplicatePaintSettingsFrom(
        FillSettingsAnimator * const fillSettings,
        OutlineSettingsAnimator * const strokeSettings) {
    duplicateFillSettingsFrom(fillSettings);
    duplicateStrokeSettingsFrom(strokeSettings);
}

void PathBox::duplicateFillSettingsFrom(
        FillSettingsAnimator * const fillSettings) {
    if(!fillSettings) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        fillSettings->writeProperty(&buffer);
        if(buffer.reset()) mFillSettings->readProperty(&buffer);
        buffer.close();
    }
}

void PathBox::duplicateStrokeSettingsFrom(
        OutlineSettingsAnimator * const strokeSettings) {
    if(!strokeSettings) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        strokeSettings->writeProperty(&buffer);
        if(buffer.reset()) mStrokeSettings->readProperty(&buffer);
        buffer.close();
    }
}

void PathBox::duplicateFillSettingsNotAnimatedFrom(
        FillSettingsAnimator * const fillSettings) {
    if(!fillSettings) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        const PaintType paintType = fillSettings->getPaintType();
        mFillSettings->setPaintType(paintType);
        if(paintType == FLATPAINT) {
            mFillSettings->setCurrentColor(
                        fillSettings->getColor());
        } else if(paintType == GRADIENTPAINT) {
            mFillSettings->setGradient(
                        fillSettings->getGradient());
            mFillSettings->setGradientType(
                        fillSettings->getGradientType());
        }
    }
}

void PathBox::duplicateStrokeSettingsNotAnimatedFrom(
        OutlineSettingsAnimator * const strokeSettings) {
    if(!strokeSettings) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        const PaintType paintType = strokeSettings->getPaintType();
        mStrokeSettings->setPaintType(paintType);
        if(paintType == FLATPAINT) {
            mStrokeSettings->getColorAnimator()->qra_setCurrentValue(
                        strokeSettings->getColor());
        } else if(paintType == GRADIENTPAINT) {
            mStrokeSettings->setGradient(
                        strokeSettings->getGradient());
            mStrokeSettings->setGradientType(
                        strokeSettings->getGradientType());
        }
        mStrokeSettings->getStrokeWidthAnimator()->setCurrentBaseValue(
                    strokeSettings->getCurrentStrokeWidth());
    }
}

void PathBox::drawHoveredSk(SkCanvas *canvas, const float invScale) {
    drawHoveredPathSk(canvas, mPathSk, invScale);
}

void PathBox::applyPaintSetting(const PaintSettingsApplier &setting) {
    setting.apply(this);
}

void PathBox::setFillColorMode(const ColorMode colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::setStrokeColorMode(const ColorMode colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::copyPathBoxDataTo(PathBox * const targetBox) {
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    PathBox::writeBoundingBox(&buffer);
    BoundingBox::sClearWriteBoxes();

    targetBox->PathBox::readBoundingBox(&buffer);
    buffer.close();

    BoundingBox::sClearReadBoxes();
}

bool PathBox::differenceInPathBetweenFrames(const int frame1, const int frame2) const {
    if(mPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(!mParentGroup) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);
    return mParentGroup->differenceInPathEffectsBetweenFrames(pFrame1, pFrame2);
}

bool PathBox::differenceInOutlinePathBetweenFrames(const int frame1, const int frame2) const {
    if(mStrokeSettings->getLineWidthAnimator()->
       prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    if(mOutlineBasePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(!mParentGroup) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);
    return mParentGroup->differenceInOutlinePathEffectsBetweenFrames(pFrame1, pFrame2);
}

bool PathBox::differenceInFillPathBetweenFrames(const int frame1, const int frame2) const {
    if(mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(!mParentGroup) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);
    return mParentGroup->differenceInFillPathEffectsBetweenFrames(pFrame1, pFrame2);
}

#include "circle.h"
#include "Boxes/smartvectorpath.h"

SmartVectorPath *PathBox::objectToVectorPathBox() {
    if(SWT_isSmartVectorPath()) return nullptr;
    auto newPath = SPtrCreate(SmartVectorPath)();
    if(SWT_isCircle()) {
        QPainterPath pathT;
        const auto circleT = GetAsPtr(this, Circle);
        pathT.addEllipse(QPointF(0, 0),
                         circleT->getCurrentXRadius(),
                         circleT->getCurrentYRadius());
        newPath->loadSkPath(toSkPath(pathT));
    } else {
        newPath->loadSkPath(mEditPathSk);
    }
    copyPathBoxDataTo(newPath.get());
    mParentGroup->addContainedBox(newPath);
    return newPath.get();
}

SmartVectorPath *PathBox::strokeToVectorPathBox() {
    if(mOutlinePathSk.isEmpty()) return nullptr;
    const auto newPath = SPtrCreate(SmartVectorPath)();
    newPath->loadSkPath(mOutlinePathSk);
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

QRectF PathBox::getRelBoundingRect(const qreal relFrame) {
    const SkPath path = getPathAtRelFrameF(relFrame);
    SkPath outline;
    if(mStrokeSettings->nonZeroLineWidth()) {
        SkStroke strokerSk;
        mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
        strokerSk.strokePath(path, &outline);
    }
    mOutlinePathEffectsAnimators->apply(relFrame, &outline);
    outline.addPath(path);
    return toQRectF(outline.computeTightBounds());
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
}

bool PathBox::relPointInsidePath(const QPointF &relPos) const {
    const SkPoint relPosSk = toSkPoint(relPos);
    if(mSkRelBoundingRectPath.contains(relPosSk.x(), relPosSk.y()) ) {
        if(mFillPathSk.contains(relPosSk.x(), relPosSk.y())) {
            return true;
        }
        return mOutlinePathSk.contains(relPosSk.x(), relPosSk.y());
    } else {
        return false;
    }
}

void PathBox::setOutlineAffectedByScale(const bool bT) {
    mOutlineAffectedByScale = bT;
    planScheduleUpdate(Animator::USER_CHANGE);
}

FillSettingsAnimator *PathBox::getFillSettings() const {
    return mFillSettings.data();
}

OutlineSettingsAnimator *PathBox::getStrokeSettings() const {
    return mStrokeSettings.data();
}
