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

#ifndef OUTPUTSETTINGS_H
#define OUTPUTSETTINGS_H
#include <QString>
#include "Private/esettings.h"
#include "smartPointers/ememory.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavresample/avresample.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/opt.h>
}

struct OutputSettings {
    static const std::map<int, QString> SAMPLE_FORMATS_NAMES;
    static const std::map<uint64_t, QString> sChannelLayouts;
    static QString sGetChannelsLayoutName(const uint64_t &layout);
    static uint64_t sGetChannelsLayout(const QString &name);

    const AVOutputFormat *outputFormat = nullptr;

    bool videoEnabled = false;
    const AVCodec *videoCodec = nullptr;
    AVPixelFormat videoPixelFormat = AV_PIX_FMT_NONE;
    int videoBitrate = 0;

    bool audioEnabled = false;
    const AVCodec *audioCodec = nullptr;
    AVSampleFormat audioSampleFormat = AV_SAMPLE_FMT_NONE;
    uint64_t audioChannelsLayout = 0;
    int audioSampleRate = 0;
    int audioBitrate = 0;
};

class OutputSettingsProfile : public StdSelfRef {
    e_OBJECT
public:
    OutputSettingsProfile();

    const QString &getName() const { return mName; }
    void setName(const QString &name) { mName = name; }

    const OutputSettings &getSettings() const { return mSettings; }
    void setSettings(const OutputSettings &settings) { mSettings = settings; }

    void save();
    void load(const QString& path);

    bool wasSaved() const { return !mPath.isEmpty(); }
    void removeFile();
    const QString& path() const { return mPath; }
private:
    QString mPath;
    QString mName = "Untitled";
    OutputSettings mSettings;
};

#endif // OUTPUTSETTINGS_H
