#include "fillstrokesettings.h"
#include "Colors/ColorWidgets/gradientwidget.h"
#include "mainwindow.h"

void Gradient::updateQGradientStops() {
    qgradientStops.clear();
    qreal inc = 1.f/(colors.length() - 1.f);
    qreal cPos = 0.f;
    for(int i = 0; i < colors.length(); i++) {
        qgradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0.f, 1.f), colors.at(i).qcol) );
        cPos += inc;
    }
}

FillStrokeSettingsWidget::FillStrokeSettingsWidget(MainWindow *parent) : QWidget(parent)
{
    mMainWindow = parent;
    mUndoRedoSaveTimer = new QTimer(this);
    connect(mUndoRedoSaveTimer, SIGNAL(timeout()),
            this, SLOT(finishTransform()) );

    mGradientWidget = new GradientWidget(this);
    mColorTypeBar = new QTabBar(this);
    mStrokeSettingsWidget = new QWidget(this);
    mColorsSettingsWidget = new ColorSettingsWidget(this);
    mFillTargetButton = new QPushButton(QIcon("pixmaps/icons/ink_properties_fill.png"), "Fill", this);
    mStrokeTargetButton = new QPushButton(QIcon("pixmaps/icons/ink_properties_stroke.png"), "Stroke", this);
    setLayout(mMainLayout);
    mMainLayout->setAlignment(Qt::AlignTop);

    mColorTypeBar->addTab(QIcon("pixmaps/icons/ink_fill_none.png"), "No");
    mColorTypeBar->addTab(QIcon("pixmaps/icons/ink_fill_solid.png"), "Flat");
    mColorTypeBar->addTab(QIcon("pixmaps/icons/ink_fill_gradient.png"),"Gradient");

    mFillTargetButton->setCheckable(true);
    mStrokeTargetButton->setCheckable(true);
    mTargetLayout->addWidget(mFillTargetButton);
    mTargetLayout->addWidget(mStrokeTargetButton);

    mMainLayout->addLayout(mTargetLayout);
    mMainLayout->addWidget(mColorTypeBar);
    mMainLayout->addWidget(mStrokeSettingsWidget);
    mMainLayout->addWidget(mGradientWidget);
    mMainLayout->addWidget(mColorsSettingsWidget);


    mBevelJoinStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_join_bevel.png"), "Bevel join", this);
    mMiterJointStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_join_miter.png"), "Miter join", this);
    mRoundJoinStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_join_round.png"), "Round join", this);
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

    mFlatCapStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_cap_flat.png"), "Flat cap", this);
    mSquareCapStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_cap_square.png"), "Square cap", this);
    mRoundCapStyleButton = new QPushButton(QIcon("pixmaps/icons/ink_cap_round.png"), "Round cap", this);
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

    connect(mFillTargetButton, SIGNAL(released()),
            this, SLOT(setFillTarget()) );
    connect(mStrokeTargetButton, SIGNAL(released()),
            this, SLOT(setStrokeTarget()) );
    connect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );

    setFillTarget();
    setCapStyle(Qt::RoundCap);
    setJoinStyle(Qt::RoundJoin);
}

void FillStrokeSettingsWidget::setCurrentDisplayedSettings(PaintSettings *settings) {
    mGradientWidget->setCurrentGradient(settings->gradient);
    setCurrentPaintType(settings->paintType);
    mColorTypeBar->setCurrentIndex(settings->paintType);
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
    emitTargetSettingsChangedTMP();
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
    mLineWidthSpin->setValue(mStrokePaintSettings.lineWidth());
    connect(mLineWidthSpin, SIGNAL(valueChanged(double)),
            this, SLOT(setStrokeWidth(qreal)));
    if(mTargetId == 0) { // fill
        setCurrentDisplayedSettings(&mFillPaintSettings);
    } else {
        setCurrentDisplayedSettings(&mStrokePaintSettings);
    }
    connect(mColorTypeBar, SIGNAL(currentChanged(int)),
            this, SLOT(colorTypeSet(int)) );

    setCapStyle(strokePaintSettings.capStyle());
    setJoinStyle(strokePaintSettings.joinStyle());
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
    emitTargetSettingsChangedTMP();
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
            this, SLOT(emitTargetSettingsChangedTMP()) );
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
            this, SLOT(emitTargetSettingsChangedTMP()) );
    disconnect(mGradientWidget, SIGNAL(currentGradientChanged(Gradient*)),
            this, SLOT(setGradient(Gradient*)) );

    connect(mColorsSettingsWidget,
            SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat,GLfloat)),
            this, SLOT(flatColorSet(GLfloat,GLfloat,GLfloat,GLfloat)) );
}

void FillStrokeSettingsWidget::setJoinStyle(Qt::PenJoinStyle joinStyle)
{
    mStrokePaintSettings.setJoinStyle(joinStyle);
    mBevelJoinStyleButton->setChecked(joinStyle == Qt::BevelJoin);
    mMiterJointStyleButton->setChecked(joinStyle == Qt::MiterJoin);
    mRoundJoinStyleButton->setChecked(joinStyle == Qt::RoundJoin);
}

void FillStrokeSettingsWidget::setCapStyle(Qt::PenCapStyle capStyle)
{
    mStrokePaintSettings.setCapStyle(capStyle);
    mFlatCapStyleButton->setChecked(capStyle == Qt::FlatCap);
    mSquareCapStyleButton->setChecked(capStyle == Qt::SquareCap);
    mRoundCapStyleButton->setChecked(capStyle == Qt::RoundCap);
}

PaintSettings *FillStrokeSettingsWidget::getCurrentTargetPaintSettings()
{
    if(mTargetId == 0) {
        return &mFillPaintSettings;
    } else {
        return &mStrokePaintSettings;
    }
}

void FillStrokeSettingsWidget::emitTargetSettingsChanged()
{
    if(mTargetId == 0) {
        emit fillSettingsChanged(mFillPaintSettings, true);
    } else {
        emit strokeSettingsChanged(mStrokePaintSettings, true);
    }
}

void FillStrokeSettingsWidget::emitTargetSettingsChangedTMP()
{
    startTransform();
    if(mTargetId == 0) {
        emit fillSettingsChanged(mFillPaintSettings, false);
    } else {
        emit strokeSettingsChanged(mStrokePaintSettings, false);
    }
}

void FillStrokeSettingsWidget::finishTransform()
{
    if(mTransormStarted) {
        mTransormStarted = false;
        if(mTargetId == 0) {
            emit finishFillSettingsTransform();
        } else {
            emit finishStrokeSettingsTransform();
        }
    }
}

void FillStrokeSettingsWidget::startTransform()
{
    waitToSaveChanges();
    if(mTransormStarted) return;
    mTransormStarted = true;
    if(mTargetId == 0) {
        emit startFillSettingsTransform();
    } else {
        emit startStrokeSettingsTransform();
    }
}

void FillStrokeSettingsWidget::setGradient(Gradient *gradient)
{
    getCurrentTargetPaintSettings()->gradient = gradient;
    emitTargetSettingsChangedTMP();
}

void FillStrokeSettingsWidget::setBevelJoinStyle()
{
    setJoinStyle(Qt::BevelJoin);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setMiterJoinStyle()
{
    setJoinStyle(Qt::MiterJoin);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setRoundJoinStyle()
{
    setJoinStyle(Qt::RoundJoin);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setFlatCapStyle()
{
    setCapStyle(Qt::FlatCap);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setSquareCapStyle()
{
    setCapStyle(Qt::SquareCap);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::setRoundCapStyle()
{
    setCapStyle(Qt::RoundCap);
    emitTargetSettingsChanged();
}

void FillStrokeSettingsWidget::waitToSaveChanges()
{
    mUndoRedoSaveTimer->stop();
    mUndoRedoSaveTimer->setInterval(250);
    mUndoRedoSaveTimer->setSingleShot(true);
    mUndoRedoSaveTimer->start();
}

void FillStrokeSettingsWidget::setFillTarget()
{
    mTargetId = 0;
    mFillTargetButton->setChecked(true);
    mStrokeTargetButton->setChecked(false);
    mStrokeSettingsWidget->hide();
    setCurrentDisplayedSettings(&mFillPaintSettings);
}

void FillStrokeSettingsWidget::setStrokeTarget()
{
    mTargetId = 1;
    mStrokeTargetButton->setChecked(true);
    mFillTargetButton->setChecked(false);
    mStrokeSettingsWidget->show();
    setCurrentDisplayedSettings(&mStrokePaintSettings);
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

    mGradientWidget->update();
}
