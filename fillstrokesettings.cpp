#include "fillstrokesettings.h"

FillStrokeSettingsWidget::FillStrokeSettingsWidget(QWidget *parent) : QWidget(parent)
{
    mColorTypeBar = new QTabBar(this);
    mStrokeSettingsWidget = new QWidget(this);
    mColorsSettingsWidget = new ColorSettingsWidget(this);
    mFillTargetButton = new QPushButton("Fill", this);
    mStrokeTargetButton = new QPushButton("Stroke", this);
    setLayout(mMainLayout);
    mMainLayout->setAlignment(Qt::AlignTop);

    mColorTypeBar->addTab("No");
    mColorTypeBar->addTab("Flat");
    mColorTypeBar->addTab("Gradient");

    mFillTargetButton->setCheckable(true);
    mStrokeTargetButton->setCheckable(true);
    mTargetLayout->addWidget(mFillTargetButton);
    mTargetLayout->addWidget(mStrokeTargetButton);

    mMainLayout->addLayout(mTargetLayout);
    mMainLayout->addWidget(mColorTypeBar);
    mMainLayout->addWidget(mColorsSettingsWidget);

    mLineWidthSpin = new QDoubleSpinBox(this);
    mLineWidthSpin->setRange(0.0, 1000.0);
    mLineWidthSpin->setSuffix(" px");
    mLineWidthSpin->setSingleStep(0.1);
    mLineWidthLayout->addWidget(mLineWidthLabel);
    mLineWidthLayout->addWidget(mLineWidthSpin);

    mStrokeSettingsLayout->addLayout(mLineWidthLayout);

    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));

    mStrokeSettingsWidget->setLayout(mStrokeSettingsLayout);
    mMainLayout->addWidget(mStrokeSettingsWidget);

    connect(mFillTargetButton, SIGNAL(released()),
            this, SLOT(setFillTarget()) );
    connect(mStrokeTargetButton, SIGNAL(released()),
            this, SLOT(setStrokeTarget()) );
    connect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );

    connect(mColorsSettingsWidget,
            SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
            this, SLOT(colorSet(GLfloat,GLfloat,GLfloat,GLfloat)) );

    setFillTarget();
}

void FillStrokeSettingsWidget::setCurrentDisplayedSettings(PaintSettings settings) {
    mColorsSettingsWidget->setCurrentColor(settings.color);
    setCurrentPaintType(settings.paintType);
    mColorTypeBar->setCurrentIndex(settings.paintType);
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

void FillStrokeSettingsWidget::setStrokeWidth(qreal width)
{
    mStrokePaintSettings.setLineWidth(width);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setCurrentSettings(PaintSettings fillPaintSettings,
                                            StrokeSettings strokePaintSettings)
{
    mFillPaintSettings = fillPaintSettings;
    mStrokePaintSettings = strokePaintSettings;
    disconnect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
    mLineWidthSpin->setValue(mStrokePaintSettings.lineWidth);
    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
    if(mTargetId == 0) { // fill
        setCurrentDisplayedSettings(mFillPaintSettings);
    } else {
        setCurrentDisplayedSettings(mStrokePaintSettings.paintSettings);
    }
}

void FillStrokeSettingsWidget::colorTypeSet(int id)
{
    if(id == 0) {
        setNoPaintType();
    } else if(id == 1) {
        setFlatPaintType();
    } else {
        setGradientPaintType();
    }
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::colorSet(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    Color newColor;
    newColor.setHSV(h, s, v, a);
    getCurrentTargetPaintSettings()->color = newColor;
    emitTargetSettingsChanged();
}

PaintSettings *FillStrokeSettingsWidget::getCurrentTargetPaintSettings()
{
    if(mTargetId == 0) {
        return &mFillPaintSettings;
    } else {
        return &mStrokePaintSettings.paintSettings;
    }
}

void FillStrokeSettingsWidget::emitTargetSettingsChanged()
{
    if(mTargetId == 0) {
        emit fillSettingsChanged(mFillPaintSettings);
    } else {
        emit strokeSettingsChanged(mStrokePaintSettings);
    }
}

void FillStrokeSettingsWidget::setFillTarget()
{
    mTargetId = 0;
    mFillTargetButton->setChecked(true);
    mStrokeTargetButton->setChecked(false);
    mStrokeSettingsWidget->hide();
    setCurrentDisplayedSettings(mFillPaintSettings);
}

void FillStrokeSettingsWidget::setStrokeTarget()
{
    mTargetId = 1;
    mStrokeTargetButton->setChecked(true);
    mFillTargetButton->setChecked(false);
    mStrokeSettingsWidget->show();
    setCurrentDisplayedSettings(mStrokePaintSettings.paintSettings);
}

void FillStrokeSettingsWidget::setNoPaintType()
{
    getCurrentTargetPaintSettings()->paintType = NOPAINT;
    mColorsSettingsWidget->hide();
}

void FillStrokeSettingsWidget::setFlatPaintType()
{
    mColorsSettingsWidget->show();
    getCurrentTargetPaintSettings()->paintType = FLATPAINT;
}

void FillStrokeSettingsWidget::setGradientPaintType()
{
    mColorsSettingsWidget->show();
    getCurrentTargetPaintSettings()->paintType = GRADIENTPAINT;
}
