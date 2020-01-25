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

#ifndef PAINTSETTINGSANIMATOR_H
#define PAINTSETTINGSANIMATOR_H

#include "Animators/qrealanimator.h"
#include "coloranimator.h"
#include "colorhelpers.h"
#include "skia/skiaincludes.h"
#include "smartPointers/ememory.h"
#include "Animators/sceneboundgradient.h"
#include "Animators/staticcomplexanimator.h"
#include "paintsettings.h"
#include "Paint/simplebrushwrapper.h"
#include "conncontextptr.h"
#include "transformvalues.h"

class PathBox;
class SkStroke;

class GradientPoints;
class AdvancedTransformAnimator;

class PaintSettingsAnimator : public ComplexAnimator {
protected:
    PaintSettingsAnimator(const QString &name,
                          const qsptr<GradientPoints> &grdPts,
                          PathBox * const parent);

    virtual void showHideChildrenBeforeChaningPaintType(
            const PaintType newPaintType);
public:
    void prp_writeProperty(eWriteStream& dst) const;
    void prp_readProperty(eReadStream& src);

    QColor getColor() const;
    PaintType getPaintType() const;
    SceneBoundGradient *getGradient() const;
    void setGradient(SceneBoundGradient *gradient);
    void setCurrentColor(const QColor &color);
    void setPaintType(const PaintType paintType);
    ColorAnimator *getColorAnimator();
    void setGradientPointsPos(const QPointF& pt1, const QPointF& pt2);

    void duplicateColorAnimatorFrom(ColorAnimator *source);
    void setGradientVar(SceneBoundGradient * const grad);
    QColor getColor(const qreal relFrame) const;
    GradientType getGradientType() { return mGradientType; }
    void setGradientType(const GradientType type);

    QMatrix getGradientTransform(const qreal relFrame) const;
    void setGradientTransform(const TransformValues& transform);
private:
    GradientType mGradientType = GradientType::LINEAR;
    PaintType mPaintType = NOPAINT;

    PathBox * const mTarget_k;
    const qsptr<GradientPoints> mGradientPoints;
    qsptr<ColorAnimator> mColor = enve::make_shared<ColorAnimator>();
    ConnContextQPtr<SceneBoundGradient> mGradient;
    qsptr<AdvancedTransformAnimator> mGradientTransform;
};

class FillSettingsAnimator : public PaintSettingsAnimator {
    e_OBJECT
protected:
    FillSettingsAnimator(const qsptr<GradientPoints> &grdPts,
                         PathBox * const parent) :
        PaintSettingsAnimator("fill", grdPts, parent) {}
public:
    bool SWT_isFillSettingsAnimator() const { return true; }
};

struct UpdatePaintSettings {
    UpdatePaintSettings(const QColor &paintColorT,
                        const PaintType paintTypeT);
    UpdatePaintSettings();

    virtual ~UpdatePaintSettings();

    virtual void applyPainterSettingsSk(SkPaint *paint);

    void updateGradient(const QGradientStops &stops,
                        const QPointF &start,
                        const QPointF &finalStop,
                        const GradientType gradientType,
                        const QMatrix &transform);
    PaintType fPaintType;
    QColor fPaintColor;
    sk_sp<SkShader> fGradient;
};

struct UpdateStrokeSettings : UpdatePaintSettings {
    UpdateStrokeSettings(
            const qreal width,
            const QColor &paintColorT,
            const PaintType paintTypeT,
            const QPainter::CompositionMode &outlineCompositionModeT);
    UpdateStrokeSettings();

    void applyPainterSettingsSk(SkPaint *paint);

    QPainter::CompositionMode fOutlineCompositionMode =
            QPainter::CompositionMode_Source;

    qreal fOutlineWidth;
    stdsptr<SimpleBrushWrapper> fStrokeBrush;
    qCubicSegment1D fTimeCurve;
    qCubicSegment1D fPressureCurve;
    qCubicSegment1D fWidthCurve;
    qCubicSegment1D fSpacingCurve;
};

#endif // PAINTSETTINGSANIMATOR_H
