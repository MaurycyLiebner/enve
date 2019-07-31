#include "outputsettingsdisplaywidget.h"
#include "GUI/global.h"

OutputSettingsDisplayWidget::OutputSettingsDisplayWidget(QWidget *parent) :
    QWidget(parent) {
//    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mAudioVideoLayout = new QHBoxLayout();
    mOutputFormatLabel = new QLabel("Format:", this);
    mVideoCodecLabel = new QLabel("Video codec:", this);
    mVideoPixelFormatLabel = new QLabel("Pixel format:", this);
    mVideoBitrateLabel = new QLabel("Video bitrate:", this);
    mAudioCodecLabel = new QLabel("Audio codec:", this);
    mAudioSampleRateLabel = new QLabel("Audio sample rate:", this);
    mAudioSampleFormatLabel = new QLabel("Audio sample format:", this);
    mAudioBitrateLabel = new QLabel("Audio bitrate:", this);
    mAudioChannelLayoutLabel = new QLabel("Audio channel layout:", this);

    mOutputFormatLabel->setFixedHeight(MIN_WIDGET_DIM);
    mVideoCodecLabel->setFixedHeight(MIN_WIDGET_DIM);
    mVideoPixelFormatLabel->setFixedHeight(MIN_WIDGET_DIM);
    mVideoBitrateLabel->setFixedHeight(MIN_WIDGET_DIM);
    mAudioCodecLabel->setFixedHeight(MIN_WIDGET_DIM);
    mAudioSampleRateLabel->setFixedHeight(MIN_WIDGET_DIM);
    mAudioSampleFormatLabel->setFixedHeight(MIN_WIDGET_DIM);
    mAudioBitrateLabel->setFixedHeight(MIN_WIDGET_DIM);
    mAudioChannelLayoutLabel->setFixedHeight(MIN_WIDGET_DIM);

    mVideoLayout = new QVBoxLayout();
    mVideoLayout->addWidget(mVideoCodecLabel);
    mVideoLayout->addWidget(mVideoPixelFormatLabel);
    mVideoLayout->addWidget(mVideoBitrateLabel);

    mAudioLayout = new QVBoxLayout();
    mAudioLayout->addWidget(mAudioCodecLabel);
    mAudioLayout->addWidget(mAudioSampleRateLabel);
    mAudioLayout->addWidget(mAudioSampleFormatLabel);
    mAudioLayout->addWidget(mAudioBitrateLabel);
    mAudioLayout->addWidget(mAudioChannelLayoutLabel);

    mAudioVideoLayout->addLayout(mVideoLayout);
    mAudioVideoLayout->addLayout(mAudioLayout);
    mAudioVideoLayout->setAlignment(mVideoLayout, Qt::AlignLeft);
    mAudioVideoLayout->setAlignment(mAudioLayout, Qt::AlignLeft);
    mAudioVideoLayout->setAlignment(Qt::AlignLeft);
    mAudioVideoLayout->setSpacing(MIN_WIDGET_DIM);
    mAudioLayout->setSpacing(0);
    mVideoLayout->setSpacing(0);
    mVideoLayout->setAlignment(Qt::AlignTop);

    mMainLayout->addWidget(mOutputFormatLabel);
    mMainLayout->addLayout(mAudioVideoLayout);
}
