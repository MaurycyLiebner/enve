#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

#include "qrealanimator.h"
#include "coloranimator.h"
#include "colorhelpers.h"
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"
#include "brushsettings.h"

enum PaintType : short {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT,
    BRUSHPAINT
};

class PathBox;
class SkStroke;

class GradientPoints;

class Gradient;
class PaintSettings : public ComplexAnimator {
    friend class SelfRef;
public:
    bool SWT_isPaintSettings() const { return true; }

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

    void setPaintPathTarget(PathBox * const path);

    void duplicateColorAnimatorFrom(ColorAnimator *source);

    void setGradientVar(Gradient * const grad);

    QColor getColorAtRelFrame(const qreal &relFrame) const;
    const bool &getGradientLinear() { return mGradientLinear; }
    void setGradientLinear(const bool &linear) {
        mGradientLinear = linear;
    }

protected:
    PaintSettings(GradientPoints * const grdPts, PathBox * const parent);

    PaintSettings(GradientPoints * const grdPts,
                  PathBox * const parent,
                  const QColor &colorT,
                  const PaintType &paintTypeT,
                  Gradient * const gradientT = nullptr);
    virtual void showHideChildrenBeforeChaningPaintType(
            const PaintType &newPaintType);
private:
    bool mGradientLinear = true;
    PaintType mPaintType = FLATPAINT;

    PathBox * const mTarget_k;
    qptr<GradientPoints> mGradientPoints;
    qsptr<ColorAnimator> mColor = SPtrCreate(ColorAnimator)();
    qptr<Gradient> mGradient;
};

class Gradient : public ComplexAnimator {
    Q_OBJECT
    friend class SelfRef;
public:
    bool SWT_isGradient() const { return true; }
    void prp_startTransform();
    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator* parentAnimator = nullptr) {
        Q_UNUSED(shift);
        if(!parentAnimator) return;
        for(const auto &key : anim_mKeys) {
            parentAnimator->ca_updateDescendatKeyFrame(key.get());
        }
    }

    int prp_getFrameShift() const {
        return 0;
    }

    int prp_getParentFrameShift() const {
        return 0;
    }

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    void swapColors(const int &id1, const int &id2);
    void removeColor(const qsptr<ColorAnimator> &color);
    void addColor(const QColor &color);
    void replaceColor(const int &id, const QColor &color);
    void addPath(PathBox * const path);
    void removePath(PathBox * const path);
    bool affectsPaths();
    void updatePaths(const UpdateReason &reason);

    //void finishTransform();

    void updateQGradientStops(const UpdateReason &reason);

    int getLoadId();
    void setLoadId(const int &id);

    void addColorToList(const QColor &color);
    QColor getCurrentColorAt(const int &id);
    int getColorCount();

    QColor getLastQGradientStopQColor();
    QColor getFirstQGradientStopQColor();

    QGradientStops getQGradientStops();
    void startColorIdTransform(const int &id);
    void addColorToList(const qsptr<ColorAnimator> &newColorAnimator);
    ColorAnimator *getColorAnimatorAt(const int &id);
    void removeColor(const int &id);

    bool isEmpty() const;

    QGradientStops getQGradientStopsAtAbsFrame(const qreal &absFrame);
signals:
    void resetGradientWidgetColorIdIfEquals(Gradient *, int);
protected:
    Gradient();
    Gradient(const QColor &color1, const QColor &color2);
private:
    int mLoadId = -1;
    QGradientStops mQGradientStops;
    QList<qsptr<ColorAnimator>> mColors;
    QList<qptr<PathBox>> mAffectedPaths;
    qptr<ColorAnimator> mCurrentColor;
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
                        const bool &linearGradient = true);
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
};

class StrokeSettings : public PaintSettings {
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

    void setStrokeBrushPressureCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushPressureCurve(curve);
    }

    void setStrokeBrushTimeCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushTimeCurve(curve);
    }

    void setStrokeBrushWidthCurve(const qCubicSegment1D& curve) {
        mBrushSettings->setStrokeBrushWidthCurve(curve);
    }

    void setStrokeBrush(_SimpleBrushWrapper* const brush) {
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
    StrokeSettings(GradientPoints * const grdPts, PathBox * const parent);

    StrokeSettings(GradientPoints* const grdPts,
                   PathBox * const parent,
                   const QColor &colorT,
                   const PaintType &paintTypeT,
                   Gradient* const gradientT = nullptr);
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
