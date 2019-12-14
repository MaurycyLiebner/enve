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

#ifndef SETTINGS_H
#define SETTINGS_H
#include <QtCore>
#include "skia/skiaincludes.h"
#include "efiltersettings.h"
#include "memorystructs.h"

enum class GpuVendor {
    intel,
    amd,
    nvidia,
    unrecognized
};

enum class AccPreference {
    cpuStrongPreference,
    cpuSoftPreference,
    defaultPreference,
    gpuSoftPreference,
    gpuStrongPreference
};

class eSettings {
public:
    eSettings();

    // accessors
    static intMB sRamMBCap();
    static int sCpuThreadsCapped();
    static const QString& sSettingsDir();
    static QString sIconsDir();
    static eSettings* sInstance;

    void loadDefaults();
    void loadFromFile();
    void saveToFile();

    // general
#ifdef QT_DEBUG
    QString fUserSettingsDir = QDir::homePath() + "/.enveD";
#else
    QString fUserSettingsDir = QDir::homePath() + "/.enve";
#endif

    // performance settings
    int fCpuThreads = 0;
    int fCpuThreadsCap = 0; // <= 0 - use all available threads

    intKB fRamKB = intKB(0);
    intMB fRamMBCap = intMB(0); // <= 0 - cap at 80 %

    GpuVendor fGpuVendor = GpuVendor::unrecognized;
    AccPreference fAccPreference = AccPreference::defaultPreference;
    bool fPathGpuAcc = true;

    bool fHddCache = true;
    QString fHddCacheFolder = ""; // "" - use system default temporary files folder
    int fHddCacheMBCap = 0; // <= 0 - no cap

    // history
    int fUndoCap = 25; // <= 0 - no cap

    enum class AutosaveTarget {
        dedicated_folder,
        same_folder
    };

    int fQuickSaveCap = 5; // <= 0 - no cap
    int fAutoQuickSaveMin = 0; // <= 0 - disabled
    AutosaveTarget fQuickSaveTarget = AutosaveTarget::same_folder;
};

#endif // SETTINGS_H
