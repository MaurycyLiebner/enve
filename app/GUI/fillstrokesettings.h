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
#include "Animators/paintsettings.h"
#include "Animators/brushsettings.h"
#include "GUI/BrushWidgets/brushselectionwidget.h"
#include "GUI/BrushWidgets/brushselectionicon.h"
#include "paintsettingsapplier.h"
class GradientWidget;
class MainWindow;
class CanvasWindow;
class ColorSettingsWidget;
class QrealAnimatorValueSlider;
class ActionButton;
class Segment1DEditor;
class QDockWidget;
class ColorSetting;

class FillStrokeSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(MainWindow *parent = nullptr);

    void setCurrentSettings(PaintSettings *fillPaintSettings,
                            StrokeSettings *strokePaintSettings);
    void updateAfterTargetChanged();
    void setCurrentPaintType(const PaintType &paintType);

    GradientWidget *getGradientWidget();

    void clearAll();
    void setFillValuesFromFillSettings(PaintSettings *settings);
    void setStrokeValuesFromStrokeSettings(StrokeSettings *settings);

    void setCanvasWindowPtr(CanvasWindow *canvasWidget);
    void updateColorAnimator();
    void setCurrentBrushSettings(BrushSettings * const brushSettings);
    PaintSettingsApplier getCurrentSettingsApplier() const;
public slots:
    void emitStrokeBrushChanged();
    void emitStrokeBrushWidthCurveChanged();
    void emitStrokeBrushTimeCurveChanged();
    void emitStrokeBrushPressureCurveChanged();

    void emitStrokeWidthChanged();
    void emitStrokeWidthChangedTMP();
    void emitCapStyleChanged();
    void emitJoinStyleChanged();
private slots:
    void setLinearGradientFill();
    void setRadialGradientFill();

    void setStrokeBrush(_SimpleBrushWrapper * const brush);
    void setBrushTimeCurve(const qCubicSegment1D& seg);
    void setBrushWidthCurve(const qCubicSegment1D& seg);
    void setBrushPressureCurve(const qCubicSegment1D& seg);
    void setStrokeWidth(const qreal &width);

    void colorTypeSet(const PaintType &type);
    void setFillTarget();
    void setStrokeTarget();

    void setGradient(Gradient* gradient);
    void setGradientType(const Gradient::Type &type);

    void setBevelJoinStyle();
    void setMiterJoinStyle();
    void setRoundJoinStyle();

    void setFlatCapStyle();
    void setSquareCapStyle();
    void setRoundCapStyle();

    void waitToSaveChanges();

    void finishTransform();
    void startTransform(const char *slot);

    void setGradientFill();
    void setBrushFill();
    void setFlatFill();
    void setNoneFill();

    void setColorAnimatorTarget(ColorAnimator *animator);
    void colorSettingReceived(const ColorSetting &colorSetting);
    void setCurrentColorMode(const ColorMode &mode);
private:
    void setTransformFinishEmitter(const char *slot);
    void applyGradient();

    CanvasWindow *mCanvasWindow;

    MainWindow *mMainWindow;
    bool mTransormStarted = false;

    QTimer *mUndoRedoSaveTimer = nullptr;

    void connectGradient();
    void disconnectGradient();

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setCapStyle(Qt::PenCapStyle capStyle);


    PaintSetting::Target mTarget = PaintSetting::FILL;

    //

    PaintType getCurrentPaintTypeVal();

    void setCurrentPaintTypeVal(const PaintType& paintType);

    QColor getCurrentColorVal();

    void setCurrentColorVal(const QColor& color);

    Gradient *getCurrentGradientVal();

    const Gradient::Type &getCurrentGradientTypeVal() {
        if(mTarget == PaintSetting::FILL) return mCurrentFillGradientType;
        else return mCurrentStrokeGradientType;
    }

    void setCurrentGradientVal(Gradient *gradient);
    void setCurrentGradientTypeVal(const Gradient::Type &type);

    qptr<ColorAnimator> mCurrentFillColorAnimator;
    qptr<ColorAnimator> mCurrentStrokeColorAnimator;
    PaintType mCurrentFillPaintType = NOPAINT;
    PaintType mCurrentStrokePaintType = NOPAINT;
    QColor mCurrentFillColor;
    QColor mCurrentStrokeColor;

    Gradient::Type mCurrentStrokeGradientType = Gradient::LINEAR;
    Gradient::Type mCurrentFillGradientType = Gradient::LINEAR;

    qptr<Gradient> mCurrentStrokeGradient;
    qptr<Gradient> mCurrentFillGradient;
    Qt::PenCapStyle mCurrentCapStyle;
    Qt::PenJoinStyle mCurrentJoinStyle;
    qreal mCurrentStrokeWidth;
    _SimpleBrushWrapper* mCurrentStrokeBrush = nullptr;
    qCubicSegment1D mCurrentStrokeBrushWidthCurve;
    qCubicSegment1D mCurrentStrokeBrushTimeCurve;
    qCubicSegment1D mCurrentStrokeBrushPressureCurve;

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

    QDockWidget* mBrushSelectionDock;
    BrushSelectionWidget* mBrushSelectionWidget;

    QDockWidget* mBrushCurvesDock;
    Segment1DEditor* mBrushWidthCurveEditor;
    Segment1DEditor* mBrushPressureCurveEditor;
    Segment1DEditor* mBrushTimeCurveEditor;
    QWidget* mBrushSettingsWidget;
};

#endif // FILLSTROKESETTINGS_H
