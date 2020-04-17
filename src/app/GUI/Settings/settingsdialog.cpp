#include "settingsdialog.h"
#include "Private/esettings.h"
#include "hardwareinfo.h"
#include "exceptions.h"
#include "GUI/global.h"

#include "performancesettingswidget.h"
#include "interfacesettingswidget.h"
#include "timelinesettingswidget.h"
#include "externalappssettingswidget.h"

#include <QVBoxLayout>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget * const parent) :
    QDialog(parent) {
    eSizesUI::widget.add(this, [this](const int size) {
        setMinimumWidth(20*size);
    });
    setWindowTitle("Settings");

    const auto mainLauout = new QVBoxLayout;
    setLayout(mainLauout);

    mTabWidget = new QTabWidget(this);
    eSizesUI::widget.add(this, [this](const int size) {
        mTabWidget->setContentsMargins(size, size, size, size);
    });

    const auto performance = new PerformanceSettingsWidget(this);
    addSettingsWidget(performance, "Performance");

    const auto interface = new InterfaceSettingsWidget(this);
    addSettingsWidget(interface, "Interface");

    const auto timeline = new TimelineSettingsWidget(this);
    addSettingsWidget(timeline, "Timeline");

    const auto external = new ExternalAppsSettingsWidget(this);
    addSettingsWidget(external, "External Apps");

    mainLauout->addWidget(mTabWidget);

    const auto buttonsLayout = new QHBoxLayout;

    const auto restoreButton = new QPushButton("Restore Defaults", this);
    const auto cancelButton = new QPushButton("Cancel", this);
    const auto applyButton = new QPushButton("Apply", this);
    buttonsLayout->addWidget(restoreButton);
    eSizesUI::widget.addSpacing(buttonsLayout);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(applyButton);

    eSizesUI::widget.addSpacing(mainLauout);
    mainLauout->addStretch();
    mainLauout->addLayout(buttonsLayout);

    connect(restoreButton, &QPushButton::released, this, [this]() {
        eSettings::sInstance->loadDefaults();
        updateSettings();
        eSizesUI::font.updateSize();
        eSizesUI::widget.updateSize();
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
