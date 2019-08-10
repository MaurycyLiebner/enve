#ifndef SETTINGS_H
#define SETTINGS_H
#include <QtCore>
#include "skia/skiaincludes.h"

struct EnveSettings {
private:
    EnveSettings() {}
public:
    // general
#ifdef QT_DEBUG
    QString fUserSettingsDir = QDir::homePath() + "/.enveD";
#else
    QString fUserSettingsDir = QDir::homePath() + "/.enve";
#endif

    // quality
    SkFilterQuality fEnveFilter = SkFilterQuality::kNone_SkFilterQuality;
    SkFilterQuality fOutputFilter = SkFilterQuality::kHigh_SkFilterQuality;
    SkFilterQuality fViewFilter = SkFilterQuality::kNone_SkFilterQuality;

    // performance settings
    int fCpuThreadsCap = 0; // <= 0 - use all available threads

    long fRamBytesCap = 0; // <= 0 - cap at 80 %

    bool fGpuAcceleration = true;

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

    // accessors
    static long sRamBytesCap();
    static int sCpuThreadsCapped();
    static const QString& sSettingsDir();
    static QString sIconsDir();
    static EnveSettings sSettings;
};

#endif // SETTINGS_H
