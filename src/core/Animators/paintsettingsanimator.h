#ifndef PAINTSETTINGSANIMATOR_H
#define PAINTSETTINGSANIMATOR_H

#include "Animators/qrealanimator.h"
#include "coloranimator.h"
#include "colorhelpers.h"
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"
#include "Animators/gradient.h"
#include "Animators/staticcomplexanimator.h"
#include "paintsettings.h"

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
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);

    QColor getColor() const;
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
    QColor getColor(const qreal relFrame) const;
    GradientType getGradientType() { return mGradientType; }
    void setGradientType(const GradientType &type) {
        if(mGradientType == type) return;
        mGradientType = type;
        prp_callFinishUpdater();
    }
private:
    GradientType mGradientType = GradientType::LINEAR;
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
                        const GradientType &gradientType);
    PaintType fPaintType;
    QColor fPaintColor;
    sk_sp<SkShader> fGradient;
};

struct UpdateStrokeSettings : UpdatePaintSettings {
    UpdateStrokeSettings(
            const qreal width,
            const QColor &paintColorT,
            const PaintType &paintTypeT,
            const QPainter::CompositionMode &outlineCompositionModeT);
    UpdateStrokeSettings();

    void applyPainterSettingsSk(SkPaint *paint);

    QPainter::CompositionMode fOutlineCompositionMode =
            QPainter::CompositionMode_Source;

    qreal fOutlineWidth;
};

#endif // PAINTSETTINGSANIMATOR_H
