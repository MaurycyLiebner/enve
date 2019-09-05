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
#include "canvas.h"
#include "Paint/autotiledsurface.h"
#include <mypaint-brush.h>
class SmartVectorPath;
class GradientPoints;
class SkStroke;
class PathEffectAnimators;
class PathEffect;

struct PathBoxRenderData : public BoxRenderData {
    PathBoxRenderData(BoundingBox * const parentBoxT) :
        BoxRenderData(parentBoxT) {}

    SkPath fEditPath;
    SkPath fPath;
    SkPath fFillPath;
    SkPath fOutlineBasePath;
    SkPath fOutlinePath;
    SkStroke fStroker;
    UpdatePaintSettings fPaintSettings;
    UpdateStrokeSettings fStrokeSettings;

    void updateRelBoundingRect() {
        SkPath totalPath;
        totalPath.addPath(fFillPath);
        totalPath.addPath(fOutlinePath);
        fRelBoundingRect = toQRectF(totalPath.computeTightBounds());
    }
    QPointF getCenterPosition() {
        return toQRectF(fEditPath.getBounds()).center();
    }
protected:
    void drawSk(SkCanvas * const canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        if(!fFillPath.isEmpty()) {
            fPaintSettings.applyPainterSettingsSk(&paint);
            canvas->drawPath(fFillPath, paint);
        }
        if(!fOutlinePath.isEmpty()) {
            paint.setShader(nullptr);
            fStrokeSettings.applyPainterSettingsSk(&paint);
            canvas->drawPath(fOutlinePath, paint);
        }
    }
};

class PathBox : public BoxWithPathEffects {
    e_OBJECT
protected:
    PathBox(const eBoxType type);
public:
    virtual bool differenceInEditPathBetweenFrames(
            const int frame1, const int frame2) const = 0;
    virtual SkPath getPathAtRelFrameF(const qreal relFrame) = 0;

    void setStrokeCapStyle(const SkPaint::Cap capStyle);
    void setStrokeJoinStyle(const SkPaint::Join joinStyle);
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

    void setupCanvasMenu(PropertyMenu * const menu);
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
