#include "colorsettingswidget.h"
#include "GUI/mainwindow.h"
#include <QResizeEvent>
#include <QMenu>
#include "GUI/ColorWidgets/colorpickingwidget.h"
#include "colorhelpers.h"
#include "colorlabel.h"
#include "global.h"
#include "GUI/actionbutton.h"

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
    GLfloat alphaGl = static_cast<GLfloat>(a_t);
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

    rSpin->setValueExternal(red);
    gSpin->setValueExternal(green);
    bSpin->setValueExternal(blue);

    hSpin->setValueExternal(hue);
    hsvSSpin->setValueExternal(hsvSat);
    vSpin->setValueExternal(val);

    qreal hslSat = hsvSat;
    qreal lig = val;
    qhsv_to_hsl(hue, hslSat, lig);

    hsl_s_rect->setDisplayedValue(hslSat);
    l_rect->setDisplayedValue(lig);

    hslSSpin->setValueExternal(hslSat);
    lSpin->setValueExternal(lig);

    if(mAlphaHidden) return;
    mColorLabel->setAlpha(alphaGl);
    aRect->setColorHSV_f(hueGl, satGl, valGl);
    aRect->setDisplayedValue(a_t);
    aSpin->setValueExternal(a_t);
}

void ColorSettingsWidget::setCurrentColor(const QColor &color) {
    setCurrentColor(color.hueF(), color.saturationF(), color.valueF(),
                    color.alphaF());
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
    setTarget(mTargetAnimator);
}

void ColorSettingsWidget::nullifyAnimator() {
    setTarget(nullptr);
}

void ColorSettingsWidget::setTarget(ColorAnimator *target) {
    if(mTargetAnimator) {
        disconnect(mTargetAnimator, nullptr, this, nullptr);
    }
    mTargetAnimator = target;
    rSpin->clearTarget();
    gSpin->clearTarget();
    bSpin->clearTarget();
    hSpin->clearTarget();
    hslSSpin->clearTarget();
    lSpin->clearTarget();
    hsvSSpin->clearTarget();
    vSpin->clearTarget();
    if(!mAlphaHidden) {
        aSpin->clearTarget();
    }
    if(target) {
        disconnect(mColorModeCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setColorMode(int)));
        mColorModeCombo->setCurrentIndex(target->getColorMode());
        connect(mColorModeCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setColorMode(int)));
        if(!mAlphaHidden) {
            aSpin->setTarget(target->getAlphaAnimator());
        }
        if(target->getColorMode() == RGBMODE) {
            rSpin->setTarget(target->getVal1Animator());
            gSpin->setTarget(target->getVal2Animator());
            bSpin->setTarget(target->getVal3Animator());

            updateValuesFromRGB();
        } else if(target->getColorMode() == HSLMODE) {
            hSpin->setTarget(target->getVal1Animator());
            hslSSpin->setTarget(target->getVal2Animator());
            lSpin->setTarget(target->getVal3Animator());

            updateValuesFromHSL();
        } else { // HSVMODE
            hSpin->setTarget(target->getVal1Animator());
            hsvSSpin->setTarget(target->getVal2Animator());
            vSpin->setTarget(target->getVal3Animator());

            updateValuesFromHSV();
        }

        updateAlphaFromSpin();
        connect(target, SIGNAL(colorModeChanged(ColorMode)),
                this, SLOT(refreshColorAnimatorTarget()));
        connect(target, SIGNAL(beingDeleted()),
                this, SLOT(nullifyAnimator()));
    }
}

ColorSetting ColorSettingsWidget::getColorSetting(
        const ColorSettingType& type) const {
    const int tabId = mTabWidget->currentIndex();
    qreal alphaVal = 1;
    if(!mAlphaHidden) alphaVal = aSpin->value();
    if(tabId == 0) {
        return ColorSetting(
                    RGBMODE, mLastTriggeredCVR,
                    rSpin->value(),
                    gSpin->value(),
                    bSpin->value(),
                    alphaVal,
                    type, mTargetAnimator);
    } else if(tabId == 1) {
        return ColorSetting(
                    HSVMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hsvSSpin->value(),
                    vSpin->value(),
                    alphaVal,
                    type, mTargetAnimator);
    } else if(tabId == 2) {
        return ColorSetting(
                    HSLMODE, mLastTriggeredCVR,
                    hSpin->value(),
                    hslSSpin->value(),
                    lSpin->value(),
                    alphaVal,
                    type, mTargetAnimator);
    }
    return ColorSetting();
}

void ColorSettingsWidget::emitColorChangedSignal() {
    emit colorSettingSignal(getColorSetting(CST_CHANGE));
}

void ColorSettingsWidget::emitEditingFinishedSignal() {
    int tabId = mTabWidget->currentIndex();
    if(mTargetAnimator) {
        if(mTargetAnimator->getColorMode() != tabId) {
            mTargetAnimator->prp_finishTransform();
        }
    }
    emit colorSettingSignal(getColorSetting(CST_FINISH));
}

void ColorSettingsWidget::emitEditingStartedSignal() {
    int tabId = mTabWidget->currentIndex();
    if(mTargetAnimator) {
        if(mTargetAnimator->getColorMode() != tabId) {
            mTargetAnimator->startVal1Transform();
            mTargetAnimator->startVal2Transform();
            mTargetAnimator->startVal3Transform();
        }
    }
    emit colorSettingSignal(getColorSetting(CST_START));
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
    if(mTargetAnimator) mTargetAnimator->prp_startTransform();
    updateValuesFromHSV();
    updateAlphaFromSpin();
    if(mTargetAnimator) mTargetAnimator->prp_finishTransform();
    emitEditingStartedSignal();
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
    const auto wid = new ColorPickingWidget(MainWindow::getInstance());
    connect(wid, &ColorPickingWidget::colorSelected,
            [this](const QColor & color) {
        setCurrentColor(color);
        emitFullColorChangedSignal();
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
    connect(mColorModeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setColorMode(int)));

    mWidgetsLayout->addLayout(mColorModeLayout);

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(moveAlphaWidgetToTab(int)));

    connect(rSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromRGB);
    connect(gSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromRGB);
    connect(bSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromRGB);

    connect(hSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromHSV);
    connect(hsvSSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromHSV);
    connect(vSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromHSV);
    connect(hslSSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromHSL);
    connect(lSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setValuesFromHSL);
    connect(aSpin, &QrealAnimatorValueSlider::valueChanged,
            this, &ColorSettingsWidget::setAlphaFromSpin);

    connect(rSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromRGB);
    connect(gSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromRGB);
    connect(bSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromRGB);

    connect(hSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromHSV);
    connect(hsvSSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromHSV);
    connect(vSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromHSV);

    connect(hslSSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromHSL);
    connect(lSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateValuesFromHSL);

    connect(aSpin, &QrealAnimatorValueSlider::displayedValueChanged,
            this, &ColorSettingsWidget::updateAlphaFromSpin);

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

    connect(r_rect, &ColorValueRect::editingStarted,
            rSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(g_rect, &ColorValueRect::editingStarted,
            gSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(b_rect, &ColorValueRect::editingStarted,
            bSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(h_rect, &ColorValueRect::editingStarted,
            hSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(hsv_s_rect, &ColorValueRect::editingStarted,
            hsvSSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(v_rect, &ColorValueRect::editingStarted,
            vSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(hsl_s_rect, &ColorValueRect::editingStarted,
            hslSSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(l_rect, &ColorValueRect::editingStarted,
            lSpin, &QrealAnimatorValueSlider::emitEditingStarted);
    connect(aRect, &ColorValueRect::editingStarted,
            aSpin, &QrealAnimatorValueSlider::emitEditingStarted);

    connect(r_rect, &ColorValueRect::editingFinished,
            rSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(g_rect, &ColorValueRect::editingFinished,
            gSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(b_rect, &ColorValueRect::editingFinished,
            bSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(h_rect, &ColorValueRect::editingFinished,
            hSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(hsv_s_rect, &ColorValueRect::editingFinished,
            hsvSSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(v_rect, &ColorValueRect::editingFinished,
            vSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(hsl_s_rect, &ColorValueRect::editingFinished,
            hslSSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(l_rect, &ColorValueRect::editingFinished,
            lSpin, &QrealAnimatorValueSlider::emitEditingFinished);
    connect(aRect, &ColorValueRect::editingFinished,
            aSpin, &QrealAnimatorValueSlider::emitEditingFinished);

    connect(r_rect, &ColorValueRect::valChanged,
            rSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(g_rect, &ColorValueRect::valChanged,
            gSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(b_rect, &ColorValueRect::valChanged,
            bSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(h_rect, &ColorValueRect::valChanged,
            hSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(hsv_s_rect, &ColorValueRect::valChanged,
            hsvSSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(v_rect, &ColorValueRect::valChanged,
            vSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(hsl_s_rect, &ColorValueRect::valChanged,
            hslSSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(l_rect, &ColorValueRect::valChanged,
            lSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);
    connect(aRect, &ColorValueRect::valChanged,
            aSpin, &QrealAnimatorValueSlider::emitValueChangedExternal);


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

void ColorSettingsWidget::setRectValuesAndColor(
        const qreal red, const qreal green, const qreal blue,
        const qreal hue, const qreal hsvSaturation, const qreal value,
        const qreal hslSaturation, const qreal lightness) {
    GLfloat hueGl = static_cast<GLfloat>(hue);
    GLfloat satGl = static_cast<GLfloat>(hsvSaturation);
    GLfloat valGl = static_cast<GLfloat>(value);

    r_rect->setDisplayedValue(red);
    g_rect->setDisplayedValue(green);
    b_rect->setDisplayedValue(blue);

    h_rect->setDisplayedValue(hue);
    hsv_s_rect->setDisplayedValue(hsvSaturation);
    v_rect->setDisplayedValue(value);

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

    mColorLabel->setColorHSV_f(hueGl, satGl, valGl);

    //emit colorChangedHSVSignal(hueGl, satGl, valGl, aSpin->value());
}

void ColorSettingsWidget::updateValuesFromRGB() {
    if(mBlockColorSettings) return;
    mBlockColorSettings = true;
    qreal red = rSpin->value();
    qreal green = gSpin->value();
    qreal blue = bSpin->value();

    qreal hue = red;
    qreal hsvSaturation = green;
    qreal value = blue;
    qrgb_to_hsv(hue, hsvSaturation, value);

    hue = red;
    qreal hslSaturation = green;
    qreal lightness = blue;
    qrgb_to_hsl(hue, hslSaturation, lightness);

    hSpin->setValueExternal(hue);
    hsvSSpin->setValueExternal(hsvSaturation);
    vSpin->setValueExternal(value);

    hslSSpin->setValueExternal(hslSaturation);
    lSpin->setValueExternal(lightness);

    setRectValuesAndColor(red, green, blue,
                          hue, hsvSaturation, value,
                          hslSaturation, lightness);
    mBlockColorSettings = false;
}

void ColorSettingsWidget::updateValuesFromHSV() {
    if(mBlockColorSettings) return;
    mBlockColorSettings = true;
    qreal hue = hSpin->value();
    qreal hsvSaturation = hsvSSpin->value();
    qreal value = vSpin->value();

    qreal red = hue;
    qreal green = hsvSaturation;
    qreal blue = value;
    qhsv_to_rgb(red, green, blue);

    qreal hslSaturation = hsvSaturation;
    qreal lightness = value;
    qhsv_to_hsl(hue, hslSaturation, lightness);

    rSpin->setValueExternal(red);
    gSpin->setValueExternal(green);
    bSpin->setValueExternal(blue);

    hslSSpin->setValueExternal(hslSaturation);
    lSpin->setValueExternal(lightness);

    setRectValuesAndColor(red, green, blue,
                          hue, hsvSaturation, value,
                          hslSaturation, lightness);
    mBlockColorSettings = false;
}

void ColorSettingsWidget::updateValuesFromHSL() {
    if(mBlockColorSettings) return;
    mBlockColorSettings = true;
    qreal hue = hSpin->value();
    qreal hslSaturation = hslSSpin->value();
    qreal lightness = lSpin->value();

    qreal red = hue;
    qreal green = hslSaturation;
    qreal blue = lightness;
    qhsl_to_rgb(red, green, blue);

    qreal hsvSaturation = hslSaturation;
    qreal value = lightness;
    qhsl_to_hsv(hue, hsvSaturation, value);

    rSpin->setValueExternal(red);
    gSpin->setValueExternal(green);
    bSpin->setValueExternal(blue);

    hsvSSpin->setValueExternal(hsvSaturation);
    vSpin->setValueExternal(value);

    setRectValuesAndColor(red, green, blue,
                          hue, hsvSaturation, value,
                          hslSaturation, lightness);
    mBlockColorSettings = false;
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
    mColorLabel->setAlpha(aSpin->value());
    aRect->setDisplayedValue(aSpin->value());
}

void ColorSettingsWidget::setColorMode(const int colorMode) {
    if(mTargetAnimator) {
        mTargetAnimator->setColorMode(static_cast<ColorMode>(colorMode));
    }
    emit colorModeChanged(static_cast<ColorMode>(colorMode));
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void ColorSettingsWidget::setAlphaFromSpin(const qreal val) {
    if(mAlphaHidden) return;
    mColorLabel->setAlpha(val);
    aRect->setDisplayedValue(val);

    emitColorChangedSignal();
}
