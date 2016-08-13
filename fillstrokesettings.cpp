#include "fillstrokesettings.h"
#include "Colors/ColorWidgets/gradientwidget.h"

void Gradient::updateQGradientStops() {
    qgradientStops.clear();
    qreal inc = 1.f/(colors.length() - 1.f);
    qreal cPos = 0.f;
    for(int i = 0; i < colors.length(); i++) {
        qgradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0.f, 1.f), colors.at(i).qcol) );
        cPos += inc;
    }
}

FillStrokeSettingsWidget::FillStrokeSettingsWidget(QWidget *parent) : QWidget(parent)
{
    mGradientWidget = new GradientWidget(this);
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
    mMainLayout->addWidget(mGradientWidget);
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

    setFillTarget();
}

void FillStrokeSettingsWidget::setCurrentDisplayedSettings(PaintSettings settings) {
    mGradientWidget->setCurrentGradient(settings.gradient);
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
    disconnect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );
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
    connect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );
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

void FillStrokeSettingsWidget::flatColorSet(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    Color newColor;
    newColor.setHSV(h, s, v, a);
    getCurrentTargetPaintSettings()->color = newColor;
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::gradientSet(Gradient *gradient)
{
    getCurrentTargetPaintSettings()->gradient = gradient;
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::gradientChanged()
{
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::connectGradient()
{
    disconnect(mColorsSettingsWidget,
            SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
            this, SLOT(flatColorSet(GLfloat,GLfloat,GLfloat,GLfloat)) );

    connect(mGradientWidget,
            SIGNAL(selectedColorChanged(GLfloat,GLfloat,GLfloat,GLfloat)),
            mColorsSettingsWidget,
            SLOT(setCurrentColor(GLfloat,GLfloat,GLfloat,GLfloat) ) );
    connect(mColorsSettingsWidget,
            SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
            mGradientWidget,
            SLOT(setCurrentColor(GLfloat,GLfloat,GLfloat,GLfloat)) );
    connect(mGradientWidget, SIGNAL(gradientSettingsChanged()),
            this, SLOT(emitTargetSettingsChanged()) );
    connect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );
}

void FillStrokeSettingsWidget::disconnectGradient()
{
    disconnect(mGradientWidget,
            SIGNAL(selectedColorChanged(GLfloat,GLfloat,GLfloat,GLfloat)),
            mColorsSettingsWidget,
            SLOT(setCurrentColor(GLfloat,GLfloat,GLfloat,GLfloat) ) );
    disconnect(mColorsSettingsWidget,
            SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
            mGradientWidget,
            SLOT(setCurrentColor(GLfloat,GLfloat,GLfloat,GLfloat)) );
    disconnect(mGradientWidget, SIGNAL(gradientSettingsChanged()),
            this, SLOT(emitTargetSettingsChanged()) );
    disconnect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );

    connect(mColorsSettingsWidget,
            SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
            this, SLOT(flatColorSet(GLfloat,GLfloat,GLfloat,GLfloat)) );
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

void FillStrokeSettingsWidget::setGradient(Gradient *gradient)
{
    getCurrentTargetPaintSettings()->gradient = gradient;
    emitTargetSettingsChanged();
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
    mGradientWidget->hide();
}

void FillStrokeSettingsWidget::setFlatPaintType()
{
    disconnectGradient();
    mColorsSettingsWidget->show();
    mGradientWidget->hide();
    mColorsSettingsWidget->setCurrentColor(getCurrentTargetPaintSettings()->color);
    getCurrentTargetPaintSettings()->paintType = FLATPAINT;
}

void FillStrokeSettingsWidget::setGradientPaintType()
{
    connectGradient();
    if(getCurrentTargetPaintSettings()->gradient == NULL) {
        getCurrentTargetPaintSettings()->gradient = mGradientWidget->getCurrentGradient();
    }
    mColorsSettingsWidget->show();
    mGradientWidget->show();
    mColorsSettingsWidget->setCurrentColor(mGradientWidget->getCurrentColor());

    getCurrentTargetPaintSettings()->paintType = GRADIENTPAINT;
}
