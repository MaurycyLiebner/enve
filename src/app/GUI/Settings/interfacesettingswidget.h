#ifndef INTERFACESETTINGSWIDGET_H
#define INTERFACESETTINGSWIDGET_H

#include "settingswidget.h"

#include <QSlider>

class InterfaceSettingsWidget : public SettingsWidget {
public:
    explicit InterfaceSettingsWidget(QWidget *parent);

    void applySettings();
    void updateSettings();
private:
    QSlider* mInterfaceScaling = nullptr;
};

#endif // INTERFACESETTINGSWIDGET_H
