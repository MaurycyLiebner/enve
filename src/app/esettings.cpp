// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
