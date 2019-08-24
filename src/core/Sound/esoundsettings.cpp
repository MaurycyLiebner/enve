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
    setAll(mSaved);
}

void eSoundSettings::setAll(const eSoundSettings::eSoundSettingsData &data) {
    if(eSoundSettingsData::operator==(data)) return;
    eSoundSettingsData::operator=(data);
    emit settingsChanged();
}

void eSoundSettings::setSampleRate(const int sampleRate) {
    if(sampleRate == fSampleRate) return;
    fSampleRate = sampleRate;
    emit settingsChanged();
}

void eSoundSettings::setSampleFormat(const AVSampleFormat format) {
    if(format == fSampleFormat) return;
    fSampleFormat = format;
    emit settingsChanged();
}

void eSoundSettings::setChannelLayout(const uint64_t layout) {
    if(layout == fChannelLayout) return;
    fChannelLayout = layout;
    emit settingsChanged();
}
