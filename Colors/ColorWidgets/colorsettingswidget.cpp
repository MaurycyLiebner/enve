#include "colorsettingswidget.h"
#include "mainwindow.h"
#include <QResizeEvent>
#include <QMenu>
#include "Colors/colorpickingwidget.h"
#include "Colors/helpers.h"
#include "colorlabel.h"
#include "global.h"

void moveAndResizeValueRect(const int &rect_x_t,
                            int *rect_y_t,
                            const int &rect_width,
                            const int &rect_height,
                            ColorValueRect *rect_t) {
    if(rect_t->isHidden()) {
        return;
    }
    rect_t->move(rect_x_t, *rect_y_t);
    *rect_y_t += rect_height;
    rect_t->resize(rect_width, rect_height);
}

void ColorSettingsWidget::setCurrentColor(const GLfloat &h_t,
                                          const GLfloat &s_t,
                                          const GLfloat &v_t,
                                          const GLfloat &a_t) {

    //wheel_triangle_widget->setColorHSV_f(h_t, s_t, v_t);
//    r_rect->setColorHSV_f(h_t, s_t, v_t);
//    g_rect->setColorHSV_f(h_t, s_t, v_t);
//    b_rect->setColorHSV_f(h_t, s_t, v_t);

//    h_rect->setColorHSV_f(h_t, s_t, v_t);
//    hsv_s_rect->setColorHSV_f(h_t, s_t, v_t);
//    v_rect->setColorHSV_f(h_t, s_t, v_t);

//    hsl_h_rect->setColorHSV_f(h_t, s_t, v_t);
//    hsl_s_rect->setColorHSV_f(h_t, s_t, v_t);
//    l_rect->setColorHSV_f(h_t, s_t, v_t);

//    color_label->setColorHSV_f(h_t, s_t, v_t);
//    color_label->setAlpha(a_t);

//    a_rect->setColorHSV_f(h_t, s_t, v_t);
//    a_rect->setDisplayedValue(a_t);

    qreal hue = h_t;
    qreal hsvSat = s_t;
    qreal val = v_t;

    qreal red = hue;
    qreal green = hsvSat;
    qreal blue = val;
    qhsv_to_rgb(&red, &green, &blue);

    rSpin->setValueExternal(red);
    gSpin->setValueExternal(green);
    bSpin->setValueExternal(blue);

    hSpin->setValueExternal(hue);
    hsvSSpin->setValueExternal(hsvSat);
    vSpin->setValueExternal(val);

    qreal hslSat = hsvSat;
    qreal lig = val;
    qhsv_to_hsl(&hue, &hslSat, &lig);

    hslSSpin->setValueExternal(hslSat);
    lSpin->setValueExternal(lig);

    if(mAlphaHidden) return;
    aSpin->setValueExternal(a_t);
}

void ColorSettingsWidget::setCurrentColor(const Color &color) {
    setCurrentColor(color.gl_h, color.gl_s, color.gl_v, color.gl_a);
}

void ColorSettingsWidget::hideAlphaControlers() {
    delete aLabel;
    delete aRect;
    delete aSpin;
    delete aLayout;
    mAlphaHidden = true;
    mColorModeCombo->hide();
    mColorModeLabel->hide();
}

void ColorSettingsWidget::refreshColorAnimatorTarget() {
    setColorAnimatorTarget(mTargetAnimator);
}

void ColorSettingsWidget::nullifyAnimator() {
    setColorAnimatorTarget(NULL);
}

void ColorSettingsWidget::setColorAnimatorTarget(ColorAnimator *target) {
    if(mTargetAnimator != NULL) {
        disconnect(mTargetAnimator, 0, this, 0);
    }
    mTargetAnimator = target;
    rSpin->clearAnimator();
    gSpin->clearAnimator();
    bSpin->clearAnimator();
    hSpin->clearAnimator();
    hslSSpin->clearAnimator();
    lSpin->clearAnimator();
    hsvSSpin->clearAnimator();
    vSpin->clearAnimator();
    if(!mAlphaHidden) {
        aSpin->clearAnimator();
    }
    if(target != NULL) {
        disconnect(mColorModeCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setColorMode(int)));
        mColorModeCombo->setCurrentIndex(target->getColorMode());
        connect(mColorModeCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setColorMode(int)));
        if(!mAlphaHidden) {
            aSpin->setAnimator(target->getAlphaAnimator());
        }
        if(target->getColorMode() == RGBMODE) {
            rSpin->setAnimator(target->getVal1Animator());
            gSpin->setAnimator(target->getVal2Animator());
            bSpin->setAnimator(target->getVal3Animator());

            updateValuesFromRGB();
        } else if(target->getColorMode() == HSLMODE) {
            hSpin->setAnimator(target->getVal1Animator());
            hslSSpin->setAnimator(target->getVal2Animator());
            lSpin->setAnimator(target->getVal3Animator());

            updateValuesFromHSL();
        } else { // HSVMODE
            hSpin->setAnimator(target->getVal1Animator());
            hsvSSpin->setAnimator(target->getVal2Animator());
            vSpin->setAnimator(target->getVal3Animator());

            updateValuesFromHSV();
        }

        updateAlphaFromSpin();
        connect(target, SIGNAL(colorModeChanged(ColorMode)),
                this, SLOT(refreshColorAnimatorTarget()));
        connect(target, SIGNAL(beingDeleted()),
                this, SLOT(nullifyAnimator()));
    }
}

void ColorSettingsWidget::emitColorChangedSignal() {
    int tabId = mTabWidget->currentIndex();
    qreal alphaVal = 1.;
    if(!mAlphaHidden) {
        alphaVal = aSpin->value();
    }
    if(tabId == 0) {
        ColorSetting setting = ColorSetting(
                    RGBMODE, mLastTriggeredCVR,
                    rSpin->value(),
                    gSpin->value(),
                    bSpin->value(),
                    alphaVal,
                    CST_CHANGE, mTargetAnimator);
        emit colorSettingSignal(setting);
    } else if(tabId == 1) {
        ColorSetting setting = ColorSetting(
                    HSVMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hsvSSpin->value(),
                    vSpin->value(),
                    alphaVal,
                    CST_CHANGE, mTargetAnimator);
        emit colorSettingSignal(setting);
    } else if(tabId == 2) {
        ColorSetting setting = ColorSetting(
                    HSLMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hslSSpin->value(),
                    lSpin->value(),
                    alphaVal,
                    CST_CHANGE, mTargetAnimator);
        emit colorSettingSignal(setting);
    }
}

void ColorSettingsWidget::emitEditingFinishedSignal() {
    int tabId = mTabWidget->currentIndex();
    if(mTargetAnimator != NULL) {
        if(mTargetAnimator->getColorMode() != tabId) {
            mTargetAnimator->prp_finishTransform();
        }
    }
    if(tabId == 0) {
        ColorSetting setting = ColorSetting(
                    RGBMODE, mLastTriggeredCVR,
                    rSpin->value(),
                    gSpin->value(),
                    bSpin->value(),
                    aSpin->value(),
                    CST_FINISH, mTargetAnimator);
        emit colorSettingSignal(setting);
    } else if(tabId == 1) {
        ColorSetting setting = ColorSetting(
                    HSVMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hsvSSpin->value(),
                    vSpin->value(),
                    aSpin->value(),
                    CST_FINISH, mTargetAnimator);
        emit colorSettingSignal(setting);
    } else if(tabId == 2) {
        ColorSetting setting = ColorSetting(
                    HSLMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hslSSpin->value(),
                    lSpin->value(),
                    aSpin->value(),
                    CST_FINISH, mTargetAnimator);
        emit colorSettingSignal(setting);
    }
}

void ColorSettingsWidget::emitEditingStartedSignal() {
    int tabId = mTabWidget->currentIndex();
    if(mTargetAnimator != NULL) {
        if(mTargetAnimator->getColorMode() != tabId) {
            mTargetAnimator->startVal1Transform();
            mTargetAnimator->startVal2Transform();
            mTargetAnimator->startVal3Transform();
        }
    }
    if(tabId == 0) {
        ColorSetting setting = ColorSetting(
                    RGBMODE, mLastTriggeredCVR,
                    rSpin->value(),
                    gSpin->value(),
                    bSpin->value(),
                    aSpin->value(),
                    CST_START, mTargetAnimator);
        emit colorSettingSignal(setting);
    } else if(tabId == 1) {
        ColorSetting setting = ColorSetting(
                    HSVMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hsvSSpin->value(),
                    vSpin->value(),
                    aSpin->value(),
                    CST_START, mTargetAnimator);
        emit colorSettingSignal(setting);
    } else if(tabId == 2) {
        ColorSetting setting = ColorSetting(
                    HSLMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hslSSpin->value(),
                    lSpin->value(),
                    aSpin->value(),
                    CST_START, mTargetAnimator);
        emit colorSettingSignal(setting);
    }
}

void ColorSettingsWidget::emitEditingStartedRed() {
    mLastTriggeredCVR = CVR_RED;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedGreen() {
    mLastTriggeredCVR = CVR_GREEN;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedBlue() {
    mLastTriggeredCVR = CVR_BLUE;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedHue() {
    mLastTriggeredCVR = CVR_HUE;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedHSVSaturation() {
    mLastTriggeredCVR = CVR_HSVSATURATION;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedValue() {
    mLastTriggeredCVR = CVR_VALUE;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedHSLSaturation() {
    mLastTriggeredCVR = CVR_HSLSATURATION;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedLightness() {
    mLastTriggeredCVR = CVR_LIGHTNESS;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitEditingStartedAlpha() {
    mLastTriggeredCVR = CVR_ALPHA;
    emitEditingStartedSignal();
}

void ColorSettingsWidget::emitFullColorChangedSignal() {
    mLastTriggeredCVR = CVR_ALL;
    if(mTargetAnimator != NULL) {
        mTargetAnimator->prp_startTransform();
    }
    updateValuesFromHSV();
    updateAlphaFromSpin();
    if(mTargetAnimator != NULL) {
        mTargetAnimator->prp_finishTransform();
    }
    emitEditingStartedSignal();
    emitColorChangedSignal();
    emitEditingFinishedSignal();
}

void ColorSettingsWidget::moveAlphaWidgetToTab(const int &tabId) {
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

void ColorSettingsWidget::startColorPicking()
{
    new ColorPickingWidget(this);
}
#include "actionbutton.h"

ColorSettingsWidget::ColorSettingsWidget(QWidget *parent) : QWidget(parent) {
    mColorModeCombo = new QComboBox(this);
    mWidgetsLayout->setAlignment(Qt::AlignTop);
    setLayout(mWidgetsLayout);

    color_label = new ColorLabel(this);

//    mWheelWidget->setLayout(mWheelLayout);
//    mWheelLayout->setAlignment(Qt::AlignTop);
//    wheel_triangle_widget = new H_Wheel_SV_Triangle(this);
//    mWheelLayout->addWidget(wheel_triangle_widget, Qt::AlignHCenter);
//    mWheelLayout->setAlignment(wheel_triangle_widget, Qt::AlignHCenter);

    int LABEL_WIDTH = MIN_WIDGET_HEIGHT;

    r_rect = new ColorValueRect(CVR_RED, this);
    rLabel->setFixedWidth(LABEL_WIDTH);
    rLayout->addWidget(rLabel);
    rLayout->addWidget(r_rect);
    rLayout->addWidget(rSpin);
    g_rect = new ColorValueRect(CVR_GREEN, this);
    gLabel->setFixedWidth(LABEL_WIDTH);
    gLayout->addWidget(gLabel);
    gLayout->addWidget(g_rect);
    gLayout->addWidget(gSpin);
    b_rect = new ColorValueRect(CVR_BLUE, this);
    bLabel->setFixedWidth(LABEL_WIDTH);
    bLayout->addWidget(bLabel);
    bLayout->addWidget(b_rect);
    bLayout->addWidget(bSpin);
    mRGBLayout->setAlignment(Qt::AlignTop);
    mRGBLayout->addLayout(rLayout);
    mRGBLayout->addLayout(gLayout);
    mRGBLayout->addLayout(bLayout);
    mRGBWidget->setLayout(mRGBLayout);

    h_rect = new ColorValueRect(CVR_HUE, this);
    hLabel->setFixedWidth(LABEL_WIDTH);
    hLayout->addWidget(hLabel);
    hLayout->addWidget(h_rect);
    hLayout->addWidget(hSpin);
    hsv_s_rect = new ColorValueRect(CVR_HSVSATURATION, this);
    hsvSLabel->setFixedWidth(LABEL_WIDTH);
    hsvSLayout->addWidget(hsvSLabel);
    hsvSLayout->addWidget(hsv_s_rect);
    hsvSLayout->addWidget(hsvSSpin);
    v_rect = new ColorValueRect(CVR_VALUE, this);
    vLabel->setFixedWidth(LABEL_WIDTH);
    vLayout->addWidget(vLabel);
    vLayout->addWidget(v_rect);
    vLayout->addWidget(vSpin);
    mHSVLayout->setAlignment(Qt::AlignTop);
    mHSVLayout->addLayout(hLayout);
    mHSVLayout->addLayout(hsvSLayout);
    mHSVLayout->addLayout(vLayout);
    mHSVWidget->setLayout(mHSVLayout);

    hsl_s_rect = new ColorValueRect(CVR_HSLSATURATION, this);
    hslSLabel->setFixedWidth(LABEL_WIDTH);
    hslSLayout->addWidget(hslSLabel);
    hslSLayout->addWidget(hsl_s_rect);
    hslSLayout->addWidget(hslSSpin);
    l_rect = new ColorValueRect(CVR_LIGHTNESS, this);
    lLabel->setFixedWidth(LABEL_WIDTH);
    lLayout->addWidget(lLabel);
    lLayout->addWidget(l_rect);
    lLayout->addWidget(lSpin);
    mHSLLayout->setAlignment(Qt::AlignTop);
    mHSLLayout->addLayout(hslSLayout);
    mHSLLayout->addLayout(lLayout);
    mHSLWidget->setLayout(mHSLLayout);

    aRect = new ColorValueRect(CVR_ALPHA, this);
    aLabel->setFixedWidth(LABEL_WIDTH);
    aLayout->addWidget(aLabel);
    aLayout->addWidget(aRect);
    aLayout->addWidget(aSpin);


    mPickingButton = new ActionButton(":/icons/draw_dropper.png",
                                     "", this);
    connect(mPickingButton, SIGNAL(released()),
            this, SLOT(startColorPicking()));
    mColorLabelLayout->addWidget(color_label);
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
    connect(mColorModeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setColorMode(int)));

    mWidgetsLayout->addLayout(mColorModeLayout);

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(moveAlphaWidgetToTab(int)));

    connect(rSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromRGB()));
    connect(gSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromRGB()));
    connect(bSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromRGB()));
    connect(hSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromHSV()));
    connect(hsvSSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromHSV()));
    connect(vSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromHSV()));
    connect(hslSSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromHSL()));
    connect(lSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setValuesFromHSL()));
    connect(aSpin, SIGNAL(valueChanged(qreal)),
            this, SLOT(setAlphaFromSpin(qreal)));

    connect(rSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromRGB()));
    connect(gSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromRGB()));
    connect(bSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromRGB()));
    connect(hSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromHSV()));
    connect(hsvSSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromHSV()));
    connect(vSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromHSV()));
    connect(hslSSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromHSL()));
    connect(lSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateValuesFromHSL()));
    connect(aSpin, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(updateAlphaFromSpin()));

    connect(rSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedRed()));
    connect(gSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedGreen()));
    connect(bSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedBlue()));
    connect(hSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedHue()));
    connect(hsvSSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedHSVSaturation()));
    connect(vSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedValue()));
    connect(hslSSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedHSLSaturation()));
    connect(lSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedLightness()));
    connect(aSpin, SIGNAL(editingStarted(qreal)),
            this, SLOT(emitEditingStartedAlpha()));

    connect(rSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(gSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(bSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(hSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(hsvSSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(vSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(hslSSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(lSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));
    connect(aSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitEditingFinishedSignal()));

    connect(r_rect, SIGNAL(editingStarted(qreal)),
            rSpin, SLOT(emitEditingStarted(qreal)));
    connect(g_rect, SIGNAL(editingStarted(qreal)),
            gSpin, SLOT(emitEditingStarted(qreal)));
    connect(b_rect, SIGNAL(editingStarted(qreal)),
            bSpin, SLOT(emitEditingStarted(qreal)));
    connect(h_rect, SIGNAL(editingStarted(qreal)),
            hSpin, SLOT(emitEditingStarted(qreal)));
    connect(hsv_s_rect, SIGNAL(editingStarted(qreal)),
            hsvSSpin, SLOT(emitEditingStarted(qreal)));
    connect(v_rect, SIGNAL(editingStarted(qreal)),
            vSpin, SLOT(emitEditingStarted(qreal)));
    connect(hsl_s_rect, SIGNAL(editingStarted(qreal)),
            hslSSpin, SLOT(emitEditingStarted(qreal)));
    connect(l_rect, SIGNAL(editingStarted(qreal)),
            lSpin, SLOT(emitEditingStarted(qreal)));
    connect(aRect, SIGNAL(editingStarted(qreal)),
            aSpin, SLOT(emitEditingStarted(qreal)));

    connect(r_rect, SIGNAL(editingFinished(qreal)),
            rSpin, SLOT(emitEditingFinished(qreal)));
    connect(g_rect, SIGNAL(editingFinished(qreal)),
            gSpin, SLOT(emitEditingFinished(qreal)));
    connect(b_rect, SIGNAL(editingFinished(qreal)),
            bSpin, SLOT(emitEditingFinished(qreal)));
    connect(h_rect, SIGNAL(editingFinished(qreal)),
            hSpin, SLOT(emitEditingFinished(qreal)));
    connect(hsv_s_rect, SIGNAL(editingFinished(qreal)),
            hsvSSpin, SLOT(emitEditingFinished(qreal)));
    connect(v_rect, SIGNAL(editingFinished(qreal)),
            vSpin, SLOT(emitEditingFinished(qreal)));
    connect(hsl_s_rect, SIGNAL(editingFinished(qreal)),
            hslSSpin, SLOT(emitEditingFinished(qreal)));
    connect(l_rect, SIGNAL(editingFinished(qreal)),
            lSpin, SLOT(emitEditingFinished(qreal)));
    connect(aRect, SIGNAL(editingFinished(qreal)),
            aSpin, SLOT(emitEditingFinished(qreal)));

    connect(r_rect, SIGNAL(valChanged(qreal)),
            rSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(g_rect, SIGNAL(valChanged(qreal)),
            gSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(b_rect, SIGNAL(valChanged(qreal)),
            bSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(h_rect, SIGNAL(valChanged(qreal)),
            hSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(hsv_s_rect, SIGNAL(valChanged(qreal)),
            hsvSSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(v_rect, SIGNAL(valChanged(qreal)),
            vSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(hsl_s_rect, SIGNAL(valChanged(qreal)),
            hslSSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(l_rect, SIGNAL(valChanged(qreal)),
            lSpin, SLOT(emitValueChangedExternal(qreal)));
    connect(aRect, SIGNAL(valChanged(qreal)),
            aSpin, SLOT(emitValueChangedExternal(qreal)));


    //setMinimumSize(250, 200);
    mTabWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Maximum);
    setCurrentColor(0.f, 0.f, 0.f);

    moveAlphaWidgetToTab(0);
}

void addColorWidgetActionToMenu(QMenu *menu_t,
                                QString label_t,
                                ColorWidget *widget_t) {
    QAction *action_t = menu_t->addAction(label_t);
    action_t->setCheckable(true);
    action_t->setChecked(widget_t->isVisible() );
}

void ColorSettingsWidget::setRectValuesAndColor(
        const qreal &red, const qreal &green, const qreal &blue,
        const qreal &hue, const qreal &hsvSaturation, const qreal &value,
        const qreal &hslSaturation, const qreal &lightness) {
    GLfloat hueGl = hue;
    GLfloat satGl = hsvSaturation;
    GLfloat valGl = value;

    r_rect->setDisplayedValue(red);
    g_rect->setDisplayedValue(green);
    b_rect->setDisplayedValue(blue);

    h_rect->setDisplayedValue(hueGl);
    hsv_s_rect->setDisplayedValue(satGl);
    v_rect->setDisplayedValue(valGl);

    hsl_s_rect->setDisplayedValue(hslSaturation);
    l_rect->setDisplayedValue(lightness);

    r_rect->setColorHSV_f(hueGl, satGl, valGl);
    g_rect->setColorHSV_f(hueGl, satGl, valGl);
    b_rect->setColorHSV_f(hueGl, satGl, valGl);

    h_rect->setColorHSV_f(hueGl, satGl, valGl);
    hsv_s_rect->setColorHSV_f(hueGl, satGl, valGl);
    v_rect->setColorHSV_f(hueGl, satGl, valGl);

    hsl_s_rect->setColorHSV_f(hueGl, satGl, valGl);
    l_rect->setColorHSV_f(hueGl, satGl, valGl);

    if(!mAlphaHidden) {
        aRect->setColorHSV_f(hueGl, satGl, valGl);
    }

    color_label->setColorHSV_f(hueGl, satGl, valGl);

    //emit colorChangedHSVSignal(hueGl, satGl, valGl, aSpin->value());
}

void ColorSettingsWidget::updateValuesFromRGB() {
    qreal red = rSpin->value();
    qreal green = gSpin->value();
    qreal blue = bSpin->value();

    qreal hue = red;
    qreal hsvSaturation = green;
    qreal value = blue;
    qrgb_to_hsv(&hue, &hsvSaturation, &value);

    hue = red;
    qreal hslSaturation = green;
    qreal lightness = blue;
    qrgb_to_hsl(&hue, &hslSaturation, &lightness);

    hSpin->setValueExternal(hue);
    hsvSSpin->setValueExternal(hsvSaturation);
    vSpin->setValueExternal(value);

    hslSSpin->setValueExternal(hslSaturation);
    lSpin->setValueExternal(lightness);

    setRectValuesAndColor(red, green, blue,
                          hue, hsvSaturation, value,
                          hslSaturation, lightness);
}

void ColorSettingsWidget::updateValuesFromHSV() {
    qreal hue = hSpin->value();
    qreal hsvSaturation = hsvSSpin->value();
    qreal value = vSpin->value();

    qreal red = hue;
    qreal green = hsvSaturation;
    qreal blue = value;
    qhsv_to_rgb(&red, &green, &blue);

    qreal hslSaturation = hsvSaturation;
    qreal lightness = value;
    qhsv_to_hsl(&hue, &hslSaturation, &lightness);

    rSpin->setValueExternal(red);
    gSpin->setValueExternal(green);
    bSpin->setValueExternal(blue);

    hslSSpin->setValueExternal(hslSaturation);
    lSpin->setValueExternal(lightness);

    setRectValuesAndColor(red, green, blue,
                          hue, hsvSaturation, value,
                          hslSaturation, lightness);
}

void ColorSettingsWidget::updateValuesFromHSL() {
    qreal hue = hSpin->value();
    qreal hslSaturation = hslSSpin->value();
    qreal lightness = lSpin->value();

    qreal red = hue;
    qreal green = hslSaturation;
    qreal blue = lightness;
    qhsl_to_rgb(&red, &green, &blue);

    qreal hsvSaturation = hslSaturation;
    qreal value = lightness;
    qhsl_to_hsv(&hue, &hsvSaturation, &value);

    rSpin->setValueExternal(red);
    gSpin->setValueExternal(green);
    bSpin->setValueExternal(blue);

    hsvSSpin->setValueExternal(hsvSaturation);
    vSpin->setValueExternal(value);

    setRectValuesAndColor(red, green, blue,
                          hue, hsvSaturation, value,
                          hslSaturation, lightness);
}

void ColorSettingsWidget::setValuesFromRGB() {
    updateValuesFromRGB();
    emitColorChangedSignal();
}

void ColorSettingsWidget::setValuesFromHSV() {
    updateValuesFromHSV();
    emitColorChangedSignal();
}

void ColorSettingsWidget::setValuesFromHSL() {
    updateValuesFromHSL();
    emitColorChangedSignal();
}

void ColorSettingsWidget::updateAlphaFromSpin() {
    if(mAlphaHidden) return;
    color_label->setAlpha(aSpin->value());
    aRect->setDisplayedValue(aSpin->value());
}

void ColorSettingsWidget::setColorMode(const int &colorMode) {
    if(mTargetAnimator != NULL) {
        mTargetAnimator->setColorMode(static_cast<ColorMode>(colorMode));
    }
    emit colorModeChanged(static_cast<ColorMode>(colorMode));
    MainWindow::getInstance()->callUpdateSchedulers();
}

void ColorSettingsWidget::setAlphaFromSpin(const qreal &val) {
    if(mAlphaHidden) return;
    color_label->setAlpha(val);
    aRect->setDisplayedValue(val);

    emitColorChangedSignal();
}
