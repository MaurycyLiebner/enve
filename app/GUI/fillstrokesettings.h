#ifndef FILLSTROKESETTINGS_H
#define FILLSTROKESETTINGS_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTabBar>
#include <QPen>
#include <QGradient>
#include <QDebug>
#include <QTimer>
#include <QPainterPathStroker>
#include <QLabel>
#include <QHBoxLayout>
#include "Animators/coloranimator.h"
#include "Animators/paintsettingsanimator.h"
#include "Animators/brushsettingsanimator.h"
#include "GUI/BrushWidgets/brushselectionwidget.h"
#include "paintsettingsapplier.h"
class GradientWidget;
class MainWindow;
class CanvasWindow;
class ColorSettingsWidget;
class QrealAnimatorValueSlider;
class ActionButton;
class Segment1DEditor;
class QDockWidget;
class ColorSettingApplier;

class FillStrokeSettingsWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(Document& document,
                                      MainWindow *parent = nullptr);

    void setCurrentSettings(PaintSettingsAnimator *fillPaintSettings,
                            OutlineSettingsAnimator *strokePaintSettings);
    void updateAfterTargetChanged();
    void setCurrentPaintType(const PaintType &paintType);

    GradientWidget *getGradientWidget();

    void clearAll();
    void setFillValuesFromFillSettings(PaintSettingsAnimator *settings);
    void setStrokeValuesFromStrokeSettings(OutlineSettingsAnimator *settings);

    void updateColorAnimator();
    void setCurrentBrushSettings(BrushSettingsAnimator * const brushSettings);

    void emitStrokeBrushChanged();
    void emitStrokeBrushWidthCurveChanged();
    void emitStrokeBrushTimeCurveChanged();
    void emitStrokeBrushSpacingCurveChanged();
    void emitStrokeBrushPressureCurveChanged();

    void emitStrokeWidthChanged();
    void emitStrokeWidthChangedTMP();
    void emitCapStyleChanged();
    void emitJoinStyleChanged();
private:
    void setLinearGradientFill();
    void setRadialGradientFill();

    void setStrokeBrush(SimpleBrushWrapper * const brush);
    void setBrushTimeCurve(const qCubicSegment1D& seg);
    void setBrushWidthCurve(const qCubicSegment1D& seg);
    void setBrushPressureCurve(const qCubicSegment1D& seg);
    void setBrushSpacingCurve(const qCubicSegment1D& seg);
    void setStrokeWidth(const qreal width);

    void colorTypeSet(const PaintType &type);
    void setFillTarget();
    void setStrokeTarget();

    void setGradient(Gradient* gradient);
    void setGradientType(const GradientType &type);

    void setBevelJoinStyle();
    void setMiterJoinStyle();
    void setRoundJoinStyle();

    void setFlatCapStyle();
    void setSquareCapStyle();
    void setRoundCapStyle();

    void setGradientFill();
    void setBrushFill();
    void setFlatFill();
    void setNoneFill();

    void setColorAnimatorTarget(ColorAnimator *animator);
    void colorSettingReceived(const ColorSettingApplier &colorSetting);
    void setCurrentColorMode(const ColorMode &mode);
private:
    void updateCurrentSettings();
    void applyGradient();


    void connectGradient();
    void disconnectGradient();

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setCapStyle(Qt::PenCapStyle capStyle);

    //

    PaintType getCurrentPaintTypeVal();

    void setCurrentPaintTypeVal(const PaintType& paintType);

    QColor getColorVal();

    void setCurrentColorVal(const QColor& color);

    Gradient *getCurrentGradientVal();

    const GradientType &getCurrentGradientTypeVal() {
        if(mTarget == PaintSetting::FILL) return mCurrentFillGradientType;
        else return mCurrentStrokeGradientType;
    }

    void setCurrentGradientVal(Gradient *gradient);
    void setCurrentGradientTypeVal(const GradientType &type);

    Document& mDocument;
    MainWindow *mMainWindow;
    bool mTransormStarted = false;
    PaintSetting::Target mTarget = PaintSetting::FILL;

    qptr<ColorAnimator> mCurrentFillColorAnimator;
    qptr<ColorAnimator> mCurrentStrokeColorAnimator;
    PaintType mCurrentFillPaintType = NOPAINT;
    PaintType mCurrentStrokePaintType = NOPAINT;
    QColor mCurrentFillColor;
    QColor mCurrentStrokeColor;

    GradientType mCurrentStrokeGradientType = GradientType::LINEAR;
    GradientType mCurrentFillGradientType = GradientType::LINEAR;

    qptr<Gradient> mCurrentStrokeGradient;
    qptr<Gradient> mCurrentFillGradient;
    Qt::PenCapStyle mCurrentCapStyle;
    Qt::PenJoinStyle mCurrentJoinStyle;
    qreal mCurrentStrokeWidth;
    SimpleBrushWrapper* mCurrentStrokeBrush = nullptr;
    qCubicSegment1D mCurrentStrokeBrushWidthCurve;
    qCubicSegment1D mCurrentStrokeBrushTimeCurve;
    qCubicSegment1D mCurrentStrokeBrushPressureCurve;
    qCubicSegment1D mCurrentStrokeBrushSpacingCurve;

    //

    void setBrushPaintType();
    void setNoPaintType();
    void setFlatPaintType();
    void setGradientPaintType();

    QVBoxLayout *mMainLayout = new QVBoxLayout();

    QHBoxLayout *mTargetLayout = new QHBoxLayout();
    QPushButton *mFillTargetButton;
    QPushButton *mStrokeTargetButton;

    QHBoxLayout *mColorTypeLayout;
    QPushButton *mFillNoneButton = nullptr;
    QPushButton *mFillFlatButton = nullptr;
    QPushButton *mFillGradientButton = nullptr;
    QPushButton *mFillBrushButton = nullptr;

    QWidget *mStrokeSettingsWidget;
    QWidget* mStrokeJoinCapWidget;
    QVBoxLayout *mStrokeSettingsLayout = new QVBoxLayout();

    QHBoxLayout *mJoinStyleLayout = new QHBoxLayout();
    QPushButton *mBevelJoinStyleButton;
    QPushButton *mMiterJointStyleButton;
    QPushButton *mRoundJoinStyleButton;

    QHBoxLayout *mCapStyleLayout = new QHBoxLayout();
    QPushButton *mFlatCapStyleButton;
    QPushButton *mSquareCapStyleButton;
    QPushButton *mRoundCapStyleButton;

    QHBoxLayout *mLineWidthLayout = new QHBoxLayout();
    QLabel *mLineWidthLabel = new QLabel("Width:");
    QrealAnimatorValueSlider *mLineWidthSpin;

    ColorSettingsWidget *mColorsSettingsWidget;

    GradientWidget *mGradientWidget;

    QHBoxLayout *mGradientTypeLayout;
    QPushButton *mLinearGradientButton;
    QPushButton *mRadialGradientButton;
    QWidget *mGradientTypeWidget;

    QWidget *mFillAndStrokeWidget;

    BrushSelectionWidget* mBrushSelectionWidget;

    QWidget* mBrushSettingsWidget;
    Segment1DEditor* mBrushWidthCurveEditor;
    Segment1DEditor* mBrushPressureCurveEditor;
    Segment1DEditor* mBrushSpacingCurveEditor;
    Segment1DEditor* mBrushTimeCurveEditor;
};

#endif // FILLSTROKESETTINGS_H
