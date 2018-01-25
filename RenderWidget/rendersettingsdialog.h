#ifndef RENDERSETTINGSDIALOG_H
#define RENDERSETTINGSDIALOG_H
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include "renderinstancesettings.h"

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
    RenderSettingsDialog(const RenderInstanceSettings &settings,
                         QWidget *parent = NULL);
    RenderInstanceSettings getSettings() {
        RenderInstanceSettings settings = mInitialSettings;

        AVOutputFormat *currentOutputFormat = NULL;
        if(mVideoCodecsComboBox->count() > 0) {
            int formatId = mOutputFormatsComboBox->currentIndex();
            currentOutputFormat = mOutputFormatsList.at(formatId);
        }
        settings.setOutputFormat(currentOutputFormat);

        settings.setVideoEnabled(mVideoGroupBox->isChecked());
        AVCodec *currentVideoCodec = NULL;
        if(mVideoCodecsComboBox->count() > 0) {
            int codecId = mVideoCodecsComboBox->currentIndex();
            currentVideoCodec = mVideoCodecsList.at(codecId);
        }
        settings.setVideoCodec(currentVideoCodec);

        AVPixelFormat currentPixelFormat = AV_PIX_FMT_NONE;
        if(mPixelFormatsList.count() > 0) {
            int formatId = mPixelFormatsComboBox->currentIndex();
            currentPixelFormat = mPixelFormatsList.at(formatId);
        }
        settings.setVideoPixelFormat(currentPixelFormat);
        settings.setVideoBitrate(mBitrateSpinBox->value()*1000000);

        settings.setAudioEnabled(mAudioGroupBox->isChecked());
        AVCodec *currentAudioCodec = NULL;
        if(mAudioCodecsComboBox->count() > 0) {
            int codecId = mAudioCodecsComboBox->currentIndex();
            currentAudioCodec = mAudioCodecsList.at(codecId);
        }
        settings.setAudioCodec(currentAudioCodec);

        AVSampleFormat currentSampleFormat = AV_SAMPLE_FMT_NONE;
        if(mSampleFormatsComboBox->count() > 0) {
            int formatId = mSampleFormatsComboBox->currentIndex();
            currentSampleFormat = mSampleFormatsList.at(formatId);
        }
        settings.setAudioSampleFormat(currentSampleFormat);

        int currentSampleRate = 0;
        if(mSampleRateComboBox->count() > 0) {
            int sampleRateId = mSampleRateComboBox->currentIndex();
            currentSampleRate = mSampleRatesList.at(sampleRateId);
        }
        settings.setAudioSampleRate(currentSampleRate);

        int currentAudioBitrate = 0;
        if(mAudioBitrateComboBox->count() > 0) {
            int bitrateId = mAudioBitrateComboBox->currentIndex();
            currentAudioBitrate = mAudioBitrateList.at(bitrateId);
        }
        settings.setAudioBitrate(currentAudioBitrate);

        uint64_t currentChannelsLayout = 0;
        if(mAudioChannelLayoutsComboBox->count() > 0) {
            int layoutId = mAudioChannelLayoutsComboBox->currentIndex();
            currentAudioBitrate = mAudioChannelLayoutsList.at(layoutId);
        }
        settings.setAudioChannelsLayout(currentChannelsLayout);

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
protected:
    QList<AVPixelFormat> mPixelFormatsList;
    QList<AVCodec*> mVideoCodecsList;
    QList<AVOutputFormat*> mOutputFormatsList;
    QList<AVCodec*> mAudioCodecsList;
    QList<int> mSampleRatesList;
    QList<AVSampleFormat> mSampleFormatsList;
    QList<int> mAudioBitrateList;
    QList<uint64_t> mAudioChannelLayoutsList;

    RenderInstanceSettings mInitialSettings;
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

    QHBoxLayout *mButtonsLayout = NULL;
    QPushButton *mOkButton = NULL;
    QPushButton *mCancelButton = NULL;
    QPushButton *mResetButton = NULL;
};

#endif // RENDERSETTINGSDIALOG_H
