#include "rendersettingsdialog.h"
#include "GUI/mainwindow.h"
#include "global.h"

RenderSettingsDialog::RenderSettingsDialog(const OutputSettings &settings,
                                           QWidget *parent) :
    QDialog(parent) {
    mSupportedFormats = {
        FormatCodecs(QList<AVCodecID>() << AV_CODEC_ID_PNG << AV_CODEC_ID_TIFF << AV_CODEC_ID_MJPEG << AV_CODEC_ID_LJPEG,
                     QList<AVCodecID>(),
                     "*.jpg"),
        FormatCodecs(QList<AVCodecID>() << AV_CODEC_ID_PNG << AV_CODEC_ID_QTRLE << AV_CODEC_ID_H264,
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
                     "*.mov"),
        FormatCodecs(QList<AVCodecID>() << AV_CODEC_ID_H264 << AV_CODEC_ID_MPEG4 << AV_CODEC_ID_MPEG2VIDEO << AV_CODEC_ID_HEVC,
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
                     "*.mp4"),
        FormatCodecs(QList<AVCodecID>() << AV_CODEC_ID_H264 << AV_CODEC_ID_MPEG4 << AV_CODEC_ID_MPEG2VIDEO << AV_CODEC_ID_HEVC,
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
        "*.mkv"),
        FormatCodecs(QList<AVCodecID>() << AV_CODEC_ID_H264 << AV_CODEC_ID_MPEG4 << AV_CODEC_ID_MPEG2VIDEO << AV_CODEC_ID_HEVC << AV_CODEC_ID_RAWVIDEO << AV_CODEC_ID_LJPEG,
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
        "*.avi"),
        FormatCodecs(QList<AVCodecID>(),
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
        "*.mp3"),
        FormatCodecs(QList<AVCodecID>(),
                     QList<AVCodecID>() << AV_CODEC_ID_FLAC,
        "*.flac"),
        FormatCodecs(QList<AVCodecID>(),
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK << AV_CODEC_ID_PCM_U8 << AV_CODEC_ID_PCM_S16LE << AV_CODEC_ID_PCM_S24LE << AV_CODEC_ID_PCM_S32LE << AV_CODEC_ID_PCM_S64LE << AV_CODEC_ID_PCM_F32LE << AV_CODEC_ID_PCM_F64LE,
        "*.wav"),
        FormatCodecs(QList<AVCodecID>(),
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
        "*.ogg"),
        FormatCodecs(QList<AVCodecID>(),
                     QList<AVCodecID>() << AV_CODEC_ID_MP3 << AV_CODEC_ID_AAC << AV_CODEC_ID_AC3 << AV_CODEC_ID_FLAC << AV_CODEC_ID_VORBIS << AV_CODEC_ID_WAVPACK,
        "*.aiff")
    };

    setStyleSheet(MainWindow::getInstance()->styleSheet());
    mInitialSettings = settings;

    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mOutputFormatsLayout = new QHBoxLayout();
    mOutputFormatsLabel = new QLabel("Format:", this);
    mOutputFormatsComboBox = new QComboBox(this);
    mOutputFormatsLayout->addWidget(mOutputFormatsLabel);
    mOutputFormatsLayout->addWidget(mOutputFormatsComboBox);

    mVideoGroupBox = new QGroupBox("Video", this);
    mVideoGroupBox->setCheckable(true);
    mVideoGroupBox->setChecked(true);
    mVideoSettingsLayout = new TwoColumnLayout();
    mVideoCodecsLabel = new QLabel("Codec:", this);
    mPixelFormatsLabel = new QLabel("Pixel format:", this);
    mBitrateLabel = new QLabel("Bitrate:", this);

    mVideoCodecsComboBox = new QComboBox(this);
    mPixelFormatsComboBox = new QComboBox(this);
    mBitrateSpinBox = new QDoubleSpinBox(this);
    mBitrateSpinBox->setRange(0.1, 100.);
    mBitrateSpinBox->setSuffix(" Mbps");

    mVideoSettingsLayout->addPair(mVideoCodecsLabel,
                                  mVideoCodecsComboBox);
    mVideoSettingsLayout->addPair(mPixelFormatsLabel,
                                  mPixelFormatsComboBox);
    mVideoSettingsLayout->addPair(mBitrateLabel,
                                  mBitrateSpinBox);

    mAudioGroupBox = new QGroupBox("Audio", this);
    mAudioGroupBox->setCheckable(true);
    mAudioSettingsLayout = new TwoColumnLayout();
    mAudioCodecsLabel = new QLabel("Codec:", this);
    mSampleRateLabel = new QLabel("Sample rate:", this);
    mSampleFormatsLabel = new QLabel("Sample format:", this);
    mAudioBitrateLabel = new QLabel("Bitrate:", this);
    mAudioChannelLayoutLabel = new QLabel("Channels:", this);

    mAudioCodecsComboBox = new QComboBox(this);
    mSampleRateComboBox = new QComboBox(this);
    mSampleFormatsComboBox = new QComboBox(this);
    mAudioBitrateComboBox = new QComboBox(this);
    mAudioChannelLayoutsComboBox = new QComboBox(this);

    mAudioSettingsLayout->addPair(mAudioCodecsLabel,
                                  mAudioCodecsComboBox);
    mAudioSettingsLayout->addPair(mSampleRateLabel,
                                  mSampleRateComboBox);
    mAudioSettingsLayout->addPair(mSampleFormatsLabel,
                                  mSampleFormatsComboBox);
    mAudioSettingsLayout->addPair(mAudioBitrateLabel,
                                  mAudioBitrateComboBox);
    mAudioSettingsLayout->addPair(mAudioChannelLayoutLabel,
                                  mAudioChannelLayoutsComboBox);

    mShowLayout = new QHBoxLayout();
    mShowLabel = new QLabel("Show all formats and codecs", this);
    mShowAllFormatsAndCodecsCheckBox = new QCheckBox(this);
    connect(mShowAllFormatsAndCodecsCheckBox, &QCheckBox::toggled,
            this, &RenderSettingsDialog::setShowAllFormatsAndCodecs);
    mShowLayout->addWidget(mShowAllFormatsAndCodecsCheckBox);
    mShowLayout->addWidget(mShowLabel);

    mShowLayout->setAlignment(Qt::AlignRight);

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mResetButton = new QPushButton("Reset", this);
    connect(mOkButton, &QPushButton::released,
            this, &RenderSettingsDialog::accept);
    connect(mCancelButton, &QPushButton::released,
            this, &RenderSettingsDialog::reject);
    connect(mResetButton, &QPushButton::released,
            this, &RenderSettingsDialog::restoreInitialSettings);
    mButtonsLayout->addWidget(mResetButton, Qt::AlignLeft);
    mButtonsLayout->addWidget(mCancelButton, Qt::AlignLeft);
    mButtonsLayout->addWidget(mOkButton, Qt::AlignRight);

    mMainLayout->addLayout(mOutputFormatsLayout);
    mMainLayout->addSpacing(MIN_WIDGET_HEIGHT);
    mVideoGroupBox->setLayout(mVideoSettingsLayout);
    mMainLayout->addWidget(mVideoGroupBox);
    mAudioGroupBox->setLayout(mAudioSettingsLayout);
    mMainLayout->addWidget(mAudioGroupBox);
    mMainLayout->addSpacing(MIN_WIDGET_HEIGHT);
    mMainLayout->addLayout(mShowLayout);
    mMainLayout->addSpacing(MIN_WIDGET_HEIGHT);
    mMainLayout->addLayout(mButtonsLayout);

    connect(mVideoCodecsComboBox, &QComboBox::currentTextChanged,
            this, &RenderSettingsDialog::updateAvailablePixelFormats);

    connect(mAudioCodecsComboBox, &QComboBox::currentTextChanged,
    [this]() {
        updateAvailableSampleFormats();
        updateAvailableSampleRates();
        updateAvailableAudioBitrates();
        updateAvailableAudioChannelLayouts();
    });

    updateAvailableOutputFormats();
    restoreInitialSettings();
}

void RenderSettingsDialog::updateAvailablePixelFormats() {
    const QString currentFormatName = mPixelFormatsComboBox->currentText();
    AVCodec *currentCodec = nullptr;
    if(mVideoCodecsComboBox->count() > 0) {
        currentCodec = mVideoCodecsList.at(mVideoCodecsComboBox->currentIndex());
    }
    mPixelFormatsComboBox->clear();
    mPixelFormatsList.clear();
    if(currentCodec) {
        const AVPixelFormat *format = currentCodec->pix_fmts;
        if(!format) return;
        while(*format != -1) {
            mPixelFormatsList << *format;
            const QString formatName(av_get_pix_fmt_name(*format));
            mPixelFormatsComboBox->addItem(formatName);
            if(formatName == currentFormatName)
                mPixelFormatsComboBox->setCurrentText(formatName);
            format++;
        }
    }
}

void RenderSettingsDialog::addVideoCodec(const AVCodecID &codecId,
                                         AVOutputFormat *outputFormat,
                                         const QString &currentCodecName) {
    AVCodec *currentCodec = avcodec_find_encoder(codecId);
    if(!currentCodec) return;
    if(currentCodec->type != AVMEDIA_TYPE_VIDEO) return;
    if(currentCodec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) return;
    if(currentCodec->pix_fmts == nullptr) return;
    if(avformat_query_codec(outputFormat, codecId, COMPLIENCE) == 0) return;
    mVideoCodecsList << currentCodec;
    const QString codecName(currentCodec->long_name);
    mVideoCodecsComboBox->addItem(codecName);
    if(codecName == currentCodecName) {
        mVideoCodecsComboBox->setCurrentText(codecName);
    }
}

void RenderSettingsDialog::addAudioCodec(const AVCodecID &codecId,
                                         AVOutputFormat *outputFormat,
                                         const QString &currentCodecName) {
    AVCodec * const currentCodec = avcodec_find_encoder(codecId);
    if(!currentCodec) return;
    if(currentCodec->type != AVMEDIA_TYPE_AUDIO) return;
    if(currentCodec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) return;
    if(currentCodec->sample_fmts == nullptr) return;
    if(avformat_query_codec(outputFormat, codecId, COMPLIENCE) == 0) return;
    mAudioCodecsList << currentCodec;
    const QString codecName(currentCodec->long_name);
    mAudioCodecsComboBox->addItem(codecName);
    if(codecName == currentCodecName) {
        mAudioCodecsComboBox->setCurrentText(codecName);
    }
}

void RenderSettingsDialog::updateAvailableCodecs() {
    updateAvailableAudioCodecs();
    updateAvailableVideoCodecs();
}

void RenderSettingsDialog::updateAvailableVideoCodecs() {
    const QString currentCodecName = mVideoCodecsComboBox->currentText();
    mVideoCodecsComboBox->clear();
    mVideoCodecsList.clear();
    if(mOutputFormatsComboBox->count() == 0) return;
    const int outputFormatId = mOutputFormatsComboBox->currentIndex();
    AVOutputFormat * const outputFormat =
            mOutputFormatsList.at(outputFormatId);
    if(!outputFormat) return;

    if(mShowAllFormatsAndCodecs) {
        AVCodec *currentCodec = nullptr;
        do {
            currentCodec = av_codec_next(currentCodec);
            if(!currentCodec) break;
            if(currentCodec->type != AVMEDIA_TYPE_VIDEO) continue;
            if(currentCodec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) {
                continue;
            }
            if(currentCodec->pix_fmts == nullptr) continue;
            addVideoCodec(currentCodec->id,
                          outputFormat,
                          currentCodecName);
        } while(currentCodec != nullptr);
    } else {
        const FormatCodecs currFormatT =
                mSupportedFormats.at(outputFormatId);
        for(const AVCodecID &codecId : currFormatT.mVidCodecs) {
            AVCodec * const currentCodec = avcodec_find_encoder(codecId);
            if(!currentCodec) break;
            if(currentCodec->type != AVMEDIA_TYPE_VIDEO) continue;
            if(currentCodec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) {
                continue;
            }
            if(currentCodec->pix_fmts == nullptr) continue;
            addVideoCodec(currentCodec->id,
                          outputFormat,
                          currentCodecName);
        }
    }
    const bool noCodecs = mVideoCodecsComboBox->count() == 0;
    mVideoGroupBox->setChecked(!noCodecs);
    mVideoGroupBox->setDisabled(noCodecs);
}

void RenderSettingsDialog::updateAvailableAudioCodecs() {
    const QString currentCodecName =
            mAudioCodecsComboBox->currentText();
    mAudioCodecsComboBox->clear();
    mAudioCodecsList.clear();
    if(mOutputFormatsComboBox->count() == 0) return;
    const int outputFormatId = mOutputFormatsComboBox->currentIndex();
    AVOutputFormat * const outputFormat =
            mOutputFormatsList.at(outputFormatId);
    if(!outputFormat) return;
    if(mShowAllFormatsAndCodecs) {
        AVCodec *currentCodec = nullptr;
        do {
            currentCodec = av_codec_next(currentCodec);
            if(!currentCodec) break;
            if(currentCodec->type != AVMEDIA_TYPE_AUDIO) continue;
            if(currentCodec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) {
                continue;
            }
            if(currentCodec->sample_fmts == nullptr) continue;
            addAudioCodec(currentCodec->id,
                          outputFormat,
                          currentCodecName);
        } while(currentCodec != nullptr);
    } else {
        const FormatCodecs currFormatT =
                mSupportedFormats.at(outputFormatId);
        for(const AVCodecID &codecId : currFormatT.mAudioCodecs) {
            AVCodec * const currentCodec = avcodec_find_encoder(codecId);
            if(!currentCodec) break;
            if(currentCodec->type != AVMEDIA_TYPE_AUDIO) continue;
            if(currentCodec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) {
                continue;
            }
            if(currentCodec->sample_fmts == nullptr) continue;
            addAudioCodec(currentCodec->id,
                          outputFormat,
                          currentCodecName);
        }
    }
    bool noCodecs = mAudioCodecsComboBox->count() == 0;
    mAudioGroupBox->setChecked(!noCodecs);
    mAudioGroupBox->setDisabled(noCodecs);
}


void RenderSettingsDialog::updateAvailableOutputFormats() {
    disconnect(mOutputFormatsComboBox, &QComboBox::currentTextChanged,
               this, &RenderSettingsDialog::updateAvailableCodecs);

    QString currentFormatName = mOutputFormatsComboBox->currentText();

    mOutputFormatsComboBox->clear();
    mOutputFormatsList.clear();

    if(mShowAllFormatsAndCodecs) {
        AVOutputFormat *currentFormat = nullptr;
        do {
            currentFormat = av_oformat_next(currentFormat);
            if(!currentFormat) break;
            //if(!currentFormat->codec_tag || !currentFormat->codec_tag[0]) continue;
//            int supportedCodecs = 0;
//            for(const AVCodecID &codecId : VIDEO_CODECS) {
//                if(avformat_query_codec(currentFormat, codecId, COMPLIENCE) == 0) continue;
//                supportedCodecs++;
//                break;
//            }
//            for(const AVCodecID &codecId : AUDIO_CODECS) {
//                if(avformat_query_codec(currentFormat, codecId, COMPLIENCE) == 0) continue;
//                supportedCodecs++;
//                break;
//            }
//            if(supportedCodecs == 0) continue;
            //if(currentFormat->query_codec == nullptr) continue;
            mOutputFormatsList << currentFormat;
            QString formatName = QString(currentFormat->long_name);
            mOutputFormatsComboBox->addItem(formatName);
            if(formatName == currentFormatName) {
                mOutputFormatsComboBox->setCurrentText(formatName);
            }
        } while(currentFormat != nullptr);
    } else {
        for(const FormatCodecs &formatT : mSupportedFormats) {
            AVOutputFormat *currentFormat = formatT.mFormat;
            if(!currentFormat) break;
            //if(!currentFormat->codec_tag || !currentFormat->codec_tag[0]) continue;
//            int supportedCodecs = 0;
//            for(const AVCodecID &codecId : currentFormat->video_codec) {
//                if(avformat_query_codec(currentFormat, codecId, COMPLIENCE) == 0) continue;
//                supportedCodecs++;
//                break;
//            }
//            for(const AVCodecID &codecId : currentFormat->audio_codec) {
//                if(avformat_query_codec(currentFormat, codecId, COMPLIENCE) == 0) continue;
//                supportedCodecs++;
//                break;
//            }
//            if(supportedCodecs == 0) continue;
            //if(currentFormat->query_codec == nullptr) continue;
            mOutputFormatsList << currentFormat;
            QString formatName = QString(currentFormat->long_name);
            mOutputFormatsComboBox->addItem(formatName);
            if(formatName == currentFormatName) {
                mOutputFormatsComboBox->setCurrentText(formatName);
            }
        }
    }
    connect(mOutputFormatsComboBox, &QComboBox::currentTextChanged,
            this, &RenderSettingsDialog::updateAvailableCodecs);

    updateAvailableCodecs();
}

void RenderSettingsDialog::updateAvailableSampleFormats() {
    QString currentFormatName = mSampleFormatsComboBox->currentText();
    mSampleFormatsComboBox->clear();
    mSampleFormatsList.clear();
    AVCodec *currentCodec = nullptr;
    if(mAudioCodecsComboBox->count() > 0) {
        const int codecId = mAudioCodecsComboBox->currentIndex();
        currentCodec = mAudioCodecsList.at(codecId);
    }
    if(!currentCodec) return;
    const AVSampleFormat *sampleFormats = currentCodec->sample_fmts;
    if(!sampleFormats) return;
    while(*sampleFormats != -1) {
        mSampleFormatsList << *sampleFormats;
        QString formatName = OutputSettings::SAMPLE_FORMATS_NAMES.at(*sampleFormats);
        mSampleFormatsComboBox->addItem(formatName);
        if(formatName == currentFormatName) {
            mSampleFormatsComboBox->setCurrentText(formatName);
        }
        sampleFormats++;
    }
}

void RenderSettingsDialog::updateAvailableAudioBitrates() {
    mAudioBitrateComboBox->clear();
    mAudioBitrateList.clear();
    AVCodec *currentCodec = nullptr;
    if(mAudioCodecsComboBox->count() > 0) {
        int codecId = mAudioCodecsComboBox->currentIndex();
        currentCodec = mAudioCodecsList.at(codecId);
    }
    if(!currentCodec) return;
    if(currentCodec->capabilities & AV_CODEC_CAP_LOSSLESS) {
        mAudioBitrateList << 384000;
        mAudioBitrateComboBox->addItem("Loseless");
    } else {
        QList<int> ratesT = { 24, 32, 48, 64, 128, 160, 192, 320, 384 };
        for(const int rateT : ratesT) {
            mAudioBitrateComboBox->addItem(QString::number(rateT) + " kbps");
            mAudioBitrateList << rateT*1000;
        }
    }
}

void RenderSettingsDialog::updateAvailableSampleRates() {
    mSampleRateComboBox->clear();
    mSampleRatesList.clear();
    AVCodec *currentCodec = nullptr;
    if(mAudioCodecsComboBox->count() > 0) {
        int codecId = mAudioCodecsComboBox->currentIndex();
        currentCodec = mAudioCodecsList.at(codecId);
    }
    if(!currentCodec) return;
    const int *sampleRates = currentCodec->supported_samplerates;
    if(!sampleRates) {
        QList<int> ratesT = { 96000, 88200, 64000, 48000, 44100,
                              32000, 22050, 11025, 8000 };
        for(const int rateT : ratesT) {
            mSampleRateComboBox->addItem(QString::number(rateT) + " Hz");
            mSampleRatesList << rateT;
        }
    } else {
        int rateT = *sampleRates;
        while(rateT != 0) {
            mSampleRateComboBox->addItem(QString::number(rateT) + " Hz");
            mSampleRatesList << rateT;
            sampleRates++;
            rateT = *sampleRates;
        }
    }
}

void RenderSettingsDialog::updateAvailableAudioChannelLayouts() {
    mAudioChannelLayoutsComboBox->clear();
    mAudioChannelLayoutsList.clear();
    AVCodec *currentCodec = nullptr;
    if(mAudioCodecsComboBox->count() > 0) {
        int codecId = mAudioCodecsComboBox->currentIndex();
        currentCodec = mAudioCodecsList.at(codecId);
    }
    if(!currentCodec) return;
    const uint64_t *layouts = currentCodec->channel_layouts;
    if(!layouts) {
        mAudioChannelLayoutsList << AV_CH_LAYOUT_MONO;
        mAudioChannelLayoutsList << AV_CH_LAYOUT_STEREO;
        mAudioChannelLayoutsComboBox->addItem("Mono");
        mAudioChannelLayoutsComboBox->addItem("Stereo");
    } else {
        uint64_t layout = *layouts;
        while(layout != 0) {
            QString layoutName = OutputSettings::getChannelsLayoutNameStatic(layout);
            mAudioChannelLayoutsList << layout;
            mAudioChannelLayoutsComboBox->addItem(layoutName);
            layouts++;
            layout = *layouts;
        }
    }
}

void RenderSettingsDialog::restoreInitialSettings() {
    AVOutputFormat *currentOutputFormat = mInitialSettings.outputFormat;
    if(!currentOutputFormat) {
        mOutputFormatsComboBox->setCurrentIndex(0);
    } else {
        QString currentOutputFormatName = QString(currentOutputFormat->long_name);
        mOutputFormatsComboBox->setCurrentText(currentOutputFormatName);
    }
    AVCodec *currentVideoCodec = mInitialSettings.videoCodec;
    if(!currentVideoCodec) {
        mVideoCodecsComboBox->setCurrentIndex(0);
    } else {
        QString currentCodecName = QString(currentVideoCodec->long_name);
        mVideoCodecsComboBox->setCurrentText(currentCodecName);
    }
    AVPixelFormat currentFormat = mInitialSettings.videoPixelFormat;
    if(currentFormat == AV_PIX_FMT_NONE) {
        mPixelFormatsComboBox->setCurrentIndex(0);
    } else {
        QString currentFormatName = QString(av_get_pix_fmt_name(currentFormat));
        mPixelFormatsComboBox->setCurrentText(currentFormatName);
    }
    int currentBitrate = mInitialSettings.videoBitrate;
    if(currentBitrate <= 0) {
        mBitrateSpinBox->setValue(9.);
    } else {
        mBitrateSpinBox->setValue(currentBitrate/1000000.);
    }
    bool noVideoCodecs = mVideoCodecsComboBox->count() == 0;
    mVideoGroupBox->setChecked(mInitialSettings.videoEnabled &&
                               !noVideoCodecs);

    AVCodec *currentAudioCodec = mInitialSettings.audioCodec;
    if(!currentAudioCodec) {
        mAudioCodecsComboBox->setCurrentIndex(0);
    } else {
        QString currentCodecName = QString(currentAudioCodec->long_name);
        mAudioCodecsComboBox->setCurrentText(currentCodecName);
    }

    AVSampleFormat currentSampleFormat = mInitialSettings.audioSampleFormat;
    if(currentSampleFormat == AV_SAMPLE_FMT_NONE) {
        mSampleFormatsComboBox->setCurrentIndex(0);
    } else {
        QString currentFormatName = OutputSettings::SAMPLE_FORMATS_NAMES.at(currentSampleFormat);
        mSampleFormatsComboBox->setCurrentText(currentFormatName);
    }

    int currentSampleRate = mInitialSettings.audioSampleRate;
    if(currentSampleRate <= 0) {
        const int i48000Id = mSampleRateComboBox->findText("48000 Hz");
        if(i48000Id == -1) {
            const int i44100Id = mSampleRateComboBox->findText("44100 Hz");
            if(i44100Id == -1) {
                int lastId = mSampleRateComboBox->count() - 1;
                mSampleRateComboBox->setCurrentIndex(lastId);
            } else {
                mSampleRateComboBox->setCurrentIndex(i44100Id);
            }
        } else {
            mSampleRateComboBox->setCurrentIndex(i48000Id);
        }
    } else {
        const QString smplsStr = QString::number(currentSampleRate) + " Hz";
        mSampleRateComboBox->setCurrentText(smplsStr);
    }

    int currentAudioBitrate = mInitialSettings.audioBitrate;
    if(currentAudioBitrate <= 0) {
        int i320Id = mAudioBitrateComboBox->findText("320 kbps");
        if(i320Id == -1) {
            int lastId = mAudioBitrateComboBox->count() - 1;
            mAudioBitrateComboBox->setCurrentIndex(lastId);
        } else {
            mAudioBitrateComboBox->setCurrentIndex(i320Id);
        }
    } else {
        const QString btrtStr = QString::number(currentAudioBitrate/1000) + " kbps";
        mAudioBitrateComboBox->setCurrentText(btrtStr);
    }

    uint64_t currentChannelsLayout = mInitialSettings.audioChannelsLayout;
    if(currentChannelsLayout == 0) {
        mAudioChannelLayoutsComboBox->setCurrentIndex(0);
    } else {
        const QString channLayStr =
                OutputSettings::getChannelsLayoutNameStatic(currentChannelsLayout);
        if(channLayStr == "-") {
            mAudioChannelLayoutsComboBox->setCurrentIndex(0);
        } else {
            mAudioBitrateComboBox->setCurrentText(channLayStr);
        }
    }

    bool noAudioCodecs = mAudioCodecsComboBox->count() == 0;
    mAudioGroupBox->setChecked(mInitialSettings.audioEnabled &&
                               !noAudioCodecs);
}


#ifndef AVFORMAT_INTERNAL_H
#define AVFORMAT_INTERNAL_H
typedef struct AVCodecTag {
    enum AVCodecID id;
    unsigned int tag;
} AVCodecTag;
#endif

RenderSettingsDialog::FormatCodecs::FormatCodecs(
        const QList<AVCodecID> &vidCodecs,
        const QList<AVCodecID> &audioCodec, const char *name) {
    mFormat = av_guess_format(nullptr, name, nullptr);
//    bool defVidAdded = false;
//    bool defAudAdded = false;
    for(const AVCodecID &vidCodec : vidCodecs) {
        if(avformat_query_codec(mFormat, vidCodec, COMPLIENCE) == 0) continue;
        mVidCodecs << vidCodec;
//        if(vidCodec == mFormat->video_codec) defVidAdded = true;
    }
    for(const AVCodecID &audCodec : audioCodec) {
        if(avformat_query_codec(mFormat, audCodec, COMPLIENCE) == 0) continue;
        mAudioCodecs << audCodec;
//        if(audCodec == mFormat->audio_codec) defAudAdded = true;
    }
//    if(!defAudAdded) {
//        if(mFormat->audio_codec != AV_CODEC_ID_NONE) {
//            mAudioCodecs << mFormat->audio_codec;
//        }
//    }
//    if(!defVidAdded) {
//        if(mFormat->video_codec != AV_CODEC_ID_NONE) {
//            mVidCodecs << mFormat->video_codec;
//        }
//    }

//    for(int i = 0; mFormat->codec_tag && mFormat->codec_tag[i]; i++) {
//        const AVCodecTag *tags2 = mFormat->codec_tag[i];
//        for(int j = 0; tags2[j].id != AV_CODEC_ID_NONE; j++) {
//            AVCodecID codecId = tags2[j].id;
//            if(avformat_query_codec(mFormat, codecId, COMPLIENCE) == 0) continue;
//            AVCodec *codecT = avcodec_find_encoder(codecId);
//            if(!codecT) continue;
//            if(codecT->type == AVMEDIA_TYPE_VIDEO) {
//                if(!mVidCodecs.contains(codecId)) {
//                    mVidCodecs << codecId;
//                }
//            } else if(codecT->type == AVMEDIA_TYPE_AUDIO) {
//                if(!mAudioCodecs.contains(codecId)) {
//                    mAudioCodecs << codecId;
//                }
//            }
//        }
//    }
}
