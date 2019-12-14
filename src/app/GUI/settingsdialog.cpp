#include "settingsdialog.h"
#include "Private/esettings.h"
#include "hardwareinfo.h"
#include "exceptions.h"

#include <QVBoxLayout>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget * const parent) :
    QDialog(parent) {
    setWindowTitle("Settings");

    const auto mainLauout = new QVBoxLayout;
    setLayout(mainLauout);

//    QHBoxLayout* cpuCapSett = new QHBoxLayout;

//    mCpuThreadsCapCheck = new QCheckBox("CPU threads cap", this);
//    mCpuThreadsCapSpin = new QSpinBox(this);
//    mCpuThreadsCapSpin->setRange(1, HardwareInfo::sCpuThreads());
//    connect(mCpuThreadsCapCheck, &QCheckBox::toggled,
//            mCpuThreadsCapSpin, &QWidget::setEnabled);

//    cpuCapSett->addWidget(mCpuThreadsCapCheck);
//    cpuCapSett->addWidget(mCpuThreadsCapSpin);
//    mainLauout->addLayout(cpuCapSett);

    QHBoxLayout* ramCapSett = new QHBoxLayout;

    mRamMBCapCheck = new QCheckBox("Process RAM MB cap", this);
    mRamMBCapSpin = new QSpinBox(this);
    mRamMBCapSpin->setRange(250, intMB(HardwareInfo::sRamKB()).fValue);

    mRamMBCapSlider = new QSlider(Qt::Horizontal);
    mRamMBCapSlider->setRange(250, intMB(HardwareInfo::sRamKB()).fValue);

    connect(mRamMBCapCheck, &QCheckBox::toggled,
            mRamMBCapSpin, &QWidget::setEnabled);
    connect(mRamMBCapCheck, &QCheckBox::toggled,
            mRamMBCapSlider, &QWidget::setEnabled);

    connect(mRamMBCapSpin, qOverload<int>(&QSpinBox::valueChanged),
            mRamMBCapSlider, &QSlider::setValue);
    connect(mRamMBCapSlider, &QSlider::valueChanged,
            mRamMBCapSpin, &QSpinBox::setValue);

    ramCapSett->addWidget(mRamMBCapCheck);
    ramCapSett->addWidget(mRamMBCapSpin);
    mainLauout->addLayout(ramCapSett);
    mainLauout->addWidget(mRamMBCapSlider);

    const auto line0 = new QFrame();
    line0->setFrameShape(QFrame::HLine);
    line0->setFrameShadow(QFrame::Sunken);
    mainLauout->addWidget(line0);


    mAccPreferenceLabel = new QLabel("Acceleration preference:");
    const auto sliderLayout = new QHBoxLayout;
    mAccPreferenceCpuLabel = new QLabel("CPU");
    mAccPreferenceSlider = new QSlider(Qt::Horizontal);
    mAccPreferenceSlider->setRange(0, 4);
    mAccPreferenceGpuLabel = new QLabel("GPU");
    sliderLayout->addWidget(mAccPreferenceCpuLabel);
    sliderLayout->addWidget(mAccPreferenceSlider);
    sliderLayout->addWidget(mAccPreferenceGpuLabel);
    mAccPreferenceDescLabel = new QLabel();
    mAccPreferenceDescLabel->setAlignment(Qt::AlignCenter);
    connect(mAccPreferenceSlider, &QSlider::valueChanged,
            this, &SettingsDialog::updateAccPreferenceDesc);
    mainLauout->addWidget(mAccPreferenceLabel);
    mainLauout->addLayout(sliderLayout);
    mainLauout->addWidget(mAccPreferenceDescLabel);

    const auto line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    mainLauout->addWidget(line1);

    mPathGpuAccCheck = new QCheckBox("Path GPU acceleration", this);
    mainLauout->addWidget(mPathGpuAccCheck);

//    const auto line2 = new QFrame();
//    line2->setFrameShape(QFrame::HLine);
//    line2->setFrameShadow(QFrame::Sunken);
//    mainLauout->addWidget(line2);

//    mHddCacheCheck = new QCheckBox("HDD cache", this);
//    mainLauout->addWidget(mHddCacheCheck);

//    QWidget* hddCacheSett = new QWidget(this);
//    hddCacheSett->setContentsMargins(0, 0, 0, 0);
//    const auto hddCacheSettLay = new QHBoxLayout;
//    hddCacheSett->setLayout(hddCacheSettLay);
//    hddCacheSettLay->setContentsMargins(0, 0, 0, 0);

//    mHddCacheMBCapCheck = new QCheckBox("HDD cache MB cap", this);
//    mHddCacheMBCapSpin = new QSpinBox(this);
//    connect(mHddCacheMBCapCheck, &QCheckBox::toggled,
//            mHddCacheMBCapSpin, &QWidget::setEnabled);

//    hddCacheSettLay->addWidget(mHddCacheMBCapCheck);
//    hddCacheSettLay->addWidget(mHddCacheMBCapSpin);
//    connect(mHddCacheCheck, &QCheckBox::toggled,
//            hddCacheSett, &QWidget::setEnabled);

//    mainLauout->addWidget(hddCacheSett);

    const auto buttonsLayout = new QHBoxLayout;

    const auto restoreButton = new QPushButton("Restore Defaults", this);
    const auto cancelButton = new QPushButton("Cancel", this);
    const auto applyButton = new QPushButton("Apply", this);
    buttonsLayout->addWidget(restoreButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(applyButton);

    mainLauout->addLayout(buttonsLayout);

    connect(restoreButton, &QPushButton::released, this, [this]() {
        eSettings::sInstance->loadDefaults();
        updateSettings();
    });

    connect(cancelButton, &QPushButton::released, this, &QDialog::close);

    connect(applyButton, &QPushButton::released, this, [this]() {
        eSettings& sett = *eSettings::sInstance;
//        sett.fCpuThreadsCap = mCpuThreadsCapCheck->isChecked() ?
//                    mCpuThreadsCapSpin->value() : 0;
        sett.fRamMBCap = intMB(mRamMBCapCheck->isChecked() ?
                    mRamMBCapSpin->value() : 0);
        sett.fAccPreference = static_cast<AccPreference>(
                    mAccPreferenceSlider->value());
        sett.fPathGpuAcc = mPathGpuAccCheck->isChecked();
//        sett.fHddCache = mHddCacheCheck->isChecked();
//        sett.fRamMBCap = mHddCacheMBCapCheck->isChecked() ?
//                    mHddCacheMBCapSpin->value() : 0;
        try {
            sett.saveToFile();
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
        close();
    });

    updateSettings();
}

void SettingsDialog::updateSettings() {
    eSettings& sett = *eSettings::sInstance;

//    mCpuThreadsCapCheck->setChecked(sett.fCpuThreadsCap > 0);
//    mCpuThreadsCapSpin->setRange(1, HardwareInfo::sCpuThreads());
//    mCpuThreadsCapSpin->setValue(sett.fCpuThreadsCap);
//    mCpuThreadsCapSpin->setEnabled(sett.fCpuThreadsCap > 0);

    mRamMBCapCheck->setChecked(sett.fRamMBCap.fValue > 250);
    mRamMBCapSpin->setValue(sett.fRamMBCap.fValue);
    mRamMBCapSpin->setEnabled(sett.fRamMBCap.fValue > 250);
    mRamMBCapSlider->setValue(sett.fRamMBCap.fValue);

    mAccPreferenceSlider->setValue(static_cast<int>(sett.fAccPreference));
    updateAccPreferenceDesc();
    mPathGpuAccCheck->setChecked(sett.fPathGpuAcc);

//    mHddCacheCheck->setChecked(sett.fHddCache);

//    mHddCacheMBCapCheck->setChecked(sett.fHddCacheMBCap > 0);
//    mHddCacheMBCapSpin->setEnabled(sett.fHddCacheMBCap > 0);
//    mHddCacheMBCapSpin->setValue(sett.fHddCacheMBCap);
}

void SettingsDialog::updateAccPreferenceDesc() {
    const int value = mAccPreferenceSlider->value();
    QString toolTip;
    if(value == 0) {
        mAccPreferenceDescLabel->setText("Strong CPU preference");
        toolTip = "Use the GPU only for tasks not supported by the CPU";
    } else if(value == 1) {
        mAccPreferenceDescLabel->setText("Soft CPU preference");
        toolTip = "Use the GPU only for tasks marked as preferred for the GPU";
    } else if(value == 2) {
        mAccPreferenceDescLabel->setText("Hardware agnostic");
        toolTip = "Adhere to the default hardware preference";
    } else if(value == 3) {
        mAccPreferenceDescLabel->setText("Soft GPU preference");
        toolTip = "Use the CPU only for tasks marked as preferred for the CPU";
    } else if(value == 4) {
        mAccPreferenceDescLabel->setText("Strong GPU preference");
        toolTip = "Use the CPU only for tasks not supported by the GPU";
    }
    mAccPreferenceDescLabel->setToolTip("<p style='white-space:pre'>" + toolTip + "</p>");
    mAccPreferenceSlider->setToolTip("<p style='white-space:pre'>" + toolTip + "</p>");
}
