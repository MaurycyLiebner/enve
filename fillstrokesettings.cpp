#include "fillstrokesettings.h"
#include "Colors/ColorWidgets/gradientwidget.h"
#include "mainwindow.h"
#include "undoredo.h"
#include "canvas.h"
#include "updatescheduler.h"
#include "qrealanimatorvalueslider.h"

#include "qdoubleslider.h"
FillStrokeSettingsWidget::FillStrokeSettingsWidget(MainWindow *parent) :
    QWidget(parent)
{
    //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mMainWindow = parent;
    mUndoRedoSaveTimer = new QTimer(this);

    mGradientWidget = new GradientWidget(this, mMainWindow);
    mStrokeSettingsWidget = new QWidget(this);
    mColorsSettingsWidget = new ColorSettingsWidget(this);
    mFillTargetButton = new QPushButton(
                QIcon(":/icons/properties_fill.png"),
                "Fill", this);
    mStrokeTargetButton = new QPushButton(
                QIcon(":/icons/properties_stroke_paint.png"),
                "Stroke", this);
    setLayout(mMainLayout);
    mMainLayout->setAlignment(Qt::AlignTop);

    mColorTypeLayout = new QHBoxLayout();
    mColorTypeLayout->setAlignment(Qt::AlignLeft);
    mFillNoneButton = new ActionButton(
                ":/icons/fill_none.png",
                "", this);
    mFillNoneButton->setCheckable(":icons/fill_none_checked.png");
    connect(mFillNoneButton, SIGNAL(pressed()),
            this, SLOT(setNoneFill()));
    mFillFlatButton = new ActionButton(
                ":/icons/fill_flat.png",
                "", this);
    mFillFlatButton->setCheckable(":icons/fill_flat_checked.png");
    connect(mFillFlatButton, SIGNAL(pressed()),
            this, SLOT(setFlatFill()));
    mFillGradientButton = new ActionButton(
                ":/icons/fill_gradient.png",
                "", this);
    mFillGradientButton->setCheckable(":icons/fill_gradient_checked.png");
    connect(mFillGradientButton, SIGNAL(pressed()),
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

    mMainLayout->addLayout(mTargetLayout);
    mMainLayout->addLayout(mColorTypeLayout);
    mMainLayout->addWidget(mStrokeSettingsWidget);
    mMainLayout->addWidget(mGradientWidget);
    mMainLayout->addWidget(mColorsSettingsWidget);


    mBevelJoinStyleButton = new QPushButton(QIcon(":/icons/join_bevel.png"), "", this);
    mMiterJointStyleButton = new QPushButton(QIcon(":/icons/join_miter.png"), "", this);
    mRoundJoinStyleButton = new QPushButton(QIcon(":/icons/join_round.png"), "", this);
    mBevelJoinStyleButton->setCheckable(true);
    mMiterJointStyleButton->setCheckable(true);
    mRoundJoinStyleButton->setCheckable(true);
    connect(mBevelJoinStyleButton, SIGNAL(released()),
            this, SLOT(setBevelJoinStyle()) );
    connect(mMiterJointStyleButton, SIGNAL(released()),
            this, SLOT(setMiterJoinStyle()) );
    connect(mRoundJoinStyleButton, SIGNAL(released()),
            this, SLOT(setRoundJoinStyle()) );
    mJoinStyleLayout->addWidget(mBevelJoinStyleButton);
    mJoinStyleLayout->addWidget(mMiterJointStyleButton);
    mJoinStyleLayout->addWidget(mRoundJoinStyleButton);

    mStrokeSettingsLayout->addLayout(mJoinStyleLayout);

    mFlatCapStyleButton = new QPushButton(QIcon(":/icons/cap_flat.png"), "", this);
    mSquareCapStyleButton = new QPushButton(QIcon(":/icons/cap_square.png"), "", this);
    mRoundCapStyleButton = new QPushButton(QIcon(":/icons/cap_round.png"), "", this);
    mFlatCapStyleButton->setCheckable(true);
    mSquareCapStyleButton->setCheckable(true);
    mRoundCapStyleButton->setCheckable(true);
    connect(mFlatCapStyleButton, SIGNAL(released()),
            this, SLOT(setFlatCapStyle()) );
    connect(mSquareCapStyleButton, SIGNAL(released()),
            this, SLOT(setSquareCapStyle()) );
    connect(mRoundCapStyleButton, SIGNAL(released()),
            this, SLOT(setRoundCapStyle()) );
    mCapStyleLayout->addWidget(mFlatCapStyleButton);
    mCapStyleLayout->addWidget(mSquareCapStyleButton);
    mCapStyleLayout->addWidget(mRoundCapStyleButton);

    mStrokeSettingsLayout->addLayout(mCapStyleLayout);

    //mLineWidthSpin = new QDoubleSpinBox(this);
    mLineWidthSpin = new QrealAnimatorValueSlider("line width", 0., 1000., 1., this);
    mLineWidthSpin->setNameVisible(false);
    //mLineWidthSpin->setValueSliderVisibile(false);
    //mLineWidthSpin->setRange(0.0, 1000.0);
    //mLineWidthSpin->setSuffix(" px");
    //mLineWidthSpin->setSingleStep(0.1);
    mLineWidthLayout->addWidget(mLineWidthLabel);
    mLineWidthLayout->addWidget(mLineWidthSpin);

    mStrokeSettingsLayout->addLayout(mLineWidthLayout);

    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));

    mStrokeSettingsWidget->setLayout(mStrokeSettingsLayout);

    connect(mFillTargetButton, SIGNAL(released()),
            this, SLOT(setFillTarget()) );
    connect(mStrokeTargetButton, SIGNAL(released()),
            this, SLOT(setStrokeTarget()) );

    connect(mColorsSettingsWidget,
            SIGNAL(colorSettingSignal(ColorSetting)),
            this, SLOT(colorSettingReceived(ColorSetting)));

    mFillPickerButton = new QPushButton(
                QIcon(":/icons/fill_dropper.png"), "", this);
    mFillPickerButton->setSizePolicy(QSizePolicy::Maximum,
                                     QSizePolicy::Maximum);
    mFillPickerButton->setFocusPolicy(Qt::NoFocus);
    mStrokePickerButton = new QPushButton(
                QIcon(":/icons/stroke_dropper.png"), "", this);
    mStrokePickerButton->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    mStrokePickerButton->setFocusPolicy(Qt::NoFocus);
    mFillStrokePickerButton = new QPushButton(
                QIcon(":/icons/fill_stroke_dropper.png"), "", this);
    mFillStrokePickerButton->setSizePolicy(QSizePolicy::Maximum,
                                           QSizePolicy::Maximum);
    mFillStrokePickerButton->setFocusPolicy(Qt::NoFocus);
    mPickersLayout->addWidget(mFillPickerButton);

    connect(mFillPickerButton, SIGNAL(pressed()),
            this, SLOT(startLoadingFillFromPath()) );

    mPickersLayout->setAlignment(Qt::AlignLeft);
    mPickersLayout->addWidget(mStrokePickerButton);
    connect(mStrokePickerButton, SIGNAL(pressed()),
            this, SLOT(startLoadingStrokeFromPath()) );
    mPickersLayout->addWidget(mFillStrokePickerButton);
    connect(mFillStrokePickerButton, SIGNAL(pressed()),
            this, SLOT(startLoadingSettingsFromPath()) );
    mMainLayout->addLayout(mPickersLayout);

    setFillTarget();
    setCapStyle(Qt::RoundCap);
    setJoinStyle(Qt::RoundJoin);

    connect(mColorsSettingsWidget, SIGNAL(colorModeChanged(ColorMode)),
            this, SLOT(setCurrentColorMode(ColorMode)));
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

void FillStrokeSettingsWidget::saveGradientsToSqlIfPathSelected(QSqlQuery *query) {
    mGradientWidget->saveGradientsToSqlIfPathSelected(query);
}

void FillStrokeSettingsWidget::updateColorAnimator() {
    if(getCurrentPaintTypeVal() == 0) {
        setColorAnimatorTarget(NULL);
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
            mCanvasWidget->setSelectedFillColorMode(mode);
        }
    } else {
        if(mCurrentStrokePaintType == FLATPAINT) {
            mCanvasWidget->setSelectedStrokeColorMode(mode);
        }
    }
}

void FillStrokeSettingsWidget::updateAfterTargetChanged() {
    if(getCurrentPaintTypeVal() == GRADIENTPAINT) {
        mGradientWidget->setCurrentGradient(getCurrentGradientVal() );
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

void FillStrokeSettingsWidget::setCurrentPaintType(PaintType paintType)
{
    if(paintType == NOPAINT) {
        setNoPaintType();
    } else if (paintType == FLATPAINT) {
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

void FillStrokeSettingsWidget::setStrokeWidth(qreal width)
{
    mCanvasWidget->startSelectedStrokeWidthTransform();
    startTransform(SLOT(emitStrokeWidthChanged()));
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

void FillStrokeSettingsWidget::saveGradientsToQuery(QSqlQuery *query)
{
    mGradientWidget->saveGradientsToQuery(query);
}

GradientWidget *FillStrokeSettingsWidget::getGradientWidget() {
    return mGradientWidget;
}

void FillStrokeSettingsWidget::clearAll()
{
    mGradientWidget->clearAll();
}

void FillStrokeSettingsWidget::colorTypeSet(int id)
{
    if(id == 0) {
        setNoPaintType();
    } else if(id == 1) {
        setFlatPaintType();
    } else {
        if((mTargetId == 0) ? (mCurrentFillGradient == NULL) :
                (mCurrentStrokeGradient == NULL) ) {
            mGradientWidget->setCurrentGradient((Gradient*)NULL);
        }
        setGradientPaintType();
    }
    if(mTargetId == 0) {
        if(mCurrentFillPaintType == FLATPAINT) {
            PaintSetting paintSetting =
                    PaintSetting(true, ColorSetting());
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        } else if(mCurrentFillPaintType == GRADIENTPAINT) {
            PaintSetting paintSetting =
                    PaintSetting(true, mCurrentFillGradient);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        } else{
            PaintSetting paintSetting =
                    PaintSetting(true);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        }
    } else {
        if(mCurrentStrokePaintType == FLATPAINT) {
            PaintSetting paintSetting =
                    PaintSetting(false, ColorSetting());
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        } else if(mCurrentStrokePaintType == GRADIENTPAINT) {
            PaintSetting paintSetting =
                    PaintSetting(false, mCurrentStrokeGradient);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        } else{
            PaintSetting paintSetting =
                    PaintSetting(false);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        }
    }

    mMainWindow->callUpdateSchedulers();
}

void FillStrokeSettingsWidget::colorSettingReceived(
        const ColorSetting &colorSetting) {
    if(mTargetId == 0) {
        if(mCurrentFillPaintType == FLATPAINT) {
            PaintSetting paintSetting =
                    PaintSetting(true, colorSetting);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        } else {
            PaintSetting paintSetting =
                    PaintSetting(true, mCurrentFillGradient);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        }
    } else {
        if(mCurrentStrokePaintType == FLATPAINT) {
            PaintSetting paintSetting =
                    PaintSetting(false, colorSetting);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        } else {
            PaintSetting paintSetting =
                    PaintSetting(false, mCurrentStrokeGradient);
            mCanvasWidget->applyPaintSettingToSelected(paintSetting);
        }
    }
}

void FillStrokeSettingsWidget::connectGradient()
{
    connect(mGradientWidget,
            SIGNAL(selectedColorChanged(ColorAnimator*)),
            mColorsSettingsWidget,
            SLOT(setColorAnimatorTarget(ColorAnimator*)) );
    connect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );
}

void FillStrokeSettingsWidget::disconnectGradient()
{
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

void FillStrokeSettingsWidget::setFillValuesFromFillSettings(
        PaintSettings *settings) {
    if(settings == NULL) {
        mCurrentFillColorAnimator = NULL;
    } else {
        mCurrentFillColor = settings->getCurrentColor();
        mCurrentFillColorAnimator = settings->getColorAnimator();
        mCurrentFillGradient = settings->getGradient();
        mCurrentFillPaintType = settings->getPaintType();
    }
}

void FillStrokeSettingsWidget::setStrokeValuesFromStrokeSettings(
        StrokeSettings *settings) {
    if(settings == NULL) {
        mCurrentStrokeColorAnimator = NULL;
        mLineWidthSpin->setAnimator(NULL);
    } else {
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

void FillStrokeSettingsWidget::setCanvasWidgetPtr(CanvasWidget *canvasWidget)
{
    mCanvasWidget = canvasWidget;
}

void FillStrokeSettingsWidget::loadSettingsFromPath(PathBox *path) {
    if(mLoadFillFromPath) {
        mLoadFillFromPath = false;
        setFillValuesFromFillSettings(path->getFillSettings());

    }
    if(mLoadStrokeFromPath) {
        mLoadStrokeFromPath = false;
        setStrokeValuesFromStrokeSettings(path->getStrokeSettings());

        mCanvasWidget->strokeCapStyleChanged(mCurrentCapStyle);
        mCanvasWidget->strokeJoinStyleChanged(mCurrentJoinStyle);
        mCanvasWidget->strokeWidthChanged(mCurrentStrokeWidth, true);
    }
}

void FillStrokeSettingsWidget::emitStrokeWidthChanged() {
    mCanvasWidget->strokeWidthChanged(mCurrentStrokeWidth, true);
}

void FillStrokeSettingsWidget::emitStrokeWidthChangedTMP() {
    mCanvasWidget->strokeWidthChanged(mCurrentStrokeWidth, false);
}

void FillStrokeSettingsWidget::emitCapStyleChanged() {
    mCanvasWidget->strokeCapStyleChanged(mCurrentCapStyle);
}

void FillStrokeSettingsWidget::emitJoinStyleChanged() {
    mCanvasWidget->strokeJoinStyleChanged(mCurrentJoinStyle);
}

void FillStrokeSettingsWidget::startLoadingFillFromPath()
{
    mLoadFillFromPath = true;
    mCanvasWidget->pickPathForSettings();
}

void FillStrokeSettingsWidget::startLoadingStrokeFromPath()
{
    mLoadStrokeFromPath = true;
    mCanvasWidget->pickPathForSettings();
}

void FillStrokeSettingsWidget::startLoadingSettingsFromPath()
{
    mLoadFillFromPath = true;
    mLoadStrokeFromPath = true;
    mCanvasWidget->pickPathForSettings();
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

void FillStrokeSettingsWidget::setGradient(Gradient *gradient)
{
    setCurrentGradientVal(gradient);
    if(mTargetId == 0) {
        PaintSetting paintSetting =
                PaintSetting(true, mCurrentFillGradient);
        mCanvasWidget->applyPaintSettingToSelected(paintSetting);
    } else {
        PaintSetting paintSetting =
                PaintSetting(false, mCurrentStrokeGradient);
        mCanvasWidget->applyPaintSettingToSelected(paintSetting);
    }
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

void FillStrokeSettingsWidget::waitToSaveChanges()
{
    if(mUndoRedoSaveTimer->isActive()) {
        mUndoRedoSaveTimer->setInterval(50);
        return;
    }
    mUndoRedoSaveTimer->stop();
    mUndoRedoSaveTimer->setInterval(50);
    mUndoRedoSaveTimer->setSingleShot(true);
    mUndoRedoSaveTimer->start();
}

void FillStrokeSettingsWidget::setFillTarget()
{
    mTargetId = 0;
    mFillTargetButton->setChecked(true);
    mStrokeTargetButton->setChecked(false);
    mStrokeSettingsWidget->hide();
    updateAfterTargetChanged();
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setStrokeTarget()
{
    mTargetId = 1;
    mStrokeTargetButton->setChecked(true);
    mFillTargetButton->setChecked(false);
    mStrokeSettingsWidget->show();
    updateAfterTargetChanged();
    updateColorAnimator();

}

void FillStrokeSettingsWidget::setNoPaintType()
{
    setCurrentPaintTypeVal(NOPAINT);
    mColorsSettingsWidget->hide();
    mGradientWidget->hide();
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setFlatPaintType()
{
    disconnectGradient();
    mColorsSettingsWidget->show();
    mGradientWidget->hide();
    setCurrentPaintTypeVal(FLATPAINT);
    updateColorAnimator();
}

void FillStrokeSettingsWidget::setGradientPaintType()
{
    connectGradient();
    if(mTargetId == 0) {
        mCurrentFillPaintType = GRADIENTPAINT;
        if(mCurrentFillGradient == NULL) {
            mCurrentFillGradient = mGradientWidget->getCurrentGradient();
        }
    } else {
        mCurrentStrokePaintType = GRADIENTPAINT;
        if(mCurrentStrokeGradient == NULL) {
            mCurrentStrokeGradient = mGradientWidget->getCurrentGradient();
        }
    }
    mColorsSettingsWidget->show();
    mGradientWidget->show();
    updateColorAnimator();

    mGradientWidget->update();
}
