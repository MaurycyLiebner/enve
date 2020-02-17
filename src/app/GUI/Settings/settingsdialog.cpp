#include "settingsdialog.h"
#include "Private/esettings.h"
#include "hardwareinfo.h"
#include "exceptions.h"
#include "GUI/global.h"

#include "performancesettingswidget.h"
#include "timelinesettingswidget.h"

#include <QVBoxLayout>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget * const parent) :
    QDialog(parent) {
    setWindowTitle("Settings");

    const auto mainLauout = new QVBoxLayout;
    setLayout(mainLauout);

    mTabWidget = new QTabWidget(this);
    mTabWidget->setContentsMargins(MIN_WIDGET_DIM, MIN_WIDGET_DIM,
                                  MIN_WIDGET_DIM, MIN_WIDGET_DIM);

    const auto performance = new PerformanceSettingsWidget(this);
    addSettingsWidget(performance, "Performance");

    const auto timeline = new TimelineSettingsWidget(this);
    addSettingsWidget(timeline, "Timeline");

    mainLauout->addWidget(mTabWidget);

    const auto buttonsLayout = new QHBoxLayout;

    const auto restoreButton = new QPushButton("Restore Defaults", this);
    const auto cancelButton = new QPushButton("Cancel", this);
    const auto applyButton = new QPushButton("Apply", this);
    buttonsLayout->addWidget(restoreButton);
    buttonsLayout->addSpacing(MIN_WIDGET_DIM);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(applyButton);

    mainLauout->addSpacing(MIN_WIDGET_DIM);
    mainLauout->addStretch();
    mainLauout->addLayout(buttonsLayout);

    connect(restoreButton, &QPushButton::released, this, [this]() {
        eSettings::sInstance->loadDefaults();
        updateSettings();
    });

    connect(cancelButton, &QPushButton::released, this, &QDialog::close);

    connect(applyButton, &QPushButton::released, this, [this]() {
        for(const auto widget : mSettingWidgets) {
            widget->applySettings();
        }
        try {
            eSettings& sett = *eSettings::sInstance;
            sett.saveToFile();
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
        close();
    });

    updateSettings();
}

void SettingsDialog::addSettingsWidget(SettingsWidget * const widget,
                                       const QString &name) {
    mTabWidget->addTab(widget, name);
    mSettingWidgets << widget;
}

void SettingsDialog::updateSettings() {
    for(const auto widget : mSettingWidgets) {
        widget->updateSettings();
    }
}
