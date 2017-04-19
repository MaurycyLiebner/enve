#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

#include "coloranimator.h"
#include "Colors/ColorWidgets/colorvaluerect.h"

class GradientWidget;

enum PaintType {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

class PathBox;


class GradientPoints;

enum ColorSettingType : short {
    CST_START,
    CST_CHANGE,
    CST_FINISH
};

class ColorSetting {
public:
    ColorSetting() {}
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

    const ColorSettingType &getType() const { return mType; }
    const ColorMode &getSettingMode() const { return mSettingMode; }
    const CVR_TYPE &getChangedValue() const { return mChangedValue; }
    const qreal &getVal1() const { return mVal1; }
    const qreal &getVal2() const { return mVal2; }
    const qreal &getVal3() const { return mVal3; }
    const qreal &getAlpa() const { return mAlpha; }
private:
    void finishColorTransform(ColorAnimator *target) const;

    void changeColor(ColorAnimator *target) const;

    void startColorTransform(ColorAnimator *target) const;
    ColorSettingType mType = CST_FINISH;
    ColorMode mSettingMode = RGBMODE;
    CVR_TYPE mChangedValue = CVR_ALL;
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

    PaintSettings(Color colorT,
                  PaintType paintTypeT,
                  Gradient *gradientT = NULL);

    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);

    Color getCurrentColor() const;

    PaintType getPaintType() const;

    Gradient *getGradient() const;

    void setGradient(Gradient *gradient, bool saveUndoRedo = true);

    void setCurrentColor(Color color);

    void setPaintType(PaintType paintType, bool saveUndoRedo = true);

    ColorAnimator *getColorAnimator();

    void setGradientPoints(GradientPoints *gradientPoints);

    void prp_loadFromSql(const int &sqlId);
    void setPaintPathTarget(PathBox *path);

    void prp_makeDuplicate(Property *target);
    Property *prp_makeDuplicate() {
        return NULL;
    }

    void duplicateColorAnimatorFrom(ColorAnimator *source);

    void setTargetPathBox(PathBox *target);

    void setGradientVar(Gradient *grad);
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

    Gradient(Color color1, Color color2);

    Gradient(int sqlIdT);

    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);

    void saveToSqlIfPathSelected(QSqlQuery *query);

    void swapColors(int id1, int id2,
                    const bool &saveUndoRedo = true);

    void removeColor(ColorAnimator *color,
                     const bool &saveUndoRedo = true);

    void addColor(Color color);

    void replaceColor(int id, Color color);

    void setColors(QList<Color> newColors);

    void prp_startTransform();

    bool isInPaths(PathBox *path);

    void addPath(PathBox *path);

    void removePath(PathBox *path);

    bool affectsPaths();

    void updatePaths();

    //void finishTransform();

    void updateQGradientStops();

    int getSqlId();
    void setSqlId(int id);

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
    ColorAnimator *getColorAnimatorAt(int id);
    void removeColor(const int &id);
    Property *prp_makeDuplicate();

    void updateQGradientStopsFinal();
    bool isEmpty() const;
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
                        const QLinearGradient &gradientT) {
        paintColor = paintColorT;
        paintType = paintTypeT;
        gradient = gradientT;
    }

    UpdatePaintSettings() {}

    virtual void applyPainterSettings(QPainter *p) {
        if(paintType == GRADIENTPAINT) {
            p->setBrush(gradient);
        } else if(paintType == FLATPAINT) {
            p->setBrush(paintColor);
        } else {
            p->setBrush(Qt::NoBrush);
        }
    }

    void updateGradient(const QGradientStops &stops,
                        const QPointF &start,
                        const QPointF &finalStop) {
        gradient.setStops(stops);
        gradient.setStart(start);
        gradient.setFinalStop(finalStop);
    }

    QColor paintColor;
    PaintType paintType;
    QLinearGradient gradient;
};

struct UpdateStrokeSettings : UpdatePaintSettings {
    UpdateStrokeSettings(
            const QColor &paintColorT,
            const PaintType &paintTypeT,
            const QLinearGradient &gradientT,
            const QPainter::CompositionMode &outlineCompositionModeT) :
                UpdatePaintSettings(paintColorT, paintTypeT, gradientT) {
        outlineCompositionMode = outlineCompositionModeT;
    }

    UpdateStrokeSettings() {}

    void applyPainterSettings(QPainter *p) {
        UpdatePaintSettings::applyPainterSettings(p);
        p->setCompositionMode(outlineCompositionMode);
    }

    QPainter::CompositionMode outlineCompositionMode =
            QPainter::CompositionMode_Source;
};

class StrokeSettings : public PaintSettings
{
public:
    StrokeSettings();

    StrokeSettings(Color colorT,
                   PaintType paintTypeT,
                   Gradient *gradientT = NULL);

    int prp_saveToSql(QSqlQuery *query,
                      const int &parentId = 0);

    void setCurrentStrokeWidth(qreal newWidth);

    void setCapStyle(Qt::PenCapStyle capStyle);

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setStrokerSettings(QPainterPathStroker *stroker);
    StrokeSettings(int strokeSqlId, int paintSqlId, GradientWidget *gradientWidget);

    qreal getCurrentStrokeWidth() const;

    Qt::PenCapStyle getCapStyle() const;

    Qt::PenJoinStyle getJoinStyle() const;

    QrealAnimator *getStrokeWidthAnimator();

    void setOutlineCompositionMode(QPainter::CompositionMode compositionMode);

    QPainter::CompositionMode getOutlineCompositionMode();

    void setLineWidthUpdaterTarget(PathBox *path);
    void prp_loadFromSql(const int &strokeSqlId);
    bool nonZeroLineWidth();

    void prp_makeDuplicate(Property *target);

    void duplicateLineWidthFrom(QrealAnimator *source);

    QrealAnimator *getLineWidthAnimator();

private:
    QSharedPointer<QrealAnimator> mLineWidth =
            (new QrealAnimator())->ref<QrealAnimator>();
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
};
#endif // PAINTSETTINGS_H
