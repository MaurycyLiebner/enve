#include "settings.h"

EnveSettings EnveSettings::sSettings;

QString EnveSettings::sSettingsDir() {
    return sSettings.fUserSettingsDir;
}
