#include "interfacesettingswidget.h"

#include "Private/esettings.h"
#include "GUI/global.h"

#include <QLabel>

InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget *parent) :
    SettingsWidget(parent) {
    mInterfaceScaling = new QSlider(Qt::Horizontal, this);
    mInterfaceScaling->setRange(50, 150);
    mInterfaceScaling->setSingleStep(5);

    const auto uiScaleLayout = new QHBoxLayout;
    const auto interfaceValue = new QLabel();
    uiScaleLayout->addWidget(new QLabel("Interface scaling"));
    uiScaleLayout->addWidget(mInterfaceScaling);
    uiScaleLayout->addWidget(interfaceValue);
    connect(mInterfaceScaling, &QSlider::valueChanged,
            interfaceValue, [interfaceValue](const int value) {
        interfaceValue->setText(QString::number(value) + " %");
    });
    emit mInterfaceScaling->valueChanged(100);

    addLayout(uiScaleLayout);
}

void InterfaceSettingsWidget::applySettings() {
    mSett.fInterfaceScaling = mInterfaceScaling->value()*0.01;
    eSizesUI::font.updateSize();
    eSizesUI::widget.updateSize();
}

void InterfaceSettingsWidget::updateSettings() {
    mInterfaceScaling->setValue(100*mSett.fInterfaceScaling);
}
