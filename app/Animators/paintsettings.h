#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

#include "Animators/qrealanimator.h"
#include "coloranimator.h"
#include "colorhelpers.h"
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"
#include "brushsettings.h"
#include "Animators/gradient.h"

enum PaintType : short {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT,
    BRUSHPAINT
};

class PathBox;
class SkStroke;

class GradientPoints;

class PaintSettingsAnimator : public ComplexAnimator {
protected:
    PaintSettingsAnimator(const QString &name,
                          GradientPoints * const grdPts,
                          PathBox * const parent);

    PaintSettingsAnimator(
            const QString &name,
            GradientPoints * const grdPts,
            PathBox * const parent,
            const QColor &colorT,
            const PaintType &paintTypeT,
            Gradient * const gradientT = nullptr);

    virtual void showHideChildrenBeforeChaningPaintType(
            const PaintType &newPaintType);
public:
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    QColor getCurrentColor() const;
    const PaintType &getPaintType() const;
    Gradient *getGradient() const;
    void setGradient(Gradient *gradient);
    void setCurrentColor(const QColor &color);
    void setPaintType(const PaintType &paintType);
    ColorAnimator *getColorAnimator();
    void setGradientPoints(GradientPoints * const gradientPoints);
    void setGradientPointsPos(const QPointF& pt1, const QPointF& pt2);

    void duplicateColorAnimatorFrom(ColorAnimator *source);
    void setGradientVar(Gradient * const grad);
    QColor getColorAtRelFrame(const qreal &relFrame) const;
    Gradient::Type getGradientType() { return mGradientType; }
    void setGradientType(const Gradient::Type &type) {
        if(mGradientType == type) return;
        mGradientType = type;
        prp_callFinishUpdater();
    }
private:
    Gradient::Type mGradientType = Gradient::LINEAR;
    PaintType mPaintType = NOPAINT;

    PathBox * const mTarget_k;
    qptr<GradientPoints> mGradientPoints;
    qsptr<ColorAnimator> mColor = SPtrCreate(ColorAnimator)();
    qptr<Gradient> mGradient;
};

class FillSettingsAnimator : public PaintSettingsAnimator {
    friend class SelfRef;
public:
    bool SWT_isFillSettingsAnimator() const { return true; }
protected:
    FillSettingsAnimator(GradientPoints * const grdPts,
                         PathBox * const parent) :
        FillSettingsAnimator(grdPts, parent, QColor(0, 0, 0),
                             PaintType::NOPAINT, nullptr) {}

    FillSettingsAnimator(GradientPoints * const grdPts,
                         PathBox * const parent,
                         const QColor &color,
                         const PaintType &paintType,
                         Gradient * const gradient = nullptr) :
        PaintSettingsAnimator("fill", grdPts, parent, color,
                              paintType, gradient) {}
};

struct UpdatePaintSettings {
    UpdatePaintSettings(const QColor &paintColorT,
                        const PaintType &paintTypeT);
    UpdatePaintSettings();

    virtual ~UpdatePaintSettings();

    virtual void applyPainterSettingsSk(SkPaint *paint);

    void updateGradient(const QGradientStops &stops,
                        const QPointF &start,
                        const QPointF &finalStop,
                        const Gradient::Type &gradientType);
    PaintType fPaintType;
    QColor fPaintColor;
    sk_sp<SkShader> fGradient;
};

struct UpdateStrokeSettings : UpdatePaintSettings {
    UpdateStrokeSettings(
            const qreal& width,
            const QColor &paintColorT,
            const PaintType &paintTypeT,
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

class OutlineSettingsAnimator : public PaintSettingsAnimator {
    friend class SelfRef;
public:
    bool SWT_isStrokeSettings() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
protected:
    void showHideChildrenBeforeChaningPaintType(
                const PaintType &newPaintType);
public:
    void setCurrentStrokeWidth(const qreal &newWidth);
    void setCapStyle(const Qt::PenCapStyle &capStyle);
    void setJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setStrokerSettings(QPainterPathStroker * const stroker);
    void setStrokerSettingsSk(SkStroke * const stroker);

    void setStrokeBrushSpacingCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushSpacingCurve(curve);
    }

    void setStrokeBrushPressureCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushPressureCurve(curve);
    }

    void setStrokeBrushTimeCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushTimeCurve(curve);
    }

    void setStrokeBrushWidthCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushWidthCurve(curve);
    }

    void setStrokeBrush(SimpleBrushWrapper* const brush) {
        mBrushSettings->setBrush(brush);
    }

    qreal getCurrentStrokeWidth() const;

    Qt::PenCapStyle getCapStyle() const;
    Qt::PenJoinStyle getJoinStyle() const;

    QrealAnimator *getStrokeWidthAnimator();
    BrushSettings *getBrushSettings() {
        return mBrushSettings.get();
    }

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    QPainter::CompositionMode getOutlineCompositionMode();

    void setLineWidthUpdaterTarget(PathBox * const path);
    bool nonZeroLineWidth();

    QrealAnimator *getLineWidthAnimator();

    void setStrokerSettingsForRelFrameSk(const qreal &relFrame,
                                         SkStroke * const stroker);
protected:
    OutlineSettingsAnimator(GradientPoints * const grdPts,
                            PathBox * const parent);

    OutlineSettingsAnimator(GradientPoints* const grdPts,
                            PathBox * const parent,
                            const QColor &color,
                            const PaintType &paintType,
                            Gradient* const gradient = nullptr);
private:
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qsptr<BrushSettings> mBrushSettings =
            SPtrCreate(BrushSettings)();
    qsptr<QrealAnimator> mLineWidth =
            SPtrCreate(QrealAnimator)(1, 0, 999, 1, "thickness");
};
#endif // PAINTSETTINGS_H
