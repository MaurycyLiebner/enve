// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "colorsettingswidget.h"
#include "GUI/mainwindow.h"
#include <QResizeEvent>
#include <QMenu>
#include "GUI/ColorWidgets/colorpickingwidget.h"
#include "colorhelpers.h"
#include "colorlabel.h"
#include "GUI/global.h"
#include "GUI/actionbutton.h"
#include "GUI/ColorWidgets/savedcolorswidget.h"

void moveAndResizeValueRect(const int rect_x_t,
                            int *rect_y_t,
                            const int rect_width,
                            const int rect_height,
                            ColorValueRect *rect_t) {
    if(rect_t->isHidden()) {
        return;
    }
    rect_t->move(rect_x_t, *rect_y_t);
    *rect_y_t += rect_height;
    rect_t->resize(rect_width, rect_height);
}

void ColorSettingsWidget::setCurrentColor(const qreal h_t,
                                          const qreal s_t,
                                          const qreal v_t,
                                          const qreal a_t) {
    GLfloat hueGl = static_cast<GLfloat>(h_t);
    GLfloat satGl = static_cast<GLfloat>(s_t);
    GLfloat valGl = static_cast<GLfloat>(v_t);
    //GLfloat alphaGl = static_cast<GLfloat>(a_t);
    //wheel_triangle_widget->setColorHSV_f(h_t, s_t, v_t);
    r_rect->setColorHSV_f(hueGl, satGl, valGl);
    g_rect->setColorHSV_f(hueGl, satGl, valGl);
    b_rect->setColorHSV_f(hueGl, satGl, valGl);

    h_rect->setColorHSV_f(hueGl, satGl, valGl);
    h_rect->setDisplayedValue(h_t);
    hsv_s_rect->setColorHSV_f(hueGl, satGl, valGl);
    h_rect->setDisplayedValue(s_t);
    v_rect->setColorHSV_f(hueGl, satGl, valGl);
    h_rect->setDisplayedValue(v_t);

    hsl_s_rect->setColorHSV_f(hueGl, satGl, valGl);
    l_rect->setColorHSV_f(hueGl, satGl, valGl);

    mColorLabel->setColorHSV_f(hueGl, satGl, valGl);

    qreal hue = h_t;
    qreal hsvSat = s_t;
    qreal val = v_t;

    qreal red = hue;
    qreal green = hsvSat;
    qreal blue = val;
    qhsv_to_rgb(red, green, blue);

    r_rect->setDisplayedValue(red);
    g_rect->setDisplayedValue(green);
    b_rect->setDisplayedValue(blue);

    rSpin->setDisplayedValue(red);
    gSpin->setDisplayedValue(green);
    bSpin->setDisplayedValue(blue);

    hSpin->setDisplayedValue(hue);
    hsvSSpin->setDisplayedValue(hsvSat);
    vSpin->setDisplayedValue(val);

    qreal hslSat = hsvSat;
    qreal lig = val;
    qhsv_to_hsl(hue, hslSat, lig);

    hsl_s_rect->setDisplayedValue(hslSat);
    l_rect->setDisplayedValue(lig);

    hslSSpin->setDisplayedValue(hslSat);
    lSpin->setDisplayedValue(lig);

    mBookmarkedColors->setColor(QColor::fromHsvF(h_t, s_t, v_t, a_t));

    if(mAlphaHidden) return;
    mColorLabel->setAlpha(a_t);
    aRect->setColorHSV_f(hueGl, satGl, valGl);
    aRect->setDisplayedValue(a_t);
    aSpin->setDisplayedValue(a_t);
}

void ColorSettingsWidget::setCurrentColor(const QColor &color) {
    setCurrentColor(color.hueF(), color.saturationF(),
                    color.valueF(), color.alphaF());
}

void ColorSettingsWidget::updateWidgetTargets() {
    mUpdateConnections.clear();
    if(mTarget) {
        auto& conn = mUpdateConnections;

        const auto anim1 = mTarget->getVal1Animator();
        const auto anim2 = mTarget->getVal2Animator();
        const auto anim3 = mTarget->getVal3Animator();

        const auto colorMode = mTarget->getColorMode();
        mColorModeCombo->setCurrentIndex(static_cast<int>(colorMode));
        if(!mAlphaHidden) aSpin->setTarget(mTarget->getAlphaAnimator());
        if(mTarget->getColorMode() == ColorMode::rgb) {
            rSpin->setTarget(anim1);
            gSpin->setTarget(anim2);
            bSpin->setTarget(anim3);
            updateValuesFromRGBSpins();
            conn << connect(mTarget, &ColorAnimator::colorChanged,
                            this, &ColorSettingsWidget::updateValuesFromRGBSpins);
        } else if(mTarget->getColorMode() == ColorMode::hsv) {
            hSpin->setTarget(anim1);
            hsvSSpin->setTarget(anim2);
            vSpin->setTarget(anim3);
            updateValuesFromHSVSpins();
            conn << connect(mTarget, &ColorAnimator::colorChanged,
                            this, &ColorSettingsWidget::updateValuesFromHSVSpins);
        } else if(mTarget->getColorMode() == ColorMode::hsl) {
            hSpin->setTarget(anim1);
            hslSSpin->setTarget(anim2);
            lSpin->setTarget(anim3);
            updateValuesFromHSLSpins();
            conn << connect(mTarget, &ColorAnimator::colorChanged,
                            this, &ColorSettingsWidget::updateValuesFromHSLSpins);
        }

        updateAlphaFromSpin();
    } else {
        rSpin->setTarget(nullptr);
        gSpin->setTarget(nullptr);
        bSpin->setTarget(nullptr);

        hSpin->setTarget(nullptr);
        hsvSSpin->setTarget(nullptr);
        vSpin->setTarget(nullptr);

        hslSSpin->setTarget(nullptr);
        lSpin->setTarget(nullptr);

        aSpin->setTarget(nullptr);
    }
}

void ColorSettingsWidget::setTarget(ColorAnimator * const target) {
    auto& conn = mTarget.assign(target);

    updateWidgetTargets();
    if(target) {
        conn << connect(target, &ColorAnimator::colorModeChanged,
                        this, &ColorSettingsWidget::updateWidgetTargets);
        conn << connect(target, &ColorAnimator::colorChanged,
                        mBookmarkedColors, &SavedColorsWidget::setColor);
        mBookmarkedColors->setColor(getCurrentQColor());
    }
}

ColorSetting ColorSettingsWidget::getColorSetting(
        const ColorSettingType type,
        const ColorParameter parameter) const {
    const int tabId = mTabWidget->currentIndex();
    qreal alphaVal = 1;
    if(!mAlphaHidden) alphaVal = aSpin->value();
    if(tabId == 0) {
        return ColorSetting(
                    ColorMode::rgb, parameter,
                    rSpin->value(),
                    gSpin->value(),
                    bSpin->value(),
                    alphaVal,
                    type);
    } else if(tabId == 1) {
        return ColorSetting(
                    ColorMode::hsv, parameter,
                    hSpin->value(),
                    hsvSSpin->value(),
                    vSpin->value(),
                    alphaVal,
                    type);
    } else { //if(tabId == 2) {
        return ColorSetting(
                    ColorMode::hsl, parameter,
                    hSpin->value(),
                    hslSSpin->value(),
                    lSpin->value(),
                    alphaVal,
                    type);
    }
}

void ColorSettingsWidget::emitColorChangedSignal() {
    const auto colorSetting = getColorSetting(ColorSettingType::change,
                                              mLastTriggered);
    mBookmarkedColors->setColor(colorSetting.getColor());
    emit colorSettingSignal(colorSetting);
}

void ColorSettingsWidget::emitEditingFinishedSignal() {
    const auto colorSetting = getColorSetting(ColorSettingType::finish,
                                              mLastTriggered);
    emit colorSettingSignal(colorSetting);
}

void ColorSettingsWidget::emitEditingStartedSignal() {
    const auto colorSetting = getColorSetting(ColorSettingType::start,
                                              mLastTriggered);
    emit colorSettingSignal(colorSetting);
}

void ColorSettingsWidget::emitEditingStartedRed() {
    mLastTriggered = ColorParameter::red;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedGreen() {
    mLastTriggered = ColorParameter::green;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedBlue() {
    mLastTriggered = ColorParameter::blue;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedHue() {
    mLastTriggered = ColorParameter::hue;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedHSVSaturation() {
    mLastTriggered = ColorParameter::hsvSaturaton;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedValue() {
    mLastTriggered = ColorParameter::value;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedHSLSaturation() {
    mLastTriggered = ColorParameter::hslSaturation;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedLightness() {
    mLastTriggered = ColorParameter::lightness;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedAlpha() {
    mLastTriggered = ColorParameter::alpha;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitStartFullColorChangedSignal() {
    mLastTriggered = ColorParameter::all;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitFinishFullColorChangedSignal() {
    emitColorChangedSignal();
    emitEditingFinishedSignal();
}

void ColorSettingsWidget::moveAlphaWidgetToTab(const int tabId) {
    if(tabId == 1) {
        QVBoxLayout *hueParentLay = ((QVBoxLayout*)hLayout->parent());
        if(hueParentLay != mHSVLayout) {
            hueParentLay->removeItem(hLayout);
            mHSVLayout->insertLayout(0, hLayout);
        }
    } else if(tabId == 2) {
        QVBoxLayout *hueParentLay = ((QVBoxLayout*)hLayout->parent());
        if(hueParentLay != mHSLLayout) {
            hueParentLay->removeItem(hLayout);
            mHSLLayout->insertLayout(0, hLayout);
        }
    }/* else if(tabId == 3) {
        mWheelLayout->addLayout(aLayout);
    }*/
    if(!mAlphaHidden) {
        ((QVBoxLayout*)aLayout->parent())->removeItem(aLayout);
        if(tabId == 0) {
            mRGBLayout->addLayout(aLayout);
        } else if(tabId == 1) {
            mHSVLayout->addLayout(aLayout);
            QVBoxLayout *hueParentLay = ((QVBoxLayout*)hLayout->parent());
            if(hueParentLay != mHSVLayout) {
                hueParentLay->removeItem(hLayout);
                mHSVLayout->insertLayout(0, hLayout);
            }
        } else if(tabId == 2) {
            mHSLLayout->addLayout(aLayout);
            QVBoxLayout *hueParentLay = ((QVBoxLayout*)hLayout->parent());
            if(hueParentLay != mHSLLayout) {
                hueParentLay->removeItem(hLayout);
                mHSLLayout->insertLayout(0, hLayout);
            }
        }/* else if(tabId == 3) {
            mWheelLayout->addLayout(aLayout);
        }*/
    }
    for(int i=0;i < mTabWidget->count();i++)
        if(i!=tabId)
            mTabWidget->widget(i)->setSizePolicy(QSizePolicy::Minimum,
                                                 QSizePolicy::Ignored);

    mTabWidget->widget(tabId)->setSizePolicy(QSizePolicy::Minimum,
                                             QSizePolicy::Preferred);
    mTabWidget->widget(tabId)->resize(
                mTabWidget->widget(tabId)->minimumSizeHint());
}

void ColorSettingsWidget::startColorPicking() {
    const auto wid = new ColorPickingWidget(MainWindow::sGetInstance());
    connect(wid, &ColorPickingWidget::colorSelected,
            [this](const QColor & color) {
        emitStartFullColorChangedSignal();
        setCurrentColor(color);
        emitFinishFullColorChangedSignal();
    });
}

ColorSettingsWidget::ColorSettingsWidget(QWidget *parent) : QWidget(parent) {
    mColorModeCombo = new QComboBox(this);
    mWidgetsLayout->setAlignment(Qt::AlignTop);
    setLayout(mWidgetsLayout);

    mColorLabel = new ColorLabel(this);

//    mWheelWidget->setLayout(mWheelLayout);
//    mWheelLayout->setAlignment(Qt::AlignTop);
//    wheel_triangle_widget = new H_Wheel_SV_Triangle(this);
//    mWheelLayout->addWidget(wheel_triangle_widget, Qt::AlignHCenter);
//    mWheelLayout->setAlignment(wheel_triangle_widget, Qt::AlignHCenter);

    int LABEL_WIDTH = MIN_WIDGET_DIM;

    r_rect = new ColorValueRect(RED_PROGRAM, this);
    rLabel->setFixedWidth(LABEL_WIDTH);
    rLayout->addWidget(rLabel);
    rLayout->addWidget(r_rect);
    rLayout->addWidget(rSpin);
    g_rect = new ColorValueRect(GREEN_PROGRAM, this);
    gLabel->setFixedWidth(LABEL_WIDTH);
    gLayout->addWidget(gLabel);
    gLayout->addWidget(g_rect);
    gLayout->addWidget(gSpin);
    b_rect = new ColorValueRect(BLUE_PROGRAM, this);
    bLabel->setFixedWidth(LABEL_WIDTH);
    bLayout->addWidget(bLabel);
    bLayout->addWidget(b_rect);
    bLayout->addWidget(bSpin);
    mRGBLayout->setAlignment(Qt::AlignTop);
    mRGBLayout->addLayout(rLayout);
    mRGBLayout->addLayout(gLayout);
    mRGBLayout->addLayout(bLayout);
    mRGBWidget->setLayout(mRGBLayout);

    h_rect = new ColorValueRect(HUE_PROGRAM, this);
    hLabel->setFixedWidth(LABEL_WIDTH);
    hLayout->addWidget(hLabel);
    hLayout->addWidget(h_rect);
    hLayout->addWidget(hSpin);
    hsv_s_rect = new ColorValueRect(HSV_SATURATION_PROGRAM, this);
    hsvSLabel->setFixedWidth(LABEL_WIDTH);
    hsvSLayout->addWidget(hsvSLabel);
    hsvSLayout->addWidget(hsv_s_rect);
    hsvSLayout->addWidget(hsvSSpin);
    v_rect = new ColorValueRect(VALUE_PROGRAM, this);
    vLabel->setFixedWidth(LABEL_WIDTH);
    vLayout->addWidget(vLabel);
    vLayout->addWidget(v_rect);
    vLayout->addWidget(vSpin);
    mHSVLayout->setAlignment(Qt::AlignTop);
    mHSVLayout->addLayout(hLayout);
    mHSVLayout->addLayout(hsvSLayout);
    mHSVLayout->addLayout(vLayout);
    mHSVWidget->setLayout(mHSVLayout);

    hsl_s_rect = new ColorValueRect(HSL_SATURATION_PROGRAM, this);
    hslSLabel->setFixedWidth(LABEL_WIDTH);
    hslSLayout->addWidget(hslSLabel);
    hslSLayout->addWidget(hsl_s_rect);
    hslSLayout->addWidget(hslSSpin);
    l_rect = new ColorValueRect(LIGHTNESS_PROGRAM, this);
    lLabel->setFixedWidth(LABEL_WIDTH);
    lLayout->addWidget(lLabel);
    lLayout->addWidget(l_rect);
    lLayout->addWidget(lSpin);
    mHSLLayout->setAlignment(Qt::AlignTop);
    mHSLLayout->addLayout(hslSLayout);
    mHSLLayout->addLayout(lLayout);
    mHSLWidget->setLayout(mHSLLayout);

    aRect = new ColorValueRect(ALPHA_PROGRAM, this);
    aLabel->setFixedWidth(LABEL_WIDTH);
    aLayout->addWidget(aLabel);
    aLayout->addWidget(aRect);
    aLayout->addWidget(aSpin);

    const QString iconsDir = eSettings::sIconsDir() + "/toolbarButtons";
    mPickingButton = new ActionButton(iconsDir + "/pickUnchecked.png", "", this);
    connect(mPickingButton, &ActionButton::released,
            this, &ColorSettingsWidget::startColorPicking);
    mColorLabelLayout->addWidget(mColorLabel);
    mColorLabelLayout->addWidget(mPickingButton);
    mWidgetsLayout->addLayout(mColorLabelLayout);

    mTabWidget->addTab(mRGBWidget, "RGB");
    mTabWidget->addTab(mHSVWidget, "HSV");
    mTabWidget->addTab(mHSLWidget, "HSL");
    //mTabWidget->addTab(mWheelWidget, "Wheel");
    mWidgetsLayout->addWidget(mTabWidget);
    mRGBLayout->addLayout(aLayout);

    mColorModeLayout->addWidget(mColorModeLabel);
    mColorModeLayout->addWidget(mColorModeCombo);
    mColorModeCombo->addItem("RGB");
    mColorModeCombo->addItem("HSV");
    mColorModeCombo->addItem("HSL");
    connect(mColorModeCombo, qOverload<int>(&QComboBox::activated),
            this, &ColorSettingsWidget::setColorMode);

    mWidgetsLayout->addLayout(mColorModeLayout);

    mBookmarkedColors = new SavedColorsWidget(this);
    mWidgetsLayout->addWidget(mBookmarkedColors);
    connect(mBookmarkedColors, &SavedColorsWidget::colorSet,
            this, [this](const QColor& color) {
        emitStartFullColorChangedSignal();
        setCurrentColor(color);
        emitFinishFullColorChangedSignal();
        Document::sInstance->actionFinished();
    });

    connect(mTabWidget, &QTabWidget::currentChanged,
            this, &ColorSettingsWidget::moveAlphaWidgetToTab);

    connect(rSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setRed);
    connect(gSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setGreen);
    connect(bSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setBlue);

    connect(hSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setHSVHue);
    connect(hsvSSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setHSVSaturation);
    connect(vSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setHSVValue);

    connect(hslSSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setHSLSaturation);
    connect(lSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setHSLLightness);

    connect(aSpin, &QrealAnimatorValueSlider::valueEdited,
            this, &ColorSettingsWidget::setAlpha);

    connect(rSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedRed);
    connect(gSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedGreen);
    connect(bSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedBlue);

    connect(hSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedHue);
    connect(hsvSSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedHSVSaturation);
    connect(vSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedValue);

    connect(hslSSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedHSLSaturation);
    connect(lSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedLightness);

    connect(aSpin, &QrealAnimatorValueSlider::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedAlpha);

    connect(rSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(gSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(bSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(hSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(hsvSSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(vSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(hslSSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(lSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(aSpin, &QrealAnimatorValueSlider::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(r_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setRed);
    connect(g_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setGreen);
    connect(b_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setBlue);

    connect(h_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setHSVHue);
    connect(hsv_s_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setHSVSaturation);
    connect(v_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setHSVValue);

    connect(hsl_s_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setHSLSaturation);
    connect(l_rect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setHSLLightness);

    connect(aRect, &ColorValueRect::valueChanged,
            this, &ColorSettingsWidget::setAlpha);

    connect(r_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedRed);
    connect(g_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedGreen);
    connect(b_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedBlue);

    connect(h_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedHue);
    connect(hsv_s_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedHSVSaturation);
    connect(v_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedValue);

    connect(hsl_s_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedHSLSaturation);
    connect(l_rect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedLightness);

    connect(aRect, &ColorValueRect::editingStarted,
            this, &ColorSettingsWidget::emitEditingStartedAlpha);

    connect(r_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(g_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(b_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(h_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(hsv_s_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(v_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(hsl_s_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);
    connect(l_rect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(aRect, &ColorValueRect::editingFinished,
            this, &ColorSettingsWidget::emitEditingFinishedSignal);

    connect(this, &ColorSettingsWidget::colorSettingSignal,
            this, [this](const ColorSetting& setting) {
        if(mTarget) setting.apply(mTarget);
    });


    //setMinimumSize(250, 200);
    mTabWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Maximum);
    setCurrentColor(0, 0, 0);

    moveAlphaWidgetToTab(0);
}

QColor ColorSettingsWidget::getCurrentQColor() {
    const qreal red = rSpin->value();
    const qreal green = gSpin->value();
    const qreal blue = bSpin->value();
    const qreal alpha = aSpin->value();
    QColor col;
    col.setRgbF(red, green, blue, alpha);
    return col;
}

void addColorWidgetActionToMenu(QMenu *menu_t,
                                QString label_t,
                                ColorWidget *widget_t) {
    QAction *action_t = menu_t->addAction(label_t);
    action_t->setCheckable(true);
    action_t->setChecked(widget_t->isVisible() );
}

void ColorSettingsWidget::updateValuesFromRGBSpins() {
    setAllDisplayedFromRGB(rSpin->value(),
                           gSpin->value(),
                           bSpin->value());
}

void ColorSettingsWidget::updateValuesFromHSVSpins() {
    setAllDisplayedFromHSV(hSpin->value(),
                           hsvSSpin->value(),
                           vSpin->value());
}

void ColorSettingsWidget::updateValuesFromHSLSpins() {
    setAllDisplayedFromHSL(hSpin->value(),
                           hslSSpin->value(),
                           lSpin->value());
}

void ColorSettingsWidget::setDisplayedRGB(const qreal red,
                                          const qreal green,
                                          const qreal blue) {
    rSpin->setDisplayedValue(red);
    gSpin->setDisplayedValue(green);
    bSpin->setDisplayedValue(blue);

    r_rect->setDisplayedValue(red);
    g_rect->setDisplayedValue(green);
    b_rect->setDisplayedValue(blue);
}

void ColorSettingsWidget::setDisplayedHSV(const qreal hue,
                                          const qreal saturation,
                                          const qreal value) {
    hSpin->setDisplayedValue(hue);
    hsvSSpin->setDisplayedValue(saturation);
    vSpin->setDisplayedValue(value);

    h_rect->setDisplayedValue(hue);
    hsv_s_rect->setDisplayedValue(saturation);
    v_rect->setDisplayedValue(value);

    r_rect->setColorHSV_f(hue, saturation, value);
    g_rect->setColorHSV_f(hue, saturation, value);
    b_rect->setColorHSV_f(hue, saturation, value);

    h_rect->setColorHSV_f(hue, saturation, value);
    hsv_s_rect->setColorHSV_f(hue, saturation, value);
    v_rect->setColorHSV_f(hue, saturation, value);

    hsl_s_rect->setColorHSV_f(hue, saturation, value);
    l_rect->setColorHSV_f(hue, saturation, value);

    if(!mAlphaHidden)
        aRect->setColorHSV_f(hue, saturation, value);

    mColorLabel->setColorHSV_f(hue, saturation, value);
}

void ColorSettingsWidget::setDisplayedHSL(const qreal hue,
                                          const qreal saturation,
                                          const qreal lightness) {
    hSpin->setDisplayedValue(hue);
    hslSSpin->setDisplayedValue(saturation);
    lSpin->setDisplayedValue(lightness);

    h_rect->setDisplayedValue(hue);
    hsl_s_rect->setDisplayedValue(saturation);
    l_rect->setDisplayedValue(lightness);
}

void ColorSettingsWidget::setAllDisplayedFromRGB(const qreal red,
                                                 const qreal green,
                                                 const qreal blue) {
    setDisplayedRGB(red, green, blue);

    qreal hue = red;
    qreal hsvSaturation = green;
    qreal value = blue;
    qrgb_to_hsv(hue, hsvSaturation, value);
    setDisplayedHSV(hue, hsvSaturation, value);

    hue = red;
    qreal hslSaturation = green;
    qreal lightness = blue;
    qrgb_to_hsl(hue, hslSaturation, lightness);
    setDisplayedHSL(hue, hslSaturation, lightness);
}

void ColorSettingsWidget::setAllDisplayedFromHSV(const qreal hue,
                                                 const qreal saturation,
                                                 const qreal value) {
    setDisplayedHSV(hue, saturation, value);

    qreal red = hue;
    qreal green = saturation;
    qreal blue = value;
    qhsv_to_rgb(red, green, blue);
    setDisplayedRGB(red, green, blue);

    qreal hslhue = hue;
    qreal hslSaturation = saturation;
    qreal lightness = value;
    qhsv_to_hsl(hslhue, hslSaturation, lightness);
    setDisplayedHSL(hslhue, hslSaturation, lightness);
}

void ColorSettingsWidget::setAllDisplayedFromHSL(const qreal hue,
                                                 const qreal saturation,
                                                 const qreal lightness) {
    setDisplayedHSL(hue, saturation, lightness);

    qreal red = hue;
    qreal green = saturation;
    qreal blue = lightness;
    qhsl_to_rgb(red, green, blue);
    setDisplayedRGB(red, green, blue);

    qreal hsvhue = hue;
    qreal hsvSaturation = saturation;
    qreal value = lightness;
    qhsl_to_hsv(hsvhue, hsvSaturation, value);
    setDisplayedHSV(hue, hsvSaturation, value);
}

void ColorSettingsWidget::setRGB(const qreal red,
                                 const qreal green,
                                 const qreal blue) {
    setAllDisplayedFromRGB(red, green, blue);
    emitColorChangedSignal();
}

void ColorSettingsWidget::setHSV(const qreal hue,
                                 const qreal saturation,
                                 const qreal value) {
    setAllDisplayedFromHSV(hue, saturation, value);
    emitColorChangedSignal();
}

void ColorSettingsWidget::setHSL(const qreal hue,
                                 const qreal saturation,
                                 const qreal lightness) {
    setAllDisplayedFromHSL(hue, saturation, lightness);
    emitColorChangedSignal();
}

void ColorSettingsWidget::setRed(const qreal red) {
    setRGB(red, gSpin->value(), bSpin->value());
}

void ColorSettingsWidget::setGreen(const qreal green) {
    setRGB(rSpin->value(), green, bSpin->value());
}

void ColorSettingsWidget::setBlue(const qreal blue) {
    setRGB(rSpin->value(), gSpin->value(), blue);
}

void ColorSettingsWidget::setHSVHue(const qreal hue) {
    setHSV(hue, hsvSSpin->value(), vSpin->value());
}

void ColorSettingsWidget::setHSVSaturation(const qreal saturation) {
    setHSV(hSpin->value(), saturation, vSpin->value());
}

void ColorSettingsWidget::setHSVValue(const qreal value) {
    setHSV(hSpin->value(), hsvSSpin->value(), value);
}

void ColorSettingsWidget::setHSLHue(const qreal hue) {
    setHSL(hue, hslSSpin->value(), lSpin->value());
}

void ColorSettingsWidget::setHSLSaturation(const qreal saturation) {
    setHSL(hSpin->value(), saturation, lSpin->value());
}

void ColorSettingsWidget::setHSLLightness(const qreal value) {
    setHSL(hSpin->value(), hslSSpin->value(), value);
}

void ColorSettingsWidget::updateAlphaFromSpin() {
    if(mAlphaHidden) return;
    mColorLabel->setAlpha(aSpin->value());
    aRect->setDisplayedValue(aSpin->value());
}

void ColorSettingsWidget::setColorMode() {
    const auto colorSetting = getColorSetting(ColorSettingType::apply,
                                              ColorParameter::colorMode);
    emit colorSettingSignal(colorSetting);
    Document::sInstance->actionFinished();
}

void ColorSettingsWidget::setAlpha(const qreal val) {
    if(mAlphaHidden) return;
    aSpin->setDisplayedValue(val);
    mColorLabel->setAlpha(val);
    aRect->setDisplayedValue(val);
    emitColorChangedSignal();
}
