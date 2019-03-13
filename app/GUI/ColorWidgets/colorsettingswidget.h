#ifndef COLORSETTINGSWIDGET_H
#define COLORSETTINGSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
//#include "h_wheel_sv_triangle.h"
#include "colorvaluerect.h"
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "GUI/qrealanimatorvalueslider.h"
#include "Animators/coloranimator.h"
#include "Animators/paintsettings.h"
#include "paintsettingsapplier.h"

class ColorLabel;
class ActionButton;

class ColorSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColorSettingsWidget(QWidget *parent = nullptr);

    QColor getCurrentQColor();
    void setCurrentColor(const QColor &color);
    void hideAlphaControlers();
    ColorSetting getCurrentColorSetting(const ColorSettingType &type) const;
signals:
    void colorSettingSignal(const ColorSetting&);
    void colorModeChanged(const ColorMode&);
public slots:
    void setCurrentColor(const qreal &h_t,
                         const qreal &s_t,
                         const qreal &v_t,
                         const qreal &a_t = 1.);
    void setColorAnimatorTarget(ColorAnimator *target);
    void emitColorChangedSignal();
    void emitEditingFinishedSignal();
    void emitEditingStartedSignal();

    void emitEditingStartedRed();
    void emitEditingStartedGreen();
    void emitEditingStartedBlue();

    void emitEditingStartedHue();
    void emitEditingStartedHSVSaturation();
    void emitEditingStartedValue();

    void emitEditingStartedHSLSaturation();
    void emitEditingStartedLightness();

    void emitEditingStartedAlpha();

    void emitFullColorChangedSignal();
private slots:
    void moveAlphaWidgetToTab(const int &tabId);

    void startColorPicking();

    void setAlphaFromSpin(const qreal &val);

    void updateValuesFromRGB();
    void updateValuesFromHSV();
    void updateValuesFromHSL();
    void setValuesFromRGB();
    void setValuesFromHSV();
    void setValuesFromHSL();
    void updateAlphaFromSpin();

    void setColorMode(const int &colorMode);
    void refreshColorAnimatorTarget();
    void nullifyAnimator();
private:
    void connectSignalsAndSlots();
    void connectColorWidgetSignalToSlot(ColorWidget *slot_obj,
                                        const char *slot,
                                        ColorWidget *signal_src,
                                        const char *signal);
    void connectColorWidgetSignalToSlots(ColorWidget *signal_src,
                                         const char *signal,
                                         const char *slot );
    void connectColorWidgetSignalToSlotsWithoutThis(ColorWidget *signal_src,
                                                    const char *signal,
                                                    const char *slot);
    void setRectValuesAndColor(
        const qreal &red,
        const qreal &green,
        const qreal &blue,
        const qreal &hue,
        const qreal &hsvSaturation,
        const qreal &value,
        const qreal &hslSaturation,
        const qreal &lightness);

    bool mBlockColorSettings = false;
    CVR_TYPE mLastTriggeredCVR;

    ColorAnimator *mTargetAnimator = nullptr;

    QHBoxLayout *mColorModeLayout = new QHBoxLayout();
    QLabel *mColorModeLabel = new QLabel("Color model:", this);
    QComboBox *mColorModeCombo;

    QTabWidget *mTabWidget = new QTabWidget();
    QVBoxLayout *mWidgetsLayout = new QVBoxLayout();

//    QWidget *mWheelWidget = new QWidget();
//    QVBoxLayout *mWheelLayout = new QVBoxLayout();
//    H_Wheel_SV_Triangle *wheel_triangle_widget = nullptr;

    QWidget *mRGBWidget = new QWidget();
    QVBoxLayout *mRGBLayout = new QVBoxLayout();
    QHBoxLayout *rLayout = new QHBoxLayout();
    QLabel *rLabel = new QLabel("R:");
    ColorValueRect *r_rect = nullptr;
    QrealAnimatorValueSlider *rSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);
    QHBoxLayout *gLayout = new QHBoxLayout();
    QLabel *gLabel = new QLabel("G:");
    ColorValueRect *g_rect = nullptr;
    QrealAnimatorValueSlider *gSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);
    QHBoxLayout *bLayout = new QHBoxLayout();
    QLabel *bLabel = new QLabel("B:");
    ColorValueRect *b_rect = nullptr;
    QrealAnimatorValueSlider *bSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);

    QWidget *mHSVWidget = new QWidget();
    QVBoxLayout *mHSVLayout = new QVBoxLayout();
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *hLabel = new QLabel("H:");
    ColorValueRect *h_rect = nullptr;
    QrealAnimatorValueSlider *hSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);
    QHBoxLayout *hsvSLayout = new QHBoxLayout();
    QLabel *hsvSLabel = new QLabel("S:");
    ColorValueRect *hsv_s_rect = nullptr;
    QrealAnimatorValueSlider *hsvSSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);
    QHBoxLayout *vLayout = new QHBoxLayout();
    QLabel *vLabel = new QLabel("V:");
    ColorValueRect *v_rect = nullptr;
    QrealAnimatorValueSlider *vSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);

    QWidget *mHSLWidget = new QWidget();
    QVBoxLayout *mHSLLayout = new QVBoxLayout();

    QHBoxLayout *hslSLayout = new QHBoxLayout();
    QLabel *hslSLabel = new QLabel("S:");
    ColorValueRect *hsl_s_rect = nullptr;
    QrealAnimatorValueSlider *hslSSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);
    QHBoxLayout *lLayout = new QHBoxLayout();
    QLabel *lLabel = new QLabel("L:");
    ColorValueRect *l_rect = nullptr;
    QrealAnimatorValueSlider *lSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);

    bool mAlphaHidden = false;
    QHBoxLayout *aLayout = new QHBoxLayout();
    QLabel *aLabel = new QLabel("A:");
    ColorValueRect *aRect = nullptr;
    QrealAnimatorValueSlider *aSpin =
            new QrealAnimatorValueSlider(0., 1., 0.1, this);

    QHBoxLayout *mColorLabelLayout = new QHBoxLayout();
    ActionButton *mPickingButton;

    ColorLabel *mColorLabel = nullptr;
};

#endif // COLORSETTINGSWIDGET_H
