#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

#include "qrealanimator.h"
#include "coloranimator.h"
#include "Colors/color.h"
#include "skiaincludes.h"

enum PaintType : short {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

class PathBox;
class SkStroke;

class GradientPoints;

enum ColorSettingType : short {
    CST_START,
    CST_CHANGE,
    CST_FINISH
};

enum CVR_TYPE : short;

class ColorSetting {
public:
    ColorSetting();
    ColorSetting(
            const ColorMode &settingModeT,
            const CVR_TYPE &changedValueT,
            const qreal &val1T,
            const qreal &val2T,
            const qreal &val3T,
            const qreal &alphaT,
            const ColorSettingType &typeT,
            ColorAnimator *excludeT = nullptr);
    void apply(ColorAnimator *target) const;

    const ColorSettingType &getType() const;
    const ColorMode &getSettingMode() const;
    const CVR_TYPE &getChangedValue() const;
    const qreal &getVal1() const;
    const qreal &getVal2() const;
    const qreal &getVal3() const;
    const qreal &getAlpa() const;
private:
    void finishColorTransform(ColorAnimator *target) const;

    void changeColor(ColorAnimator *target) const;

    void startColorTransform(ColorAnimator *target) const;
    ColorSettingType mType = CST_FINISH;
    ColorMode mSettingMode = RGBMODE;
    CVR_TYPE mChangedValue;
    qreal mVal1 = 1.;
    qreal mVal2 = 1.;
    qreal mVal3 = 1.;
    qreal mAlpha = 1.;
    ColorAnimator *mExclude = nullptr;
};

class Gradient;
class PaintSetting {
public:
    PaintSetting(const bool &targetFillSettings,
                 const ColorSetting &colorSetting);

    PaintSetting(const bool &targetFillSettings);

    PaintSetting(const bool &targetFillSettings,
                 const bool &linearGradient,
                 Gradient *gradient);

    void apply(PathBox *box) const;

    void applyColorSetting(ColorAnimator *animator) const;

    bool targetsFill() const { return mTargetFillSettings; }
private:
    bool mTargetFillSettings;
    bool mLinearGradient = true;
    Gradient *mGradient;
    PaintType mPaintType;
    ColorSetting mColorSetting;
};

class PaintSettings : public ComplexAnimator {
public:
    PaintSettings();

    PaintSettings(const Color &colorT,
                  const PaintType &paintTypeT,
                  Gradient *gradientT = nullptr);

    Color getCurrentColor() const;

    PaintType getPaintType() const;

    Gradient *getGradient() const;

    void setGradient(Gradient *gradient,
                     const bool &saveUndoRedo = true);

    void setCurrentColor(const Color &color);

    void setPaintType(const PaintType &paintType,
                      const bool &saveUndoRedo = true);

    ColorAnimator *getColorAnimator();

    void setGradientPoints(GradientPoints *gradientPoints);

    void setPaintPathTarget(PathBox *path);


    void duplicateColorAnimatorFrom(ColorAnimator *source);

    void setTargetPathBox(PathBox *target);

    void setGradientVar(Gradient *grad);

    bool SWT_isPaintSettings() { return true; }
    Color getColorAtRelFrame(const int &relFrame) const;
    Color getColorAtRelFrameF(const qreal &relFrame) const;
    const bool &getGradientLinear() { return mGradientLinear; }
    void setGradientLinear(const bool &linear) {
        mGradientLinear = linear;
    }

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    bool mGradientLinear = true;
    PathBox *mTarget;
    GradientPoints *mGradientPoints = nullptr;
    QSharedPointer<ColorAnimator> mColor =
            (new ColorAnimator())->ref<ColorAnimator>();
    PaintType mPaintType = FLATPAINT;
    QSharedPointer<Gradient> mGradient;
};

class Gradient : public ComplexAnimator {
    Q_OBJECT
public:
    Gradient();
    ~Gradient();

    Gradient(const Color &color1,
             const Color &color2);

    void swapColors(const int &id1, const int &id2,
                    const bool &saveUndoRedo = true);

    void removeColor(ColorAnimator *color,
                     const bool &saveUndoRedo = true);

    void addColor(const Color &color);

    void replaceColor(const int &id,
                      const Color &color);

    void prp_startTransform();

    bool isInPaths(PathBox *path);

    void addPath(PathBox *path);

    void removePath(PathBox *path);

    bool affectsPaths();

    void updatePaths(const UpdateReason &reason);

    //void finishTransform();

    void updateQGradientStops(const UpdateReason &reason);

    int getLoadId();
    void setLoadId(const int &id);

    void addColorToList(const Color &color,
                        const bool &saveUndoRedo = true);

    Color getCurrentColorAt(const int &id);

    int getColorCount();

    QColor getLastQGradientStopQColor();
    QColor getFirstQGradientStopQColor();

    QGradientStops getQGradientStops();
    void startColorIdTransform(int id);
    void addColorToList(ColorAnimator *newColorAnimator,
                        const bool &saveUndoRedo = true);
    ColorAnimator *getColorAnimatorAt(const int &id);
    void removeColor(const int &id);

    void updateQGradientStopsFinal(const UpdateReason &reason);
    bool isEmpty() const;

    bool SWT_isGradient() { return true; }

    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator = nullptr) {
        Q_UNUSED(shift);
        if(parentAnimator == nullptr) return;
        Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
            parentAnimator->ca_updateDescendatKeyFrame(key.get());
        }
    }

    int prp_getFrameShift() const {
        return 0;
    }

    int prp_getParentFrameShift() const {
        return 0;
    }
    QGradientStops getQGradientStopsAtAbsFrame(const int &absFrame);
    QGradientStops getQGradientStopsAtAbsFrameF(const qreal &absFrame);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

signals:
    void resetGradientWidgetColorIdIfEquals(Gradient *, int);
private:
    int mLoadId = -1;
    QGradientStops mQGradientStops;
    QList<ColorAnimator*> mColors;
    QList<PathBox*> mAffectedPaths;
    ColorAnimator *mCurrentColor = nullptr;
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

    QColor paintColor;
    PaintType paintType;
    sk_sp<SkShader> gradientSk;
};

struct UpdateStrokeSettings : UpdatePaintSettings {
    UpdateStrokeSettings(const QColor &paintColorT,
            const PaintType &paintTypeT,
            const QPainter::CompositionMode &outlineCompositionModeT);

    UpdateStrokeSettings();

    void applyPainterSettingsSk(SkPaint *paint);

    QPainter::CompositionMode outlineCompositionMode =
            QPainter::CompositionMode_Source;
};

class StrokeSettings : public PaintSettings
{
public:
    StrokeSettings();

    StrokeSettings(const Color &colorT,
                   const PaintType &paintTypeT,
                   Gradient *gradientT = nullptr);

    void setCurrentStrokeWidth(const qreal &newWidth);

    void setCapStyle(const Qt::PenCapStyle &capStyle);

    void setJoinStyle(const Qt::PenJoinStyle &joinStyle);

    void setStrokerSettings(QPainterPathStroker *stroker);
    void setStrokerSettingsSk(SkStroke *stroker);

    qreal getCurrentStrokeWidth() const;

    Qt::PenCapStyle getCapStyle() const;

    Qt::PenJoinStyle getJoinStyle() const;

    QrealAnimator *getStrokeWidthAnimator();

    void setOutlineCompositionMode(
            const QPainter::CompositionMode &compositionMode);

    QPainter::CompositionMode getOutlineCompositionMode();

    void setLineWidthUpdaterTarget(PathBox *path);
    bool nonZeroLineWidth();

    QrealAnimator *getLineWidthAnimator();

    bool SWT_isStrokeSettings() { return true; }
    void setStrokerSettingsForRelFrameSk(const int &relFrame,
                                         SkStroke *stroker);
    void setStrokerSettingsForRelFrameSkF(const qreal &relFrame,
                                          SkStroke *stroker);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QSharedPointer<QrealAnimator> mLineWidth =
            (new QrealAnimator())->ref<QrealAnimator>();
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
};
#endif // PAINTSETTINGS_H
