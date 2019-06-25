#ifndef OUTLINESETTINGSANIMATOR_H
#define OUTLINESETTINGSANIMATOR_H
#include "paintsettingsanimator.h"

class OutlineSettingsAnimator : public PaintSettingsAnimator {
    friend class SelfRef;
protected:
    OutlineSettingsAnimator(GradientPoints * const grdPts,
                            PathBox * const parent);

    OutlineSettingsAnimator(GradientPoints* const grdPts,
                            PathBox * const parent,
                            const QColor &color,
                            const PaintType &paintType,
                            Gradient* const gradient = nullptr);
public:
    bool SWT_isStrokeSettings() const { return true; }
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
protected:
    void showHideChildrenBeforeChaningPaintType(
                const PaintType &newPaintType);
public:
    void setCurrentStrokeWidth(const qreal newWidth);
    void setCapStyle(const SkPaint::Cap capStyle);
    void setJoinStyle(const SkPaint::Join joinStyle);
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

    SkPaint::Cap getCapStyle() const;
    SkPaint::Join getJoinStyle() const;

    QrealAnimator *getStrokeWidthAnimator();
    BrushSettingsAnimator *getBrushSettings() {
        return mBrushSettings.get();
    }
    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    QPainter::CompositionMode getOutlineCompositionMode();
    void setLineWidthUpdaterTarget(PathBox * const path);
    bool nonZeroLineWidth();

    QrealAnimator *getLineWidthAnimator();

    void setStrokerSettingsForRelFrameSk(const qreal relFrame,
                                         SkStroke * const stroker);
private:
    SkPaint::Cap mCapStyle = SkPaint::kRound_Cap;
    SkPaint::Join mJoinStyle = SkPaint::kRound_Join;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qsptr<BrushSettingsAnimator> mBrushSettings =
            SPtrCreate(BrushSettingsAnimator)();
    qsptr<QrealAnimator> mLineWidth =
            SPtrCreate(QrealAnimator)(1, 0, 999, 1, "thickness");
};
#endif // OUTLINESETTINGSANIMATOR_H
