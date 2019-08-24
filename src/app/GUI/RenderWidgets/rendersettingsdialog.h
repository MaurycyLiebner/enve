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

#ifndef RENDERSETTINGSDIALOG_H
#define RENDERSETTINGSDIALOG_H
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include "renderinstancesettings.h"
#include "GUI/twocolumnlayout.h"
#define COMPLIENCE FF_COMPLIANCE_NORMAL

class RenderSettingsDialog : public QDialog {
    Q_OBJECT
public:
    struct FormatCodecs {
        FormatCodecs(const QList<AVCodecID> &vidCodecs,
                     const QList<AVCodecID> &audioCodec,
                     const char *name);

        const AVOutputFormat *mFormat = nullptr;
        QList<AVCodecID> mVidCodecs;
        QList<AVCodecID> mAudioCodecs;
    };
    RenderSettingsDialog(const OutputSettings &settings,
                         QWidget *parent = nullptr);
    OutputSettings getSettings() {
        OutputSettings settings = mInitialSettings;

        const AVOutputFormat *currentOutputFormat = nullptr;
        if(mVideoCodecsComboBox->count() > 0) {
            int formatId = mOutputFormatsComboBox->currentIndex();
            currentOutputFormat = mOutputFormatsList.at(formatId);
        }
        settings.outputFormat = currentOutputFormat;

        settings.videoEnabled = mVideoGroupBox->isChecked();
        AVCodec *currentVideoCodec = nullptr;
        if(mVideoCodecsComboBox->count() > 0) {
            int codecId = mVideoCodecsComboBox->currentIndex();
            currentVideoCodec = mVideoCodecsList.at(codecId);
        }
        settings.videoCodec = currentVideoCodec;

        AVPixelFormat currentPixelFormat = AV_PIX_FMT_NONE;
        if(mPixelFormatsList.count() > 0) {
            int formatId = mPixelFormatsComboBox->currentIndex();
            currentPixelFormat = mPixelFormatsList.at(formatId);
        }
        settings.videoPixelFormat = currentPixelFormat;
        settings.videoBitrate = qRound(mBitrateSpinBox->value()*1000000);

        settings.audioEnabled = mAudioGroupBox->isChecked();
        AVCodec *currentAudioCodec = nullptr;
        if(mAudioCodecsComboBox->count() > 0) {
            const int codecId = mAudioCodecsComboBox->currentIndex();
            currentAudioCodec = mAudioCodecsList.at(codecId);
        }
        settings.audioCodec = currentAudioCodec;

        AVSampleFormat currentSampleFormat = AV_SAMPLE_FMT_NONE;
        if(mSampleFormatsComboBox->count() > 0) {
            const int formatId = mSampleFormatsComboBox->currentIndex();
            currentSampleFormat = mSampleFormatsList.at(formatId);
        }
        settings.audioSampleFormat = currentSampleFormat;

        int currentSampleRate = 0;
        if(mSampleRateComboBox->count() > 0) {
            currentSampleRate = mSampleRateComboBox->currentData().toInt();
        }
        settings.audioSampleRate = currentSampleRate;

        int currentAudioBitrate = 0;
        if(mAudioBitrateComboBox->count() > 0) {
            currentAudioBitrate = mAudioBitrateComboBox->currentData().toInt();
        }
        settings.audioBitrate = currentAudioBitrate;

        uint64_t currentChannelsLayout = 0;
        if(mAudioChannelLayoutsComboBox->count() > 0) {
            const int layoutId = mAudioChannelLayoutsComboBox->currentIndex();
            currentChannelsLayout = mAudioChannelLayoutsList.at(layoutId);
        }
        settings.audioChannelsLayout = currentChannelsLayout;

        return settings;
    }
protected:
    void updateAvailableOutputFormats();

    void updateAvailablePixelFormats();
    void updateAvailableVideoCodecs();

    void updateAvailableAudioCodecs();
    void updateAvailableSampleRates();
    void updateAvailableSampleFormats();
    void updateAvailableAudioBitrates();
    void updateAvailableAudioChannelLayouts();

    void restoreInitialSettings();

    void setShowAllFormatsAndCodecs(const bool bT) {
        if(mShowAllFormatsAndCodecs == bT) return;
        mShowAllFormatsAndCodecs = bT;
        updateAvailableOutputFormats();
    }
    void updateAvailableCodecs();

    QList<FormatCodecs> mSupportedFormats;
    bool mShowAllFormatsAndCodecs = false;
    QList<AVPixelFormat> mPixelFormatsList;
    QList<AVCodec*> mVideoCodecsList;
    QList<const AVOutputFormat*> mOutputFormatsList;
    QList<AVCodec*> mAudioCodecsList;
    QList<AVSampleFormat> mSampleFormatsList;
    QList<uint64_t> mAudioChannelLayoutsList;

    OutputSettings mInitialSettings;
    QVBoxLayout *mMainLayout = nullptr;

    QHBoxLayout *mOutputFormatsLayout = nullptr;
    QLabel *mOutputFormatsLabel = nullptr;
    QComboBox *mOutputFormatsComboBox = nullptr;

    QGroupBox *mVideoGroupBox = nullptr;
    TwoColumnLayout *mVideoSettingsLayout = nullptr;
    QLabel *mVideoCodecsLabel = nullptr;
    QComboBox *mVideoCodecsComboBox = nullptr;
    QLabel *mPixelFormatsLabel = nullptr;
    QComboBox *mPixelFormatsComboBox = nullptr;
    QLabel *mBitrateLabel = nullptr;
    QDoubleSpinBox *mBitrateSpinBox = nullptr;

    QGroupBox *mAudioGroupBox = nullptr;
    TwoColumnLayout *mAudioSettingsLayout = nullptr;
    QLabel *mAudioCodecsLabel = nullptr;
    QComboBox *mAudioCodecsComboBox = nullptr;
    QLabel *mSampleFormatsLabel = nullptr;
    QComboBox *mSampleFormatsComboBox = nullptr;
    QLabel *mSampleRateLabel = nullptr;
    QComboBox *mSampleRateComboBox = nullptr;
    QLabel *mAudioBitrateLabel = nullptr;
    QComboBox *mAudioBitrateComboBox = nullptr;
    QLabel *mAudioChannelLayoutLabel = nullptr;
    QComboBox *mAudioChannelLayoutsComboBox = nullptr;

    QHBoxLayout *mShowLayout = nullptr;
    QLabel *mShowLabel = nullptr;
    QCheckBox *mShowAllFormatsAndCodecsCheckBox = nullptr;

    QHBoxLayout *mButtonsLayout = nullptr;
    QPushButton *mOkButton = nullptr;
    QPushButton *mCancelButton = nullptr;
    QPushButton *mResetButton = nullptr;
    void addVideoCodec(const AVCodecID &codecId,
                       const AVOutputFormat *outputFormat,
                       const QString &currentCodecName);
    void addAudioCodec(const AVCodecID &codecId,
                       const AVOutputFormat *outputFormat,
                       const QString &currentCodecName);
};

#endif // RENDERSETTINGSDIALOG_H
