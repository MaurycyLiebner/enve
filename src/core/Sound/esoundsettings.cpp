#include "esoundsettings.h"

eSoundSettings eSoundSettings::sSettings;

eSoundSettings::eSoundSettings() {}

int eSoundSettings::sSampleRate() {
    return sSettings.fSampleRate;
}

AVSampleFormat eSoundSettings::sSampleFormat() {
    return sSettings.fSampleFormat;
}

uint64_t eSoundSettings::sChannelLayout() {
    return sSettings.fChannelLayout;
}

bool eSoundSettings::sPlanarFormat() {
    return sSettings.planarFormat();
}

int eSoundSettings::sChannelCount() {
    return sSettings.channelCount();
}

int eSoundSettings::sBytesPerSample() {
    return sSettings.bytesPerSample();
}

eSoundSettingsData eSoundSettings::sData() {
    return sSettings;
}

void eSoundSettings::sSetSampleRate(const int sampleRate) {
    sSettings.setSampleRate(sampleRate);
}

void eSoundSettings::sSetSampleFormat(const AVSampleFormat format) {
    sSettings.setSampleFormat(format);
}

void eSoundSettings::sSetChannelLayout(const uint64_t layout) {
    sSettings.setChannelLayout(layout);
}

void eSoundSettings::sSave() {
    sSettings.save();
}

void eSoundSettings::sRestore() {
    sSettings.restore();
}

void eSoundSettings::save() {
    mSaved = sData();
}

void eSoundSettings::restore() {
    eSoundSettingsData::operator=(mSaved);
    emit settingsChanged();
}

void eSoundSettings::setSampleRate(const int sampleRate) {
    fSampleRate = sampleRate;
    emit settingsChanged();
}

void eSoundSettings::setSampleFormat(const AVSampleFormat format) {
    fSampleFormat = format;
    emit settingsChanged();
}

void eSoundSettings::setChannelLayout(const uint64_t layout) {
    fChannelLayout = layout;
    emit settingsChanged();
}
