// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "outputsettingsdisplaywidget.h"
#include "GUI/global.h"

OutputSettingsDisplayWidget::OutputSettingsDisplayWidget(QWidget *parent) :
    QWidget(parent) {
//    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mAudioVideoLayout = new QHBoxLayout();
    mOutputFormatLabel = new QLabel("<b>Format:</b>", this);
    mVideoCodecLabel = new QLabel("<b>Video codec:</b>", this);
    mVideoPixelFormatLabel = new QLabel("<b>Pixel format:</b>", this);
    mVideoBitrateLabel = new QLabel("<b>Video bitrate:</b>", this);
    mAudioCodecLabel = new QLabel("<b>Audio codec:</b>", this);
    mAudioSampleRateLabel = new QLabel("<b>Audio sample rate:</b>", this);
    mAudioSampleFormatLabel = new QLabel("<b>Audio sample format:</b>", this);
    mAudioBitrateLabel = new QLabel("<b>Audio bitrate:</b>", this);
    mAudioChannelLayoutLabel = new QLabel("<b>Audio channel layout:</b>", this);

    mOutputFormatLabel->setFixedHeight(eSizesUI::widget);
    mVideoCodecLabel->setFixedHeight(eSizesUI::widget);
    mVideoPixelFormatLabel->setFixedHeight(eSizesUI::widget);
    mVideoBitrateLabel->setFixedHeight(eSizesUI::widget);
    mAudioCodecLabel->setFixedHeight(eSizesUI::widget);
    mAudioSampleRateLabel->setFixedHeight(eSizesUI::widget);
    mAudioSampleFormatLabel->setFixedHeight(eSizesUI::widget);
    mAudioBitrateLabel->setFixedHeight(eSizesUI::widget);
    mAudioChannelLayoutLabel->setFixedHeight(eSizesUI::widget);

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
    mAudioVideoLayout->setSpacing(eSizesUI::widget);
    mAudioLayout->setSpacing(0);
    mVideoLayout->setSpacing(0);
    mVideoLayout->setAlignment(Qt::AlignTop);

    mMainLayout->addWidget(mOutputFormatLabel);
    mMainLayout->addLayout(mAudioVideoLayout);
}

void OutputSettingsDisplayWidget::setOutputSettings(const OutputSettings &settings) {
    if(!settings.fOutputFormat) {
        setOutputFormatText("-");
    } else {
        setOutputFormatText(QString(settings.fOutputFormat->long_name));
    }
    if(!mAlwaysShowAll) {
        setVideoLabelsVisible(settings.fVideoEnabled);
    }
    if(!settings.fVideoCodec) {
        setVideoCodecText("-");
    } else {
        setVideoCodecText(QString(settings.fVideoCodec->long_name));
    }
    const char *pixelFormat = av_get_pix_fmt_name(settings.fVideoPixelFormat);
    if(!pixelFormat) {
        setPixelFormatText("-");
    } else {
        setPixelFormatText(QString(pixelFormat));
    }
    setVideoBitrateText(QString::number(settings.fVideoBitrate/1000000) + " Mbps");
    if(!mAlwaysShowAll) {
        setAudioLabelsVisible(settings.fAudioEnabled);
    }
    if(!settings.fAudioCodec) {
        setAudioCodecText("-");
    } else {
        setAudioCodecText(QString(settings.fAudioCodec->long_name));
    }
    setAudioSampleRateText(QString::number(settings.fAudioSampleRate) + " Hz");
    int formatId = settings.fAudioSampleFormat;
    if(OutputSettings::sSampleFormatNames.find(formatId) ==
            OutputSettings::sSampleFormatNames.end()) {
        setAudioSampleFormatText("-");
    } else {
        setAudioSampleFormatText(OutputSettings::sSampleFormatNames.at(formatId));
    }
    setAudioBitrateText(QString::number(settings.fAudioBitrate/1000) + " kbps");

    const auto channLay = OutputSettings::sGetChannelsLayoutName(
                settings.fAudioChannelsLayout);
    setAudioChannelLayoutText(channLay);
}
