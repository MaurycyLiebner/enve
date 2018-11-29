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

        AVOutputFormat *mFormat = nullptr;
        QList<AVCodecID> mVidCodecs;
        QList<AVCodecID> mAudioCodecs;
    };
    RenderSettingsDialog(const OutputSettings &settings,
                         QWidget *parent = nullptr);
    OutputSettings getSettings() {
        OutputSettings settings = mInitialSettings;

        AVOutputFormat *currentOutputFormat = nullptr;
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
        settings.videoBitrate = mBitrateSpinBox->value()*1000000;

        settings.audioEnabled = mAudioGroupBox->isChecked();
        AVCodec *currentAudioCodec = nullptr;
        if(mAudioCodecsComboBox->count() > 0) {
            int codecId = mAudioCodecsComboBox->currentIndex();
            currentAudioCodec = mAudioCodecsList.at(codecId);
        }
        settings.audioCodec = currentAudioCodec;

        AVSampleFormat currentSampleFormat = AV_SAMPLE_FMT_NONE;
        if(mSampleFormatsComboBox->count() > 0) {
            int formatId = mSampleFormatsComboBox->currentIndex();
            currentSampleFormat = mSampleFormatsList.at(formatId);
        }
        settings.audioSampleFormat = currentSampleFormat;

        int currentSampleRate = 0;
        if(mSampleRateComboBox->count() > 0) {
            int sampleRateId = mSampleRateComboBox->currentIndex();
            currentSampleRate = mSampleRatesList.at(sampleRateId);
        }
        settings.audioSampleRate = currentSampleRate;

        int currentAudioBitrate = 0;
        if(mAudioBitrateComboBox->count() > 0) {
            int bitrateId = mAudioBitrateComboBox->currentIndex();
            currentAudioBitrate = mAudioBitrateList.at(bitrateId);
        }
        settings.audioBitrate = currentAudioBitrate;

        uint64_t currentChannelsLayout = 0;
        if(mAudioChannelLayoutsComboBox->count() > 0) {
            int layoutId = mAudioChannelLayoutsComboBox->currentIndex();
            currentAudioBitrate = mAudioChannelLayoutsList.at(layoutId);
        }
        settings.audioChannelsLayout = currentChannelsLayout;

        return settings;
    }
protected slots:
    void updateAvailableOutputFormats();

    void updateAvailablePixelFormats();
    void updateAvailableVideoCodecs();

    void updateAvailableAudioCodecs();
    void updateAvailableSampleRates();
    void updateAvailableSampleFormats();
    void updateAvailableAudioBitrates();
    void updateAvailableAudioChannelLayouts();

    void restoreInitialSettings();

    void setShowAllFormatsAndCodecs(const bool &bT) {
        if(mShowAllFormatsAndCodecs == bT) return;
        mShowAllFormatsAndCodecs = bT;
        updateAvailableOutputFormats();
    }
protected:
    QList<FormatCodecs> mSupportedFormats;
    bool mShowAllFormatsAndCodecs = false;
    QList<AVPixelFormat> mPixelFormatsList;
    QList<AVCodec*> mVideoCodecsList;
    QList<AVOutputFormat*> mOutputFormatsList;
    QList<AVCodec*> mAudioCodecsList;
    QList<int> mSampleRatesList;
    QList<AVSampleFormat> mSampleFormatsList;
    QList<int> mAudioBitrateList;
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
                       AVOutputFormat *outputFormat,
                       const QString &currentCodecName);
    void addAudioCodec(const AVCodecID &codecId,
                       AVOutputFormat *outputFormat,
                       const QString &currentCodecName);
};

#endif // RENDERSETTINGSDIALOG_H
