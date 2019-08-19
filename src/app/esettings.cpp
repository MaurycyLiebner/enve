#include "esettings.h"
#include "hardwareinfo.h"

eSettings eSettings::sSettings;

int eSettings::sCpuThreadsCapped() {
    if(sSettings.fCpuThreadsCap > 0)
        return sSettings.fCpuThreadsCap;
    return HardwareInfo::sCpuThreads();
}

long eSettings::sRamBytesCap() {
    if(sSettings.fRamBytesCap > 0)
        return sSettings.fRamBytesCap;
    return HardwareInfo::sRamBytes()*8/10;
}

const QString &eSettings::sSettingsDir() {
    return sSettings.fUserSettingsDir;
}

#include "GUI/global.h"
QString eSettings::sIconsDir() {
    return sSettingsDir() + "/icons/" + QString::number(MIN_WIDGET_DIM);
}
