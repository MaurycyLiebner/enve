#ifndef OUTLINESETTINGSANIMATOR_H
#define OUTLINESETTINGSANIMATOR_H
#include "paintsettingsanimator.h"

class OutlineSettingsAnimator : public PaintSettingsAnimator {
    e_OBJECT
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
public:
    void strokeWidthAction(const QrealAction& action);
    void setCurrentStrokeWidth(const qreal newWidth);
    void setCapStyle(const SkPaint::Cap capStyle);
    void setJoinStyle(const SkPaint::Join joinStyle);
    void setStrokerSettingsSk(SkStroke * const stroker);

    qreal getCurrentStrokeWidth() const;

    SkPaint::Cap getCapStyle() const;
    SkPaint::Join getJoinStyle() const;

    QrealAnimator *getStrokeWidthAnimator();

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
    qsptr<QrealAnimator> mLineWidth =
            enve::make_shared<QrealAnimator>(1, 0, 999, 1, "thickness");
};
#endif // OUTLINESETTINGSANIMATOR_H
