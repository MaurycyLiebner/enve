#ifndef OUTPUTSETTINGSDISPLAYWIDGET_H
#define OUTPUTSETTINGSDISPLAYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QGroupBox>
#include "renderinstancesettings.h"

class OutputSettingsDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OutputSettingsDisplayWidget(QWidget *parent = nullptr);
    void setOutputSettings(const OutputSettings &settings) {
        if(settings.outputFormat  == nullptr) {
            setOutputFormatText("-");
        } else {
            setOutputFormatText(QString(settings.outputFormat->long_name));
        }
        if(!mAlwaysShowAll) {
            setVideoLabelsVisible(settings.videoEnabled);
        }
        if(settings.videoCodec  == nullptr) {
            setVideoCodecText("-");
        } else {
            setVideoCodecText(QString(settings.videoCodec->long_name));
        }
        const char *pixelFormat = av_get_pix_fmt_name(settings.videoPixelFormat);
        if(pixelFormat == nullptr) {
            setPixelFormatText("-");
        } else {
            setPixelFormatText(QString(pixelFormat));
        }
        setVideoBitrateText(QString::number(settings.videoBitrate/1000000) + " Mbps");
        if(!mAlwaysShowAll) {
            setAudioLabelsVisible(settings.audioEnabled);
        }
        if(settings.audioCodec  == nullptr) {
            setAudioCodecText("-");
        } else {
            setAudioCodecText(QString(settings.audioCodec->long_name));
        }
        setAudioSampleRateText(QString::number(settings.audioSampleRate) + " Hz");
        int formatId = settings.audioSampleFormat;
        if(formatId < 0 || formatId >= OutputSettings::SAMPLE_FORMATS_NAMES.count()) {
            setAudioSampleFormatText("-");
        } else {
            setAudioSampleFormatText(OutputSettings::SAMPLE_FORMATS_NAMES.at(formatId));
        }
        setAudioBitrateText(QString::number(settings.audioBitrate/1000) + " kbps");
        setAudioChannelLayoutText(OutputSettings::getChannelsLayoutNameStatic(settings.audioChannelsLayout));
    }

    void setAlwaysShowAll(const bool &bT) {
        mAlwaysShowAll = bT;
        setVideoLabelsVisible(true);
        setAudioLabelsVisible(true);
    }
signals:

public slots:
protected:
    void setVideoLabelsVisible(const bool &bT) {
        mVideoCodecLabel->setVisible(bT);
        mVideoPixelFormatLabel->setVisible(bT);
        mVideoBitrateLabel->setVisible(bT);
    }

    void setAudioLabelsVisible(const bool &bT) {
        mAudioCodecLabel->setVisible(bT);
        mAudioSampleRateLabel->setVisible(bT);
        mAudioSampleFormatLabel->setVisible(bT);
        mAudioBitrateLabel->setVisible(bT);
        mAudioChannelLayoutLabel->setVisible(bT);
    }

    void setOutputFormatText(const QString &txt) {
        mOutputFormatLabel->setText("<b>Format:</b> " + txt);
    }

    void setVideoCodecText(const QString &txt) {
        mVideoCodecLabel->setText("<b>Video codec:</b> " + txt);
    }

    void setPixelFormatText(const QString &txt) {
        mVideoPixelFormatLabel->setText("<b>Pixel format:</b> " + txt);
    }

    void setVideoBitrateText(const QString &txt) {
        mVideoBitrateLabel->setText("<b>Video bitrate:</b> " + txt);
    }

    void setAudioCodecText(const QString &txt) {
        mAudioCodecLabel->setText("<b>Audio codec:</b> " + txt);
    }

    void setAudioSampleRateText(const QString &txt) {
        mAudioSampleRateLabel->setText("<b>Audio sample rate:</b> " + txt);
    }

    void setAudioSampleFormatText(const QString &txt) {
        mAudioSampleFormatLabel->setText("<b>Audio sample format:</b> " + txt);
    }

    void setAudioBitrateText(const QString &txt) {
        mAudioBitrateLabel->setText("<b>Audio bitrate:</b> " + txt);
    }

    void setAudioChannelLayoutText(const QString &txt) {
        mAudioChannelLayoutLabel->setText("<b>Audio channel layout:</b> " + txt);
    }

    QVBoxLayout *mMainLayout;

    QLabel *mOutputFormatLabel;

    QHBoxLayout *mAudioVideoLayout;

    //QGroupBox *mVideoGroupBox;
    QVBoxLayout *mVideoLayout;
    QLabel *mVideoCodecLabel;
    QLabel *mVideoPixelFormatLabel;
    QLabel *mVideoBitrateLabel;

    //QGroupBox *mAudioGroupBox;
    QVBoxLayout *mAudioLayout;
    QLabel *mAudioCodecLabel;
    QLabel *mAudioSampleRateLabel;
    QLabel *mAudioSampleFormatLabel;
    QLabel *mAudioBitrateLabel;
    QLabel *mAudioChannelLayoutLabel;

    bool mAlwaysShowAll = false;
};

#endif // OUTPUTSETTINGSDISPLAYWIDGET_H
