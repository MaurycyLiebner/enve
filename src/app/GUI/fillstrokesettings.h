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

class FillStrokeSettingsWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(Document& document,
                                      QWidget * const parent = nullptr);

    void setCurrentSettings(PaintSettingsAnimator *fillPaintSettings,
                            OutlineSettingsAnimator *strokePaintSettings);
    void updateAfterTargetChanged();
    void setCurrentPaintType(const PaintType paintType);

    GradientWidget *getGradientWidget();

    void clearAll();
    void setFillValuesFromFillSettings(PaintSettingsAnimator *settings);
    void setStrokeValuesFromStrokeSettings(OutlineSettingsAnimator *settings);

    void updateColorAnimator();

    void emitCapStyleChanged();
    void emitJoinStyleChanged();
private:
    void setLinearGradientAction();
    void setRadialGradientAction();

    void colorTypeSet(const PaintType type);
    void setFillTarget();
    void setStrokeTarget();

    void setGradientAction(Gradient* gradient);
    void setGradientType(const GradientType type);

    void setBevelJoinStyleAction();
    void setMiterJoinStyleAction();
    void setRoundJoinStyleAction();

    void setFlatCapStyleAction();
    void setSquareCapStyleAction();
    void setRoundCapStyleAction();

    void setGradientFillAction();
    void setFlatFillAction();
    void setNoneFillAction();

    void setColorAnimatorTarget(ColorAnimator *animator);
    void colorSettingReceived(const ColorSetting &colorSetting);
private:
    void updateCurrentSettings();
    void applyGradient();


    void connectGradient();
    void disconnectGradient();

    void setJoinStyle(const SkPaint::Join joinStyle);
    void setCapStyle(const SkPaint::Cap capStyle);

    //

    PaintType getCurrentPaintTypeVal();

    void setCurrentPaintTypeVal(const PaintType paintType);

    QColor getColorVal();

    void setCurrentColorVal(const QColor& color);

    Gradient *getCurrentGradientVal();

    GradientType getCurrentGradientTypeVal() const {
        if(mTarget == PaintSetting::FILL) return mCurrentFillGradientType;
        else return mCurrentStrokeGradientType;
    }

    void setCurrentGradientVal(Gradient *gradient);
    void setCurrentGradientTypeVal(const GradientType type);

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
    SkPaint::Cap mCurrentCapStyle;
    SkPaint::Join mCurrentJoinStyle;
    qreal mCurrentStrokeWidth;

    //

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
};

#endif // FILLSTROKESETTINGS_H
