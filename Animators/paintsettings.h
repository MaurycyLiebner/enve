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
            ColorAnimator *excludeT = NULL);
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
    ColorAnimator *mExclude = NULL;
};

class Gradient;
class PaintSetting{
public:
    PaintSetting(const bool &targetFillSettings,
                 const ColorSetting &colorSetting);

    PaintSetting(const bool &targetFillSettings);

    PaintSetting(const bool &targetFillSettings,
                 Gradient *gradient);

    void apply(PathBox *box) const;

    void applyColorSetting(ColorAnimator *animator) const;

    bool targetsFill() const { return mTargetFillSettings; }
private:
    bool mTargetFillSettings;
    Gradient *mGradient;
    PaintType mPaintType;
    ColorSetting mColorSetting;
};

class PaintSettings : public ComplexAnimator {
public:
    PaintSettings();

    PaintSettings(const Color &colorT,
                  const PaintType &paintTypeT,
                  Gradient *gradientT = NULL);

    int prp_saveToSql(QSqlQuery *query,
                      const int &parentId = 0);

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

    void prp_loadFromSql(const int &sqlId);
    void setPaintPathTarget(PathBox *path);

    void makeDuplicate(Property *target);
    Property *makeDuplicate() {
        return NULL;
    }

    void duplicateColorAnimatorFrom(ColorAnimator *source);

    void setTargetPathBox(PathBox *target);

    void setGradientVar(Gradient *grad);

    bool SWT_isPaintSettings() { return true; }
    Color getColorAtRelFrame(const int &relFrame) const;
private:
    PathBox *mTarget;
    GradientPoints *mGradientPoints = NULL;
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

    Gradient(const int &sqlIdT);

    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);

    void saveToSqlIfPathSelected(QSqlQuery *query);

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

    void updatePaths();

    //void finishTransform();

    void updateQGradientStops();

    int getSqlId();
    void setSqlId(const int &id);

    void addColorToList(const Color &color,
                        const bool &saveUndoRedo = true);

    Color getCurrentColorAt(int id);

    int getColorCount();

    QColor getLastQGradientStopQColor();
    QColor getFirstQGradientStopQColor();

    QGradientStops getQGradientStops();
    void scheduleQGradientStopsUpdate();
    void updateQGradientStopsIfNeeded();
    void startColorIdTransform(int id);
    void addColorToList(ColorAnimator *newColorAnimator,
                        const bool &saveUndoRedo = true);
    ColorAnimator *getColorAnimatorAt(const int &id);
    void removeColor(const int &id);
    Property *makeDuplicate();

    void updateQGradientStopsFinal();
    bool isEmpty() const;

    bool SWT_isGradient() { return true; }

    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator = NULL) {
        Q_UNUSED(shift);
        if(parentAnimator == NULL) return;
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
signals:
    void resetGradientWidgetColorIdIfEquals(Gradient *, int);
private:
    int mSqlId = -1;
    QGradientStops mQGradientStops;
    QList<ColorAnimator*> mColors;
    QList<PathBox*> mAffectedPaths;
    ColorAnimator *mCurrentColor = NULL;

    bool mQGradientStopsUpdateNeeded = false;
};

struct UpdatePaintSettings {
    UpdatePaintSettings(const QColor &paintColorT,
                        const PaintType &paintTypeT,
                        const QLinearGradient &gradientT);

    UpdatePaintSettings();

    virtual ~UpdatePaintSettings();

    virtual void applyPainterSettingsSk(SkPaint *paint);

    void updateGradient(const QGradientStops &stops,
                        const QPointF &start,
                        const QPointF &finalStop);

    QColor paintColor;
    PaintType paintType;
    QLinearGradient gradient;
    sk_sp<SkShader> gradientSk;
};

struct UpdateStrokeSettings : UpdatePaintSettings {
    UpdateStrokeSettings(
            const QColor &paintColorT,
            const PaintType &paintTypeT,
            const QLinearGradient &gradientT,
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
                   Gradient *gradientT = NULL);
//    StrokeSettings(const int &strokeSqlId,
//                   const int &paintSqlId,
//                   GradientWidget *gradientWidget);

    int prp_saveToSql(QSqlQuery *query,
                      const int &parentId = 0);

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
    void prp_loadFromSql(const int &strokeSqlId);
    bool nonZeroLineWidth();

    void makeDuplicate(Property *target);

    void duplicateLineWidthFrom(QrealAnimator *source);

    QrealAnimator *getLineWidthAnimator();

    bool SWT_isStrokeSettings() { return true; }
    void setStrokerSettingsForRelFrameSk(const int &relFrame,
                                         SkStroke *stroker);
private:
    QSharedPointer<QrealAnimator> mLineWidth =
            (new QrealAnimator())->ref<QrealAnimator>();
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
};
#endif // PAINTSETTINGS_H
