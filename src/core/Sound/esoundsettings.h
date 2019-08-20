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
    uint64_t fChannelLayout = AV_CH_LAYOUT_MONO;

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
    Q_OBJECT

    eSoundSettings();
    eSoundSettingsData mSaved;
public:
    static eSoundSettings sSettings;

    static int sSampleRate();
    static AVSampleFormat sSampleFormat();
    static uint64_t sChannelLayout();
    static bool sPlanarFormat();
    static int sChannelCount();
    static int sBytesPerSample();
    static eSoundSettingsData sData();

    static void sSetSampleRate(const int sampleRate);
    static void sSetSampleFormat(const AVSampleFormat format);
    static void sSetChannelLayout(const uint64_t layout);

    static void sSave();
    static void sRestore();

    void save();
    void restore();

    void setSampleRate(const int sampleRate);
    void setSampleFormat(const AVSampleFormat format);
    void setChannelLayout(const uint64_t layout);
signals:
    void settingsChanged();
};

#endif // ESOUNDSETTINGS_H
