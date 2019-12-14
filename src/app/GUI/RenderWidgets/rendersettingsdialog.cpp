#include "rendersettingsdialog.h"
#include <QPushButton>
#include "GUI/global.h"

RenderSettingsDialog::RenderSettingsDialog(const RenderSettings &settings,
                                           QWidget *parent) :
    QDialog(parent), mInitialSettings(settings) {
    const auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    const auto resolutionLay = new QHBoxLayout;
    mResolutionLabel = new QLabel("Resolution: ");
    mResolutionSpin = new QDoubleSpinBox(this);
    mResolutionSpin->setDecimals(2);
    mResolutionSpin->setSuffix(" %");
    mResolutionSpin->setRange(1, 999.99);
    mResolutionSpin->setValue(settings.fResolution*100);
    resolutionLay->addWidget(mResolutionLabel);
    resolutionLay->addWidget(mResolutionSpin);
    mainLayout->addLayout(resolutionLay);

    const auto dimLay = new QHBoxLayout;
    mWidthLabel = new QLabel("Width :");
    mWidthSpin = new QSpinBox(this);
    mWidthSpin->setRange(1, 9999);
    mWidthSpin->setValue(settings.fVideoWidth);
    dimLay->addWidget(mWidthLabel);
    dimLay->addWidget(mWidthSpin);

    connectDims();

    addSeparator();

    mHeightLabel = new QLabel("Height: ");
    mHeightSpin = new QSpinBox(this);
    mHeightSpin->setRange(1, 9999);
    mHeightSpin->setValue(settings.fVideoHeight);
    dimLay->addWidget(mHeightLabel);
    dimLay->addWidget(mHeightSpin);

    mainLayout->addLayout(dimLay);
    addSeparator();

    const auto rangeLay = new QHBoxLayout;
    mFrameRangeLabel = new QLabel("Frame range: ", this);
    mMinFrameSpin = new QSpinBox(this);
    mMinFrameSpin->setRange(-999999, 999999);
    mMaxFrameSpin = new QSpinBox(this);
    mMaxFrameSpin->setRange(-999999, 999999);

    connect(mMinFrameSpin, qOverload<int>(&QSpinBox::valueChanged),
            mMaxFrameSpin, &QSpinBox::setMinimum);
    connect(mMaxFrameSpin, qOverload<int>(&QSpinBox::valueChanged),
            mMinFrameSpin, &QSpinBox::setMaximum);
    mMinFrameSpin->setValue(settings.fMinFrame);
    mMaxFrameSpin->setValue(settings.fMaxFrame);

    rangeLay->addWidget(mFrameRangeLabel);
    rangeLay->addWidget(mMinFrameSpin);
    rangeLay->addWidget(new QLabel(" - "));
    rangeLay->addWidget(mMaxFrameSpin);

    mainLayout->addLayout(rangeLay);

//    addSeparator();

//    const auto fpsLay = new QHBoxLayout;
//    mFpsLabel = new QLabel("Fps: ", this);
//    mFpsSpin = new QDoubleSpinBox(this);
//    mFpsSpin->setRange(1, 999.99);
//    mFpsSpin->setValue(settings.fFps);
//    fpsLay->addWidget(mFpsLabel);
//    fpsLay->addWidget(mFpsSpin);

//    mainLayout->addLayout(fpsLay);

    mainLayout->addSpacing(MIN_WIDGET_DIM);
    mainLayout->addStretch();

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mResetButton = new QPushButton("Reset", this);
    connect(mOkButton, &QPushButton::released,
            this, &RenderSettingsDialog::accept);
    connect(mCancelButton, &QPushButton::released,
            this, &RenderSettingsDialog::reject);
    connect(mResetButton, &QPushButton::released,
            this, &RenderSettingsDialog::restoreInitialSettings);
    mButtonsLayout->addWidget(mResetButton, Qt::AlignLeft);
    mButtonsLayout->addWidget(mCancelButton, Qt::AlignLeft);
    mButtonsLayout->addWidget(mOkButton, Qt::AlignRight);

    mainLayout->addLayout(mButtonsLayout);
}

RenderSettings RenderSettingsDialog::getSettings() const {
    RenderSettings sett = mInitialSettings;
    sett.fResolution = mResolutionSpin->value()/100;
    sett.fVideoWidth = mWidthSpin->value();
    sett.fVideoHeight = mHeightSpin->value();
    sett.fMinFrame = mMinFrameSpin->value();
    sett.fMaxFrame = mMaxFrameSpin->value();
//    sett.fFps = mFpsSpin->value();
    return sett;
}

void RenderSettingsDialog::connectDims() {
    connect(mResolutionSpin,
            qOverload<qreal>(&QDoubleSpinBox::valueChanged),
            this, &RenderSettingsDialog::updateValuesFromRes);
    connect(mWidthSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &RenderSettingsDialog::updateValuesFromWidth);
    connect(mHeightSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &RenderSettingsDialog::updateValuesFromHeight);
}

void RenderSettingsDialog::disconnectDims() {
    disconnect(mResolutionSpin,
               qOverload<qreal>(&QDoubleSpinBox::valueChanged),
               this, &RenderSettingsDialog::updateValuesFromRes);
    disconnect(mWidthSpin, qOverload<int>(&QSpinBox::valueChanged),
               this, &RenderSettingsDialog::updateValuesFromWidth);
    disconnect(mHeightSpin, qOverload<int>(&QSpinBox::valueChanged),
               this, &RenderSettingsDialog::updateValuesFromHeight);
}

void RenderSettingsDialog::updateValuesFromRes() {
    disconnectDims();
    const qreal res = mResolutionSpin->value()/100;
    mWidthSpin->setValue(qRound(mInitialSettings.fBaseWidth*res));
    mHeightSpin->setValue(qRound(mInitialSettings.fBaseHeight*res));
    connectDims();
}

void RenderSettingsDialog::updateValuesFromWidth() {
    disconnectDims();
    const qreal res = qreal(mWidthSpin->value())/
                      mInitialSettings.fBaseWidth;
    mResolutionSpin->setValue(res*100);
    mHeightSpin->setValue(qRound(mInitialSettings.fBaseHeight*res));
    connectDims();
}

void RenderSettingsDialog::updateValuesFromHeight() {
    disconnectDims();
    const qreal res = qreal(mHeightSpin->value())/
                      mInitialSettings.fBaseHeight;
    mResolutionSpin->setValue(res*100);
    mWidthSpin->setValue(qRound(mInitialSettings.fBaseWidth*res));
    connectDims();
}

void RenderSettingsDialog::restoreInitialSettings() {
    mResolutionSpin->setValue(mInitialSettings.fResolution*100);
    mMinFrameSpin->setValue(mInitialSettings.fMinFrame);
    mMaxFrameSpin->setValue(mInitialSettings.fMaxFrame);
//    mFpsSpin->setValue(mInitialSettings.fFps);
}

void RenderSettingsDialog::addSeparator() {
    const auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout()->addWidget(line);
}
