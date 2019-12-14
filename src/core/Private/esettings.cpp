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

#include "GUI/global.h"
#include "exceptions.h"

eSettings* eSettings::sInstance = nullptr;

eSettings::eSettings() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

int eSettings::sCpuThreadsCapped() {
    if(sInstance->fCpuThreadsCap > 0)
        return sInstance->fCpuThreadsCap;
    return sInstance->fCpuThreads;
}

intMB eSettings::sRamMBCap() {
    if(sInstance->fRamMBCap.fValue > 0) return sInstance->fRamMBCap;
    auto mbTot = intMB(sInstance->fRamKB);
    mbTot.fValue *= 8; mbTot.fValue /= 10;
    return mbTot;
}

const QString &eSettings::sSettingsDir() {
    return sInstance->fUserSettingsDir;
}

QString eSettings::sIconsDir() {
    return sSettingsDir() + "/icons/" + QString::number(MIN_WIDGET_DIM);
}

void eSettings::loadDefaults() {
    fCpuThreadsCap = 0;
    fRamMBCap = intMB(0);
    fAccPreference = AccPreference::defaultPreference;
    fPathGpuAcc = fGpuVendor != GpuVendor::nvidia;
    fHddCache = true;
    fHddCacheFolder = "";
    fHddCacheMBCap = intMB(0);
    fUndoCap = 25;
}

void eSettings::loadFromFile() {
    loadDefaults();
    const QString settingsFile = sSettingsDir() + "/settings";
    QFile file(settingsFile);
    if(!file.exists()) return;
    if(!file.open(QIODevice::ReadOnly))
        RuntimeThrow("Could not open \"" + settingsFile + "\" for reading");

    QTextStream textStream(&file);

    QString line;
    bool ok = true;
    while(!textStream.atEnd()) {
        line = textStream.readLine();
        const QStringList split = line.split(":");
        if(split.count() != 2) continue;
        const QString setting = split.first().trimmed();
        const QString value = split.last().trimmed();
        if(setting == "cpuThreadsCap") {
            const int cpuThreadsCap = value.toInt(&ok);
            if(ok) fCpuThreads = cpuThreadsCap;
        } else if(setting == "ramMBCap") {
            const int ramMBCap = value.toInt(&ok);
            if(ok) fRamMBCap = intMB(ramMBCap);
        } else if(setting == "accPreference") {
            const int accPreference = value.toInt(&ok);
            ok = accPreference >= 0 && accPreference <= 4;
            if(ok) fAccPreference = static_cast<AccPreference>(accPreference);
        } else if(setting == "pathGpuAcc") { //
            const bool pathGpuAcc = value == "enabled";
            ok = pathGpuAcc || value == "disabled";
            if(ok) fPathGpuAcc = pathGpuAcc;
        } else if(setting == "hddCache") { //
            const bool hddCache = value == "enabled";
            ok = hddCache || value == "disabled";
            if(ok) fHddCache = hddCache;
        } else if(setting == "hddCacheMBCap") {
            const int hddCacheMBCap = value.toInt(&ok);
            if(ok) fHddCacheMBCap = intMB(hddCacheMBCap);
        } else ok = false;

        if(!ok) break;
    }
    file.close();

    if(!ok) RuntimeThrow("Invalid setting \"" + line +
                         "\" in \"" + settingsFile + "\"");
}

void eSettings::saveToFile() {
    const QString settingsFile = sSettingsDir() + "/settings";
    QFile file(settingsFile);
    if(!file.open(QIODevice::WriteOnly))
        RuntimeThrow("Could not open \"" + settingsFile + "\" for writing");
    QTextStream textStream(&file);

    textStream << "cpuThreadsCap: " << fCpuThreadsCap << endl;
    textStream << "ramMBCap: " << fRamMBCap.fValue << endl;

    textStream << "accPreference: " << static_cast<int>(fAccPreference) << endl;
    textStream << "pathGpuAcc: " << (fPathGpuAcc ? "enabled" : "disabled") << endl;

//    textStream << "hddCache: " << (fHddCache ? "enabled" : "disabled") << endl;
//    textStream << "hddCacheMBCap: " << fHddCacheMBCap << endl;

    textStream.flush();
    file.close();
}
