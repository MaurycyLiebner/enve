// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#ifndef PATHBOX_H
#define PATHBOX_H
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include "Boxes/boxwithpatheffects.h"
#include "Animators/paintsettingsanimator.h"
#include "Paint/autotiledsurface.h"
#include "pathboxrenderdata.h"
#include <mypaint-brush.h>
class SmartVectorPath;
class GradientPoints;
class SkStroke;
class PathEffectAnimators;
class PathEffect;

class PathBox : public BoxWithPathEffects {
    e_OBJECT
protected:
    PathBox(const eBoxType type);
public:
    virtual bool differenceInEditPathBetweenFrames(
            const int frame1, const int frame2) const = 0;
    virtual SkPath getPathAtRelFrameF(const qreal relFrame) = 0;

    HardwareSupport hardwareSupport() const;

    void setStrokeCapStyle(const SkPaint::Cap capStyle);
    void setStrokeJoinStyle(const SkPaint::Join joinStyle);
    void setStrokeBrush(SimpleBrushWrapper * const brush);
    void setStrokeBrushWidthCurve(const qCubicSegment1D& curve);
    void setStrokeBrushTimeCurve(const qCubicSegment1D& curve);
    void setStrokeBrushPressureCurve(const qCubicSegment1D& curve);
    void setStrokeBrushSpacingCurve(const qCubicSegment1D& curve);
    void strokeWidthAction(const QrealAction &action);

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    OutlineSettingsAnimator *getStrokeSettings() const;
    FillSettingsAnimator *getFillSettings() const;

    SmartVectorPath *objectToVectorPathBox();
    SmartVectorPath *strokeToVectorPathBox();

    bool relPointInsidePath(const QPointF &relPos) const;

    void drawHoveredSk(SkCanvas *canvas, const float invScale);

    void applyPaintSetting(const PaintSettingsApplier &setting);

    bool SWT_isPathBox() const { return true; }

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);
    stdsptr<BoxRenderData> createRenderData() {
        return enve::make_shared<PathBoxRenderData>(this);
    }
    void updateCurrentPreviewDataFromRenderData(
            BoxRenderData *renderData);

    void setupPathEffects(PathBoxRenderData * const pathData,
                          const qreal relFrame,
                          Canvas* const scene);
    void setupStrokerSettings(PathBoxRenderData * const pathData,
                              const qreal relFrame);
    void setupPaintSettings(PathBoxRenderData * const pathData,
                            const qreal relFrame);

    void duplicateStrokeSettingsFrom(
            OutlineSettingsAnimator * const strokeSettings);
    void duplicateFillSettingsFrom(
            FillSettingsAnimator * const fillSettings);
    void duplicateStrokeSettingsNotAnimatedFrom(
            OutlineSettingsAnimator * const strokeSettings);
    void duplicateFillSettingsNotAnimatedFrom(
            FillSettingsAnimator * const fillSettings);
    void duplicatePaintSettingsFrom(FillSettingsAnimator * const fillSettings,
                                    OutlineSettingsAnimator * const strokeSettings);

    void updateStrokeDrawGradient();
    void updateFillDrawGradient();
    const SkPath &getRelativePath() const;
    void updateDrawGradients();
    void setOutlineAffectedByScale(const bool bT);

    GradientPoints *getFillGradientPoints();
    GradientPoints *getStrokeGradientPoints();

    void copyPathBoxDataTo(PathBox * const targetBox);

    bool differenceInOutlinePathBetweenFrames(
            const int frame1, const int frame2) const;

    void setPathsOutdated(const UpdateReason reason) {
        mCurrentPathsOutdated = true;
        planUpdate(reason);
    }

    void setOutlinePathOutdated(const UpdateReason reason) {
        mCurrentOutlinePathOutdated = true;
        planUpdate(reason);
    }

    void setFillPathOutdated(const UpdateReason reason) {
        mCurrentFillPathOutdated = true;
        planUpdate(reason);
    }

    void resetStrokeGradientPointsPos();
    void resetFillGradientPointsPos();
protected:
    bool mOutlineAffectedByScale = true;
    bool mCurrentPathsOutdated = true;
    bool mCurrentOutlinePathOutdated = true;
    bool mCurrentFillPathOutdated = true;

    qreal mCurrentPathsFrame = 0;

    SkPath mEditPathSk;
    SkPath mPathSk;
    SkPath mFillPathSk;
    SkPath mOutlinePathSk;

    qsptr<GradientPoints> mFillGradientPoints;
    qsptr<GradientPoints> mStrokeGradientPoints;

    qsptr<FillSettingsAnimator> mFillSettings;
    qsptr<OutlineSettingsAnimator> mStrokeSettings;
};

#endif // PATHBOX_H
