#ifndef ESOUNDSETTINGS_H
#define ESOUNDSETTINGS_H

#include <QObject>
extern "C" {
    #include <libavutil/samplefmt.h>
    #include <libavutil/channel_layout.h>
}

struct eSoundSettingsData {
    int fSampleRate = 44100;
    AVSampleFormat fSampleFormat = AV_SAMPLE_FMT_FLT;
    uint64_t fChannelLayout = AV_CH_LAYOUT_STEREO;

    bool planarFormat() const {
        return av_sample_fmt_is_planar(fSampleFormat);
    }

    int channelCount() const {
        return av_get_channel_layout_nb_channels(fChannelLayout);
    }

    int bytesPerSample() const {
        return av_get_bytes_per_sample(fSampleFormat);
    }
};

class eSoundSettings : public QObject, private eSoundSettingsData {
    eSoundSettings();
public:
    static eSoundSettings sSettings;

    static int sSampleRate() {
        return sSettings.fSampleRate;
    }

    static AVSampleFormat sSampleFormat() {
        return sSettings.fSampleFormat;
    }

    static uint64_t sChannelLayout() {
        return sSettings.fChannelLayout;
    }

    static bool sPlanarFormat() {
        return sSettings.planarFormat();
    }

    static int sChannelCount() {
        return sSettings.channelCount();
    }

    static int sBytesPerSample() {
        return sSettings.bytesPerSample();
    }

    static eSoundSettingsData sData() {
        return sSettings;
    }

    static void sSetSampleRate(const int sampleRate) {
        sSettings.setSampleRate(sampleRate);
    }

    static void sSetSampleFormat(const AVSampleFormat format) {
        sSettings.setSampleFormat(format);
    }

    static void sSetChannelLayout(const uint64_t layout) {
        sSettings.setChannelLayout(layout);
    }

    void setSampleRate(const int sampleRate) {
        fSampleRate = sampleRate;
        emit settingsChanged();
    }

    void setSampleFormat(const AVSampleFormat format) {
        fSampleFormat = format;
        emit settingsChanged();
    }

    void setChannelLayout(const uint64_t layout) {
        fChannelLayout = layout;
        emit settingsChanged();
    }
signals:
    void settingsChanged();
};

#endif // ESOUNDSETTINGS_H
