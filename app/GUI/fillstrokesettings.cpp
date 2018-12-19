#include "fillstrokesettings.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "mainwindow.h"
#include "undoredo.h"
#include "canvas.h"
#include "qrealanimatorvalueslider.h"
#include "GUI/ColorWidgets/colorsettingswidget.h"
#include "actionbutton.h"
#include "qdoubleslider.h"

FillStrokeSettingsWidget::FillStrokeSettingsWidget(MainWindow *parent) :
    QWidget(parent) {
    //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mMainWindow = parent;
    mUndoRedoSaveTimer = new QTimer(this);

    mGradientWidget = new GradientWidget(this, mMainWindow);
    mStrokeSettingsWidget = new QWidget(this);
    mColorsSettingsWidget = new ColorSettingsWidget(this);

    mTargetLayout->setSpacing(0);
    mFillTargetButton = new QPushButton(
                QIcon(":/icons/properties_fill.png"),
                "Fill", this);
    mFillTargetButton->setObjectName("leftButton");
    mStrokeTargetButton = new QPushButton(
                QIcon(":/icons/properties_stroke_paint.png"),
                "Stroke", this);
    mStrokeTargetButton->setObjectName("rightButton");
    setLayout(mMainLayout);
    mMainLayout->setAlignment(Qt::AlignTop);

    mColorTypeLayout = new QHBoxLayout();
    mColorTypeLayout->setSpacing(0);
    mFillNoneButton = new QPushButton(
                QIcon(":/icons/fill_none.png"),
                "None", this);
    mFillNoneButton->setCheckable(true);
    mFillNoneButton->setObjectName("leftButton");
    connect(mFillNoneButton, SIGNAL(released()),
            this, SLOT(setNoneFill()));
    mFillFlatButton = new QPushButton(
                QIcon(":/icons/fill_flat.png"),
                "Flat", this);
    mFillFlatButton->setCheckable(true);
    mFillFlatButton->setObjectName("middleButton");
    connect(mFillFlatButton, SIGNAL(released()),
            this, SLOT(setFlatFill()));
    mFillGradientButton = new QPushButton(
                QIcon(":/icons/fill_gradient.png"),
                "Gradient", this);
    mFillGradientButton->setCheckable(true);
    mFillGradientButton->setObjectName("rightButton");
    connect(mFillGradientButton, SIGNAL(released()),
            this, SLOT(setGradientFill()));

    mColorTypeLayout->addWidget(mFillNoneButton);
    mColorTypeLayout->addWidget(mFillFlatButton);
    mColorTypeLayout->addWidget(mFillGradientButton);

    mFillTargetButton->setCheckable(true);
    mFillTargetButton->setFocusPolicy(Qt::NoFocus);
    mStrokeTargetButton->setCheckable(true);
    mStrokeTargetButton->setFocusPolicy(Qt::NoFocus);
    mTargetLayout->addWidget(mFillTargetButton);
    mTargetLayout->addWidget(mStrokeTargetButton);

    //mLineWidthSpin = new QDoubleSpinBox(this);
    mLineWidthSpin = new QrealAnimatorValueSlider("line width",
                                                  0., 1000., 1., this);
    mLineWidthSpin->setNameVisible(false);
    //mLineWidthSpin->setValueSliderVisibile(false);
    //mLineWidthSpin->setRange(0.0, 1000.0);
    //mLineWidthSpin->setSuffix(" px");
    //mLineWidthSpin->setSingleStep(0.1);
    mLineWidthLayout->addWidget(mLineWidthLabel);
    mLineWidthLayout->addWidget(mLineWidthSpin, Qt::AlignLeft);

    mStrokeSettingsLayout->addLayout(mLineWidthLayout);

    mJoinStyleLayout->setSpacing(0);
    mBevelJoinStyleButton = new QPushButton(QIcon(":/icons/join_bevel.png"),
                                            "", this);
    mBevelJoinStyleButton->setObjectName("leftButton");
    mMiterJointStyleButton = new QPushButton(QIcon(":/icons/join_miter.png"),
                                             "", this);
    mMiterJointStyleButton->setObjectName("middleButton");
    mRoundJoinStyleButton = new QPushButton(QIcon(":/icons/join_round.png"),
                                            "", this);
    mRoundJoinStyleButton->setObjectName("rightButton");

    mBevelJoinStyleButton->setCheckable(true);
    mMiterJointStyleButton->setCheckable(true);
    mRoundJoinStyleButton->setCheckable(true);
    connect(mBevelJoinStyleButton, SIGNAL(released()),
            this, SLOT(setBevelJoinStyle()) );
    connect(mMiterJointStyleButton, SIGNAL(released()),
            this, SLOT(setMiterJoinStyle()) );
    connect(mRoundJoinStyleButton, SIGNAL(released()),
            this, SLOT(setRoundJoinStyle()) );

    mJoinStyleLayout->addWidget(new QLabel("Join:", this));
    mJoinStyleLayout->addWidget(mBevelJoinStyleButton);
    mJoinStyleLayout->addWidget(mMiterJointStyleButton);
    mJoinStyleLayout->addWidget(mRoundJoinStyleButton);

    mStrokeSettingsLayout->addLayout(mJoinStyleLayout);

    mCapStyleLayout->setSpacing(0);
    mFlatCapStyleButton = new QPushButton(QIcon(":/icons/cap_flat.png"),
                                          "", this);
    mFlatCapStyleButton->setObjectName("leftButton");
    mSquareCapStyleButton = new QPushButton(QIcon(":/icons/cap_square.png"),
                                            "", this);
    mSquareCapStyleButton->setObjectName("middleButton");
    mRoundCapStyleButton = new QPushButton(QIcon(":/icons/cap_round.png"),
                                           "", this);
    mRoundCapStyleButton->setObjectName("rightButton");
    mFlatCapStyleButton->setCheckable(true);
    mSquareCapStyleButton->setCheckable(true);
    mRoundCapStyleButton->setCheckable(true);
    connect(mFlatCapStyleButton, SIGNAL(released()),
            this, SLOT(setFlatCapStyle()) );
    connect(mSquareCapStyleButton, SIGNAL(released()),
            this, SLOT(setSquareCapStyle()) );
    connect(mRoundCapStyleButton, SIGNAL(released()),
            this, SLOT(setRoundCapStyle()) );

    mCapStyleLayout->addWidget(new QLabel("Cap:", this));
    mCapStyleLayout->addWidget(mFlatCapStyleButton);
    mCapStyleLayout->addWidget(mSquareCapStyleButton);
    mCapStyleLayout->addWidget(mRoundCapStyleButton);

    mStrokeSettingsLayout->addLayout(mCapStyleLayout);

    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));

    connect(mLineWidthSpin, SIGNAL(editingFinished(qreal)),
            this, SLOT(emitStrokeWidthChanged()));

    mStrokeSettingsWidget->setLayout(mStrokeSettingsLayout);

    connect(mFillTargetButton, SIGNAL(released()),
            this, SLOT(setFillTarget()) );
    connect(mStrokeTargetButton, SIGNAL(released()),
            this, SLOT(setStrokeTarget()) );

    connect(mColorsSettingsWidget,
            SIGNAL(colorSettingSignal(ColorSetting)),
            this, SLOT(colorSettingReceived(ColorSetting)));

    connect(mColorsSettingsWidget, SIGNAL(colorModeChanged(ColorMode)),
            this, SLOT(setCurrentColorMode(ColorMode)));

    mGradientTypeLayout = new QHBoxLayout();
    mGradientTypeLayout->setSpacing(0);
    mLinearGradientButton = new QPushButton(
                QIcon(":/icons/fill_gradient.png"),
                "Linear", this);
    mLinearGradientButton->setCheckable(true);
    mLinearGradientButton->setObjectName("leftButton");
    connect(mLinearGradientButton, SIGNAL(released()),
            this, SLOT(setLinearGradientFill()));

    mRadialGradientButton = new QPushButton(
                QIcon(":/icons/fill_gradient_radial.png"),
                "Radial", this);
    mRadialGradientButton->setCheckable(true);
    mRadialGradientButton->setObjectName("rightButton");
    connect(mRadialGradientButton, SIGNAL(released()),
            this, SLOT(setRadialGradientFill()));

    mGradientTypeLayout->addWidget(mLinearGradientButton);
    mGradientTypeLayout->addWidget(mRadialGradientButton);
    mGradientTypeWidget = new QWidget(this);
    mGradientTypeWidget->setContentsMargins(0, 0, 0, 0);
    mGradientTypeWidget->setLayout(mGradientTypeLayout);

    mMainLayout->addLayout(mTargetLayout);
    mMainLayout->addLayout(mColorTypeLayout);
    mMainLayout->addWidget(mGradientTypeWidget);
    mMainLayout->addWidget(mStrokeSettingsWidget);
    mMainLayout->addWidget(mGradientWidget);
    mMainLayout->addWidget(mColorsSettingsWidget);
    mMainLayout->addStretch(1);

    mGradientTypeWidget->hide();

    setFillTarget();
    setCapStyle(Qt::RoundCap);
    setJoinStyle(Qt::RoundJoin);
}

void FillStrokeSettingsWidget::setLinearGradientFill() {
    setGradientLinear(true);
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void FillStrokeSettingsWidget::setRadialGradientFill() {
    setGradientLinear(false);
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void FillStrokeSettingsWidget::setGradientFill() {
    mFillGradientButton->setChecked(true);
    mFillFlatButton->setChecked(false);
    mFillNoneButton->setChecked(false);
    colorTypeSet(2);
}

void FillStrokeSettingsWidget::setFlatFill() {
    mFillGradientButton->setChecked(false);
    mFillFlatButton->setChecked(true);
    mFillNoneButton->setChecked(false);
    colorTypeSet(1);
}

void FillStrokeSettingsWidget::setNoneFill() {
    mFillGradientButton->setChecked(false);
    mFillFlatButton->setChecked(false);
    mFillNoneButton->setChecked(true);
    colorTypeSet(0);
}

void FillStrokeSettingsWidget::updateColorAnimator() {
    if(getCurrentPaintTypeVal() == 0) {
        setColorAnimatorTarget(nullptr);
    } else if(getCurrentPaintTypeVal() == 1) {
        if(mTargetId == 0) {
            setColorAnimatorTarget(mCurrentFillColorAnimator);
        } else {
            setColorAnimatorTarget(mCurrentStrokeColorAnimator);
        }
    } else {// if(getCurrentPaintTypeVal() == 2) {
        setColorAnimatorTarget(mGradientWidget->getCurrentColorAnimator());
    }
}

void FillStrokeSettingsWidget::setCurrentColorMode(const ColorMode &mode) {
    if(mTargetId == 0) {
        if(mCurrentFillPaintType == FLATPAINT) {
            mCanvasWindow->setSelectedFillColorMode(mode);
        }
    } else {
        if(mCurrentStrokePaintType == FLATPAINT) {
            mCanvasWindow->setSelectedStrokeColorMode(mode);
        }
    }
}

void FillStrokeSettingsWidget::updateAfterTargetChanged() {
    if(getCurrentPaintTypeVal() == GRADIENTPAINT) {
        mGradientWidget->setCurrentGradient(getCurrentGradientVal(), false);
        mLinearGradientButton->setChecked(getCurrentGradientLinearVal());
        mRadialGradientButton->setChecked(!getCurrentGradientLinearVal());
    }
    setCurrentPaintType(getCurrentPaintTypeVal());
    if(getCurrentPaintTypeVal() == 0) {
        mFillGradientButton->setChecked(false);
        mFillFlatButton->setChecked(false);
        mFillNoneButton->setChecked(true);
    } else if(getCurrentPaintTypeVal() == 1) {
        mFillGradientButton->setChecked(false);
        mFillFlatButton->setChecked(true);
        mFillNoneButton->setChecked(false);
    } else {
        mFillGradientButton->setChecked(true);
        mFillFlatButton->setChecked(false);
        mFillNoneButton->setChecked(false);
    }
}

void FillStrokeSettingsWidget::setCurrentPaintType(
        const PaintType &paintType) {
    if(paintType == NOPAINT) {
        setNoPaintType();
    } else if(paintType == FLATPAINT) {
        setFlatPaintType();
    } else {
        setGradientPaintType();
    }
}

void FillStrokeSettingsWidget::setTransformFinishEmitter(const char *slot) {
    mUndoRedoSaveTimer->disconnect();
    connect(mUndoRedoSaveTimer, SIGNAL(timeout()),
            this, SLOT(finishTransform() ) );
    connect(mUndoRedoSaveTimer, SIGNAL(timeout()),
            this, slot );
}

void FillStrokeSettingsWidget::setStrokeWidth(const qreal &width) {
    //startTransform(SLOT(emitStrokeWidthChanged()));
    mCurrentStrokeWidth = width;
    emitStrokeWidthChangedTMP();
}

void FillStrokeSettingsWidget::setCurrentSettings(
        PaintSettings *fillPaintSettings,
        StrokeSettings *strokePaintSettings) {
    disconnect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));

    setFillValuesFromFillSettings(fillPaintSettings);
    setStrokeValuesFromStrokeSettings(strokePaintSettings);
    //mLineWidthSpin->setValue(strokePaintSettings->getCurrentStrokeWidth());

    if(mTargetId == 0) {
        setFillTarget();
    } else {
        setStrokeTarget();
    }

    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
}

GradientWidget *FillStrokeSettingsWidget::getGradientWidget() {
    return mGradientWidget;
}

void FillStrokeSettingsWidget::clearAll()
{
    mGradientWidget->clearAll();
}

void FillStrokeSettingsWidget::colorTypeSet(const int &id) {
    if(id == 0) {
        setNoPaintType();
    } else if(id == 1) {
        setFlatPaintType();
    } else {
        if((mTargetId == 0) ? (mCurrentFillGradient == nullptr) :
                (mCurrentStrokeGradient == nullptr) ) {
            mGradientWidget->setCurrentGradient(nullptr);
        }
        setGradientPaintType();
    }

    bool isFill;
    PaintType currentPaintType;
    Gradient *currentGradient;
    bool currentGradientLinear;
    if(mTargetId == 0) {
        isFill = true;
        currentPaintType = mCurrentFillPaintType;
        currentGradient = mCurrentFillGradient;
        currentGradientLinear = mCurrentFillGradientLinear;
    } else {
        isFill = false;
        currentPaintType = mCurrentStrokePaintType;
        currentGradient = mCurrentStrokeGradient;
        currentGradientLinear = mCurrentStrokeGradientLinear;
    }
    stdsptr<PaintSetting> paintSetting;
    if(currentPaintType == FLATPAINT) {
        paintSetting = SPtrCreate(PaintSetting)(isFill, ColorSetting());
    } else if(currentPaintType == GRADIENTPAINT) {
        paintSetting = SPtrCreate(PaintSetting)(
                    isFill, currentGradientLinear, currentGradient);
    } else{
        paintSetting = SPtrCreate(PaintSetting)(isFill);
    }
    mCanvasWindow->applyPaintSettingToSelected(paintSetting.get());

    mMainWindow->queScheduledTasksAndUpdate();
}

void FillStrokeSettingsWidget::colorSettingReceived(
        const ColorSetting &colorSetting) {
    bool isFill;
    PaintType currentPaintType;
    Gradient *currentGradient;
    bool currentGradientLinear;
    if(mTargetId == 0) {
        isFill = true;
        currentPaintType = mCurrentFillPaintType;
        currentGradient = mCurrentFillGradient;
        currentGradientLinear = mCurrentFillGradientLinear;
    } else {
        isFill = false;
        currentPaintType = mCurrentStrokePaintType;
        currentGradient = mCurrentStrokeGradient;
        currentGradientLinear = mCurrentStrokeGradientLinear;
    }
    stdsptr<PaintSetting> paintSetting;
    if(currentPaintType == FLATPAINT) {
        paintSetting = SPtrCreate(PaintSetting)(isFill, colorSetting);
    } else { // GRADIENTPAINT
        paintSetting = SPtrCreate(PaintSetting)(
                    isFill, currentGradientLinear, currentGradient);
    }
    mCanvasWindow->applyPaintSettingToSelected(paintSetting.get());
}

void FillStrokeSettingsWidget::connectGradient() {
    connect(mGradientWidget,
            SIGNAL(selectedColorChanged(ColorAnimator*)),
            mColorsSettingsWidget,
            SLOT(setColorAnimatorTarget(ColorAnimator*)) );
    connect(mGradientWidget,
            SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );

}

void FillStrokeSettingsWidget::disconnectGradient() {
    disconnect(mGradientWidget,
            SIGNAL(selectedColorChanged(ColorAnimator*)),
            mColorsSettingsWidget,
            SLOT(setColorAnimatorTarget(ColorAnimator*) ) );
    disconnect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
               this, SLOT(setGradient(Gradient*)) );
}

void FillStrokeSettingsWidget::setColorAnimatorTarget(
        ColorAnimator *animator) {
    mColorsSettingsWidget->setColorAnimatorTarget(animator);
}

void FillStrokeSettingsWidget::setJoinStyle(Qt::PenJoinStyle joinStyle)
{
    mCurrentJoinStyle = joinStyle;
    mBevelJoinStyleButton->setChecked(joinStyle == Qt::BevelJoin);
    mMiterJointStyleButton->setChecked(joinStyle == Qt::MiterJoin);
    mRoundJoinStyleButton->setChecked(joinStyle == Qt::RoundJoin);
}

void FillStrokeSettingsWidget::setCapStyle(Qt::PenCapStyle capStyle)
{
    mCurrentCapStyle = capStyle;
    mFlatCapStyleButton->setChecked(capStyle == Qt::FlatCap);
    mSquareCapStyleButton->setChecked(capStyle == Qt::SquareCap);
    mRoundCapStyleButton->setChecked(capStyle == Qt::RoundCap);
}

PaintType FillStrokeSettingsWidget::getCurrentPaintTypeVal() {
    if(mTargetId == 0) {
        return mCurrentFillPaintType;
    } else {
        return mCurrentStrokePaintType;
    }
}

void FillStrokeSettingsWidget::setCurrentPaintTypeVal(const PaintType& paintType) {
    if(mTargetId == 0) {
        mCurrentFillPaintType = paintType;
    } else {
        mCurrentStrokePaintType = paintType;
    }
}

QColor FillStrokeSettingsWidget::getCurrentColorVal() {
    if(mTargetId == 0) {
        return mCurrentFillColor;
    } else {
        return mCurrentStrokeColor;
    }
}

void FillStrokeSettingsWidget::setCurrentColorVal(const QColor& color) {
    if(mTargetId == 0) {
        mCurrentFillColor = color;
    } else {
        mCurrentStrokeColor = color;
    }
}

Gradient *FillStrokeSettingsWidget::getCurrentGradientVal() {
    if(mTargetId == 0) {
        return mCurrentFillGradient;
    } else {
        return mCurrentStrokeGradient;
    }
}

void FillStrokeSettingsWidget::setCurrentGradientVal(Gradient *gradient) {
    if(mTargetId == 0) {
        mCurrentFillGradient = gradient;
    } else {
        mCurrentStrokeGradient = gradient;
    }
}

void FillStrokeSettingsWidget::setCurrentGradientLinearVal(
                                const bool &linear) {
    if(mTargetId == 0) {
        mCurrentFillGradientLinear = linear;
    } else {
        mCurrentStrokeGradientLinear = linear;
    }
}

void FillStrokeSettingsWidget::setFillValuesFromFillSettings(
        PaintSettings *settings) {
    if(settings == nullptr) {
        mCurrentFillColorAnimator = nullptr;
    } else {
        mCurrentFillGradientLinear = settings->getGradientLinear();
        mCurrentFillColor = settings->getCurrentColor();
        mCurrentFillColorAnimator = settings->getColorAnimator();
        mCurrentFillGradient = settings->getGradient();
        mCurrentFillPaintType = settings->getPaintType();
    }
}

void FillStrokeSettingsWidget::setStrokeValuesFromStrokeSettings(
        StrokeSettings *settings) {
    if(settings == nullptr) {
        mCurrentStrokeColorAnimator = nullptr;
        mLineWidthSpin->clearAnimator();
    } else {
        mCurrentStrokeGradientLinear = settings->getGradientLinear();
        mCurrentStrokeColor = settings->getCurrentColor();
        mCurrentStrokeColorAnimator = settings->getColorAnimator();
        mCurrentStrokeGradient = settings->getGradient();
        mCurrentStrokePaintType = settings->getPaintType();
        mCurrentStrokeWidth = settings->getCurrentStrokeWidth();
        mLineWidthSpin->setAnimator(settings->getLineWidthAnimator());
        mCurrentCapStyle = settings->getCapStyle();
        mCurrentJoinStyle = settings->getJoinStyle();
    }
}

void FillStrokeSettingsWidget::setCanvasWindowPtr(CanvasWindow *canvasWidget) {
    mCanvasWindow = canvasWidget;
    connect(mLineWidthSpin, SIGNAL(editingStarted(qreal)),
            mCanvasWindow, SLOT(startSelectedStrokeWidthTransform()));
}

void FillStrokeSettingsWidget::emitStrokeWidthChanged() {
    mCanvasWindow->strokeWidthChanged(mCurrentStrokeWidth, true);
}

void FillStrokeSettingsWidget::emitStrokeWidthChangedTMP() {
    mCanvasWindow->strokeWidthChanged(mCurrentStrokeWidth, false);
}

void FillStrokeSettingsWidget::emitCapStyleChanged() {
    mCanvasWindow->strokeCapStyleChanged(mCurrentCapStyle);
}

void FillStrokeSettingsWidget::emitJoinStyleChanged() {
    mCanvasWindow->strokeJoinStyleChanged(mCurrentJoinStyle);
}

void FillStrokeSettingsWidget::finishTransform()
{
    if(mTransormStarted) {
        mTransormStarted = false;
    }
}

void FillStrokeSettingsWidget::startTransform(const char *slot)
{
    if(!mTransormStarted) {
        mTransormStarted = true;
        setTransformFinishEmitter(slot);
    }
    waitToSaveChanges();
    /*if(mCurrentPaintType == GRADIENTPAINT) {
        mGradientWidget->startGradientTransform();
    } else {
        if(mTargetId == 0) {
            emit startFillSettingsTransform();
        } else {
            emit startStrokeSettingsTransform();
        }
    }*/
}

void FillStrokeSettingsWidget::applyGradient() {
    bool isFill;
    Gradient *currentGradient;
    bool currentGradientLinear;
    if(mTargetId == 0) {
        isFill = true;
        currentGradient = mCurrentFillGradient;
        currentGradientLinear = mCurrentFillGradientLinear;
    } else {
        isFill = false;
        currentGradient = mCurrentStrokeGradient;
        currentGradientLinear = mCurrentStrokeGradientLinear;
    }
    stdsptr<PaintSetting> paintSetting = SPtrCreate(PaintSetting)(
                isFill, currentGradientLinear, currentGradient);
    mCanvasWindow->applyPaintSettingToSelected(paintSetting.get());
}

void FillStrokeSettingsWidget::setGradient(Gradient *gradient) {
    setCurrentGradientVal(gradient);
    applyGradient();
}

void FillStrokeSettingsWidget::setGradientLinear(const bool &linear) {
    setCurrentGradientLinearVal(linear);
    mLinearGradientButton->setChecked(linear);
    mRadialGradientButton->setChecked(!linear);
    applyGradient();
}

void FillStrokeSettingsWidget::setBevelJoinStyle() {
    setJoinStyle(Qt::BevelJoin);
    emitJoinStyleChanged();
}

void FillStrokeSettingsWidget::setMiterJoinStyle() {
    setJoinStyle(Qt::MiterJoin);
    emitJoinStyleChanged();
}

void FillStrokeSettingsWidget::setRoundJoinStyle() {
    setJoinStyle(Qt::RoundJoin);
    emitJoinStyleChanged();
}

void FillStrokeSettingsWidget::setFlatCapStyle() {
    setCapStyle(Qt::FlatCap);
    emitCapStyleChanged();
}

void FillStrokeSettingsWidget::setSquareCapStyle() {
    setCapStyle(Qt::SquareCap);
    emitCapStyleChanged();
}

void FillStrokeSettingsWidget::setRoundCapStyle() {
    setCapStyle(Qt::RoundCap);
    emitCapStyleChanged();
}

void FillStrokeSettingsWidget::waitToSaveChanges() {
    if(mUndoRedoSaveTimer->isActive()) {
        mUndoRedoSaveTimer->setInterval(50);
        return;
    }
    mUndoRedoSaveTimer->stop();
    mUndoRedoSaveTimer->setInterval(50);
    mUndoRedoSaveTimer->setSingleShot(true);
    mUndoRedoSaveTimer->start();
}

void FillStrokeSettingsWidget::setFillTarget() {
    mTargetId = 0;
    mFillTargetButton->setChecked(true);
    mStrokeTargetButton->setChecked(false);
    mStrokeSettingsWidget->hide();
    updateAfterTargetChanged();
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setStrokeTarget() {
    mTargetId = 1;
    mStrokeTargetButton->setChecked(true);
    mFillTargetButton->setChecked(false);
    mStrokeSettingsWidget->show();
    updateAfterTargetChanged();
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setNoPaintType() {
    setCurrentPaintTypeVal(NOPAINT);
    mColorsSettingsWidget->hide();
    mGradientWidget->hide();
    mGradientTypeWidget->hide();
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setFlatPaintType() {
    disconnectGradient();
    mColorsSettingsWidget->show();
    mGradientWidget->hide();
    mGradientTypeWidget->hide();
    setCurrentPaintTypeVal(FLATPAINT);
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setGradientPaintType() {
    connectGradient();
    if(mTargetId == 0) {
        mCurrentFillPaintType = GRADIENTPAINT;
        if(mCurrentFillGradient == nullptr) {
            mCurrentFillGradient =
                    mGradientWidget->getCurrentGradient();
        }
    } else {
        mCurrentStrokePaintType = GRADIENTPAINT;
        if(mCurrentStrokeGradient == nullptr) {
            mCurrentStrokeGradient =
                    mGradientWidget->getCurrentGradient();
        }
    }
    if(mColorsSettingsWidget->isHidden()) {
        mColorsSettingsWidget->show();
    }
    if(mGradientWidget->isHidden()) {
        mGradientWidget->show();
    }
    if(mGradientTypeWidget->isHidden()) {
        mGradientTypeWidget->show();
    }
    updateColorAnimator();

    mGradientWidget->update();
}
