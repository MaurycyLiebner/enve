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
#define COMPLIENCE FF_COMPLIANCE_NORMAL

class TwoColumnLayout : public QHBoxLayout {
public:
    TwoColumnLayout() {
        mLayout1 = new QVBoxLayout();
        mLayout2 = new QVBoxLayout();
        addLayout(mLayout1);
        addLayout(mLayout2);
    }

    void addWidgetToFirstColumn(QWidget *wid) {
        mLayout1->addWidget(wid);
    }

    void addWidgetToSecondColumn(QWidget *wid) {
        mLayout2->addWidget(wid);
    }

    void addPair(QWidget *column1, QWidget *column2) {
        addWidgetToFirstColumn(column1);
        addWidgetToSecondColumn(column2);
    }
protected:
    QVBoxLayout *mLayout1 = NULL;
    QVBoxLayout *mLayout2 = NULL;
};

class RenderSettingsDialog : public QDialog {
    Q_OBJECT
public:
    struct FormatCodecs {
        FormatCodecs(const QList<AVCodecID> &vidCodecs,
                     const QList<AVCodecID> &audioCodec,
                     const char *name);

        AVOutputFormat *mFormat = NULL;
        QList<AVCodecID> mVidCodecs;
        QList<AVCodecID> mAudioCodecs;
    };
    RenderSettingsDialog(const OutputSettings &settings,
                         QWidget *parent = NULL);
    OutputSettings getSettings() {
        OutputSettings settings = mInitialSettings;

        AVOutputFormat *currentOutputFormat = NULL;
        if(mVideoCodecsComboBox->count() > 0) {
            int formatId = mOutputFormatsComboBox->currentIndex();
            currentOutputFormat = mOutputFormatsList.at(formatId);
        }
        settings.outputFormat = currentOutputFormat;

        settings.videoEnabled = mVideoGroupBox->isChecked();
        AVCodec *currentVideoCodec = NULL;
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
        AVCodec *currentAudioCodec = NULL;
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
    QVBoxLayout *mMainLayout = NULL;

    QHBoxLayout *mOutputFormatsLayout = NULL;
    QLabel *mOutputFormatsLabel = NULL;
    QComboBox *mOutputFormatsComboBox = NULL;

    QGroupBox *mVideoGroupBox = NULL;
    TwoColumnLayout *mVideoSettingsLayout = NULL;
    QLabel *mVideoCodecsLabel = NULL;
    QComboBox *mVideoCodecsComboBox = NULL;
    QLabel *mPixelFormatsLabel = NULL;
    QComboBox *mPixelFormatsComboBox = NULL;
    QLabel *mBitrateLabel = NULL;
    QDoubleSpinBox *mBitrateSpinBox = NULL;

    QGroupBox *mAudioGroupBox = NULL;
    TwoColumnLayout *mAudioSettingsLayout = NULL;
    QLabel *mAudioCodecsLabel = NULL;
    QComboBox *mAudioCodecsComboBox = NULL;
    QLabel *mSampleFormatsLabel = NULL;
    QComboBox *mSampleFormatsComboBox = NULL;
    QLabel *mSampleRateLabel = NULL;
    QComboBox *mSampleRateComboBox = NULL;
    QLabel *mAudioBitrateLabel = NULL;
    QComboBox *mAudioBitrateComboBox = NULL;
    QLabel *mAudioChannelLayoutLabel = NULL;
    QComboBox *mAudioChannelLayoutsComboBox = NULL;

    QHBoxLayout *mShowLayout = NULL;
    QLabel *mShowLabel = NULL;
    QCheckBox *mShowAllFormatsAndCodecsCheckBox = NULL;

    QHBoxLayout *mButtonsLayout = NULL;
    QPushButton *mOkButton = NULL;
    QPushButton *mCancelButton = NULL;
    QPushButton *mResetButton = NULL;
    void addVideoCodec(const AVCodecID &codecId,
                       AVOutputFormat *outputFormat,
                       const QString &currentCodecName);
    void addAudioCodec(const AVCodecID &codecId,
                       AVOutputFormat *outputFormat,
                       const QString &currentCodecName);
};

#endif // RENDERSETTINGSDIALOG_H
