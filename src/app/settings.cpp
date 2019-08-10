#include "settings.h"
#include "hardwareinfo.h"
EnveSettings EnveSettings::sSettings;

int EnveSettings::sCpuThreadsCapped() {
    if(sSettings.fCpuThreadsCap > 0)
        return sSettings.fCpuThreadsCap;
    return HardwareInfo::sCpuThreads();
}

long EnveSettings::sRamBytesCap() {
    if(sSettings.fRamBytesCap > 0)
        return sSettings.fRamBytesCap;
    return HardwareInfo::sRamBytes()*8/10;
}

const QString &EnveSettings::sSettingsDir() {
    return sSettings.fUserSettingsDir;
}

#include "GUI/global.h"
QString EnveSettings::sIconsDir() {
    return sSettingsDir() + "/icons/" + QString::number(MIN_WIDGET_DIM);
}
