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

#include "Boxes/pathbox.h"
#include "MovablePoints/gradientpoint.h"
#include "Animators/gradientpoints.h"
#include "skia/skiaincludes.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectcollection.h"
#include "canvas.h"
#include "Animators/transformanimator.h"
#include "paintsettingsapplier.h"
#include "Animators/gradient.h"
#include "RasterEffects/rastereffectcollection.h"
#include "Animators/outlinesettingsanimator.h"
#include "PathEffects/patheffectstask.h"
#include "Private/Tasks/taskscheduler.h"

PathBox::PathBox(const eBoxType type) : BoxWithPathEffects(type) {
    connect(this, &eBoxOrSound::parentChanged, this, [this]() {
        setPathsOutdated(UpdateReason::userChange);
    });

    connect(mPathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
        setPathsOutdated(reason);
    });

    connect(mFillPathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
        setFillPathOutdated(reason);
    });

    connect(mOutlineBasePathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
        setOutlinePathOutdated(reason);
    });

    connect(mOutlinePathEffectsAnimators.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
        setOutlinePathOutdated(reason);
    });

    mStrokeGradientPoints = enve::make_shared<GradientPoints>(this);
    mFillGradientPoints = enve::make_shared<GradientPoints>(this);

    mFillSettings = enve::make_shared<FillSettingsAnimator>(
                mFillGradientPoints.data(), this);
    mStrokeSettings = enve::make_shared<OutlineSettingsAnimator>(
                mStrokeGradientPoints.data(), this);
    mStrokeSettings->setPaintType(PaintType::FLATPAINT);
    mStrokeSettings->setCurrentColor(QColor(0, 0, 0));

    ca_prependChild(mPathEffectsAnimators.get(),
                            mFillSettings);
    ca_prependChild(mPathEffectsAnimators.get(),
                            mStrokeSettings);

    ca_moveChildBelow(mRasterEffectsAnimators.data(),
                      mOutlinePathEffectsAnimators.data());
}

HardwareSupport PathBox::hardwareSupport() const {
    if(!eSettings::sInstance->fPathGpuAcc ||
       mStrokeSettings->getPaintType() == PaintType::BRUSHPAINT) {
        return HardwareSupport::cpuOnly;
    }
    return HardwareSupport::cpuPreffered;
}

void PathBox::setupRenderData(const qreal relFrame,
                              BoxRenderData * const data,
                              Canvas* const scene) {
    if(!getParentScene()) return;
    BoundingBox::setupRenderData(relFrame, data, scene);

    bool currentEditPathCompatible = false;
    bool currentPathCompatible = false;
    bool currentOutlinePathCompatible = false;
    bool currentFillPathCompatible = false;

    if(!mCurrentPathsOutdated) {
        const int prevFrame = qFloor(qMin(data->fRelFrame, mCurrentPathsFrame));
        const int nextFrame = qCeil(qMax(data->fRelFrame, mCurrentPathsFrame));

        const bool sameFrame = prevFrame == nextFrame;

        currentEditPathCompatible = sameFrame || !differenceInEditPathBetweenFrames(prevFrame, nextFrame);
        if(currentEditPathCompatible) {
            currentPathCompatible = sameFrame || !differenceInPathBetweenFrames(prevFrame, nextFrame);
            if(currentPathCompatible && !mCurrentOutlinePathOutdated) {
                currentOutlinePathCompatible = sameFrame || !differenceInOutlinePathBetweenFrames(prevFrame, nextFrame);
            }
            if(currentPathCompatible && !mCurrentFillPathOutdated) {
                currentFillPathCompatible = sameFrame || !differenceInFillPathBetweenFrames(prevFrame, nextFrame);
            }
        }
    }

    const auto pathData = static_cast<PathBoxRenderData*>(data);
    if(currentEditPathCompatible) {
        pathData->fEditPath = mEditPathSk;
    } else {
        pathData->fEditPath = getPathAtRelFrameF(relFrame);
    }

    QList<stdsptr<PathEffectCaller>> pathEffects;
    if(currentPathCompatible) {
        pathData->fPath = mPathSk;
    } else {
        if(scene->getPathEffectsVisible())
            addBasePathEffects(relFrame, pathEffects);
        pathData->fPath = pathData->fEditPath;
    }

    QList<stdsptr<PathEffectCaller>> fillEffects;
    if(currentFillPathCompatible) {
        pathData->fFillPath = mFillPathSk;
    } else {
        if(scene->getPathEffectsVisible()) {
            addFillEffects(relFrame, fillEffects);
        }
        if(pathEffects.isEmpty() && fillEffects.isEmpty()) {
            pathData->fFillPath = pathData->fPath;
        }
    }

    QList<stdsptr<PathEffectCaller>> outlineBaseEffects;
    QList<stdsptr<PathEffectCaller>> outlineEffects;
    if(currentOutlinePathCompatible) {
        pathData->fOutlinePath = mOutlinePathSk;
    } else {
        setupStrokerSettings(pathData, relFrame);

        if(scene->getPathEffectsVisible()) {
            addOutlineBaseEffects(relFrame, outlineBaseEffects);
            addOutlineEffects(relFrame, outlineEffects);
        }

        if(pathEffects.isEmpty() && outlineBaseEffects.isEmpty()) {
            pathData->fOutlineBasePath = pathData->fPath;
            pathData->fStroker.strokePath(pathData->fOutlineBasePath,
                                          &pathData->fOutlinePath);
        }
    }

    if(!pathEffects.isEmpty() || !fillEffects.isEmpty() ||
       !outlineBaseEffects.isEmpty() || !outlineEffects.isEmpty()) {
        const auto pathTask = enve::make_shared<PathEffectsTask>(
                    pathData, std::move(pathEffects), std::move(fillEffects),
                    std::move(outlineBaseEffects), std::move(outlineEffects));
        pathTask->addDependent(pathData);
        pathData->delayDataSet();
        pathTask->queTask();
    }

    if(currentOutlinePathCompatible && currentFillPathCompatible) {
        data->fRelBoundingRectSet = true;
        data->fRelBoundingRect = mRelRect;
    }
    setupPaintSettings(pathData, relFrame);
}

void PathBox::addPathEffects(
        const qreal relFrame, Canvas* const scene,
        PathEffectsCList& pathEffects,
        PathEffectsCList& fillEffects,
        PathEffectsCList& outlineBaseEffects,
        PathEffectsCList& outlineEffects) {
    if(scene->getPathEffectsVisible()) {
        addBasePathEffects(relFrame, pathEffects);
        addFillEffects(relFrame, fillEffects);
        addOutlineBaseEffects(relFrame, outlineBaseEffects);
        addOutlineEffects(relFrame, outlineEffects);
    }
}

void PathBox::setupStrokerSettings(PathBoxRenderData * const pathData,
                                   const qreal relFrame) {
    mStrokeSettings->setStrokerSettingsForRelFrameSk(
                relFrame, &pathData->fStroker);
}

void PathBox::setupPaintSettings(PathBoxRenderData * const pathData,
                                 const qreal relFrame) {

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

    strokeSettings.fPaintColor = mStrokeSettings->getColor(relFrame);
    strokeSettings.fPaintType = mStrokeSettings->getPaintType();
    const auto strokeGrad = mStrokeSettings->getGradient();
    if(strokeSettings.fPaintType == PaintType::GRADIENTPAINT && strokeGrad) {
        strokeSettings.updateGradient(
                    strokeGrad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrameF(relFrame)),
                        mStrokeGradientPoints->getStartPointAtRelFrameF(relFrame),
                        mStrokeGradientPoints->getEndPointAtRelFrameF(relFrame),
                        mStrokeSettings->getGradientType());
    }

    const auto brushSettings = mStrokeSettings->getBrushSettings();
    if(strokeSettings.fPaintType == PaintType::BRUSHPAINT && brushSettings) {
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
            strokeSettings.fSpacingCurve =
                    brushSettings->getSpacingAnimator()->
                        getValueAtRelFrame(relFrame);
        }
    }
}

void PathBox::resetStrokeGradientPointsPos() {
    mStrokeGradientPoints->anim_setRecording(false);
    mStrokeGradientPoints->setPositions(mRelRect.topLeft(), mRelRect.bottomRight());
}

void PathBox::resetFillGradientPointsPos() {
    mFillGradientPoints->anim_setRecording(false);
    mFillGradientPoints->setPositions(mRelRect.topLeft(), mRelRect.bottomRight());
}


void PathBox::setStrokeBrush(SimpleBrushWrapper * const brush) {
    mStrokeSettings->setStrokeBrush(brush);
}

void PathBox::applyStrokeBrushWidthAction(const PathBox::SegAction &action)
{ mStrokeSettings->applyStrokeBrushWidthAction(action); }

void PathBox::applyStrokeBrushPressureAction(const PathBox::SegAction &action)
{ mStrokeSettings->applyStrokeBrushPressureAction(action); }

void PathBox::applyStrokeBrushSpacingAction(const PathBox::SegAction &action)
{ mStrokeSettings->applyStrokeBrushSpacingAction(action); }

void PathBox::applyStrokeBrushTimeAction(const PathBox::SegAction &action)
{ mStrokeSettings->applyStrokeBrushTimeAction(action); }

void PathBox::setStrokeCapStyle(const SkPaint::Cap capStyle) {
    mStrokeSettings->setCapStyle(capStyle);
    prp_afterWholeInfluenceRangeChanged();
    planUpdate(UpdateReason::userChange);
}

void PathBox::setStrokeJoinStyle(const SkPaint::Join joinStyle) {
    mStrokeSettings->setJoinStyle(joinStyle);
    prp_afterWholeInfluenceRangeChanged();
    planUpdate(UpdateReason::userChange);
}

void PathBox::setOutlineCompositionMode(
        const QPainter::CompositionMode &compositionMode) {
    mStrokeSettings->setOutlineCompositionMode(compositionMode);
    prp_afterWholeInfluenceRangeChanged();
    planUpdate(UpdateReason::userChange);
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
        eWriteStream writeStream(&buffer);
        fillSettings->prp_writeProperty(writeStream);
        if(buffer.reset()) {
            eReadStream readStream(&buffer);
            mFillSettings->prp_readProperty(readStream);
        }
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
        eWriteStream writeStream(&buffer);
        strokeSettings->prp_writeProperty(writeStream);
        if(buffer.reset()) {
            eReadStream readStream(&buffer);
            mStrokeSettings->prp_readProperty(readStream);
        }
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
            mFillSettings->setCurrentColor(fillSettings->getColor());
        } else if(paintType == GRADIENTPAINT) {
            mFillSettings->setGradient(fillSettings->getGradient());
            mFillSettings->setGradientType(fillSettings->getGradientType());
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
            mStrokeSettings->getColorAnimator()->setColor(
                        strokeSettings->getColor());
        } else if(paintType == GRADIENTPAINT) {
            mStrokeSettings->setGradient(strokeSettings->getGradient());
            mStrokeSettings->setGradientType(strokeSettings->getGradientType());
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

void PathBox::copyDataToOperationResult(PathBox * const targetBox) const {
    copyRasterEffectsTo(targetBox);
    sWriteReadMember(this, targetBox, &PathBox::mFillSettings);
    sWriteReadMember(this, targetBox, &PathBox::mStrokeSettings);
    sWriteReadMember(this, targetBox, &PathBox::mFillPathEffectsAnimators);
    sWriteReadMember(this, targetBox, &PathBox::mOutlineBasePathEffectsAnimators);
    sWriteReadMember(this, targetBox, &PathBox::mOutlinePathEffectsAnimators);
}

void PathBox::copyPathBoxDataTo(PathBox * const targetBox) const {
    copyBoundingBoxDataTo(targetBox);
    sWriteReadMember(this, targetBox, &PathBox::mFillSettings);
    sWriteReadMember(this, targetBox, &PathBox::mStrokeSettings);
    sWriteReadMember(this, targetBox, &PathBox::mPathEffectsAnimators);
    sWriteReadMember(this, targetBox, &PathBox::mFillPathEffectsAnimators);
    sWriteReadMember(this, targetBox, &PathBox::mOutlineBasePathEffectsAnimators);
    sWriteReadMember(this, targetBox, &PathBox::mOutlinePathEffectsAnimators);
}

bool PathBox::differenceInOutlinePathBetweenFrames(const int frame1, const int frame2) const {
    const auto width = mStrokeSettings->getLineWidthAnimator();
    if(width->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return BoxWithPathEffects::differenceInOutlinePathBetweenFrames(frame1, frame2);
}

#include "circle.h"
#include "Boxes/smartvectorpath.h"

SmartVectorPath *PathBox::objectToVectorPathBox() {
    if(SWT_isSmartVectorPath()) return nullptr;
    const auto newPath = enve::make_shared<SmartVectorPath>();
    newPath->loadSkPath(mEditPathSk);
    copyPathBoxDataTo(newPath.get());
    mParentGroup->addContained(newPath);
    return newPath.get();
}

SmartVectorPath *PathBox::strokeToVectorPathBox() {
    if(mOutlinePathSk.isEmpty()) return nullptr;
    const auto newPath = enve::make_shared<SmartVectorPath>();
    newPath->loadSkPath(mOutlinePathSk);
    copyPathBoxDataTo(newPath.get());
    mParentGroup->addContained(newPath);
    return newPath.get();
}

const SkPath &PathBox::getRelativePath() const { return mPathSk; }

void PathBox::updateFillDrawGradient() {
    const auto gradient = mFillSettings->getGradient();
    if(mFillSettings->getPaintType() == GRADIENTPAINT && gradient) {
        mFillGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                       gradient->getLastQGradientStopQColor());
        mFillGradientPoints->enable();
    } else {
        mFillGradientPoints->disable();
    }
}

void PathBox::updateStrokeDrawGradient() {
    const auto gradient = mStrokeSettings->getGradient();
    if(mStrokeSettings->getPaintType() == GRADIENTPAINT && gradient) {
        mStrokeGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                         gradient->getLastQGradientStopQColor());
        mStrokeGradientPoints->enable();
    } else {
        mStrokeGradientPoints->disable();
    }
}

void PathBox::updateDrawGradients() {
    updateFillDrawGradient();
    updateStrokeDrawGradient();
}

void PathBox::updateCurrentPreviewDataFromRenderData(
        BoxRenderData* renderData) {
    const auto pathRenderData = dynamic_cast<PathBoxRenderData*>(renderData);
    if(pathRenderData) {
        mCurrentPathsFrame = renderData->fRelFrame;
        mEditPathSk = pathRenderData->fEditPath;
        mPathSk = pathRenderData->fPath;
        mOutlinePathSk = pathRenderData->fOutlinePath;
        mFillPathSk = pathRenderData->fFillPath;
        mCurrentPathsOutdated = false;
        mCurrentOutlinePathOutdated = false;
        mCurrentFillPathOutdated = false;
    }

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
    planUpdate(UpdateReason::userChange);
}

FillSettingsAnimator *PathBox::getFillSettings() const {
    return mFillSettings.data();
}

OutlineSettingsAnimator *PathBox::getStrokeSettings() const {
    return mStrokeSettings.data();
}
