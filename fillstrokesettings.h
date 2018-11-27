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

class GradientWidget;
class MainWindow;
class CanvasWindow;
class ColorSettingsWidget;
class QrealAnimatorValueSlider;
class ActionButton;

class FillStrokeSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(MainWindow *parent = 0);

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
public slots:
    void emitStrokeWidthChanged();
    void emitStrokeWidthChangedTMP();
    void emitCapStyleChanged();
    void emitJoinStyleChanged();
private slots:
    void setLinearGradientFill();
    void setRadialGradientFill();

    void setStrokeWidth(const qreal &width);

    void colorTypeSet(const int &id);
    void setFillTarget();
    void setStrokeTarget();

    void setGradient(Gradient* gradient);
    void setGradientLinear(const bool &linear);

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


    int mTargetId = 0;

    //

    PaintType getCurrentPaintTypeVal();

    void setCurrentPaintTypeVal(const PaintType& paintType);

    QColor getCurrentColorVal();

    void setCurrentColorVal(const QColor& color);

    Gradient *getCurrentGradientVal();

    const bool &getCurrentGradientLinearVal() {
        if(mTargetId == 0) {
            return mCurrentFillGradientLinear;
        } else {
            return mCurrentStrokeGradientLinear;
        }
    }

    void setCurrentGradientVal(Gradient *gradient);
    void setCurrentGradientLinearVal(const bool &linear);

    ColorAnimatorQPtr mCurrentFillColorAnimator;
    ColorAnimatorQPtr mCurrentStrokeColorAnimator;
    PaintType mCurrentFillPaintType = NOPAINT;
    PaintType mCurrentStrokePaintType = NOPAINT;
    QColor mCurrentFillColor;
    QColor mCurrentStrokeColor;

    bool mCurrentStrokeGradientLinear = true;
    bool mCurrentFillGradientLinear = true;

    GradientQPtr mCurrentStrokeGradient;
    GradientQPtr mCurrentFillGradient;
    Qt::PenCapStyle mCurrentCapStyle;
    Qt::PenJoinStyle mCurrentJoinStyle;
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
};

#endif // FILLSTROKESETTINGS_H
