#include "renderinstancewidget.h"
#include "global.h"
#include <QFileDialog>
#include "rendersettingsdialog.h"

RenderInstanceWidget::RenderInstanceWidget(QWidget *parent) :
    ClosableContainer(parent) {
    setStyleSheet("QWidget { background: rgb(45, 45, 45); }"
                  "QPushButton {"
                      "color: lightblue;"
                      "background: transparent;"
                      "border: 0;"
                  "}"
                  "QPushButton:hover {"
                      "text-decoration: underline;"
                  "}");

    mNameLabel = new QLabel(this);
    mNameLabel->setFixedHeight(MIN_WIDGET_HEIGHT);

    setLabelWidget(mNameLabel);

    QWidget *contWid = new QWidget(this);
    contWid->setLayout(mContentLayout);
    contWid->setStyleSheet("QWidget { background: rgb(45, 45, 45); }");
    addContentWidget(contWid);

    ClosableContainer *renderSettings = new ClosableContainer();

    QWidget *renderSettingsLabelWidget = new QWidget();
    QHBoxLayout *renderSettingsLayout = new QHBoxLayout();
    renderSettingsLayout->setAlignment(Qt::AlignLeft);
    renderSettingsLayout->setSpacing(0);
    renderSettingsLayout->setMargin(0);

    renderSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);
    QLabel *renderSettingsLabel = new QLabel("Render Settings:", this);
    renderSettingsLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    renderSettingsLayout->addWidget(renderSettingsLabel);
    renderSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);

    QPushButton *renderSettingsButton = new QPushButton("Settings");
    renderSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Minimum);
    connect(renderSettingsButton, SIGNAL(pressed()),
            this, SLOT(openRenderSettingsDialog()));
    renderSettingsLayout->addWidget(renderSettingsButton);

    renderSettingsLabelWidget->setLayout(renderSettingsLayout);
    renderSettings->setLabelWidget(renderSettingsLabelWidget);

    mContentLayout->addWidget(renderSettings);

    mOutputSettings = new ClosableContainer();
    mOutputFormatLabel = new QLabel("Format:", this);
    mVideoCodecLabel = new QLabel("Video codec:", this);
    mVideoPixelFormatLabel = new QLabel("Pixel format:", this);
    mVideoBitrateLabel = new QLabel("Video bitrate:", this);
    mAudioCodecLabel = new QLabel("Audio codec:", this);
    mAudioSampleRateLabel = new QLabel("Audio sample rate:", this);
    mAudioSampleFormatLabel = new QLabel("Audio sample format:", this);
    mAudioBitrateLabel = new QLabel("Audio bitrate:", this);
    mAudioChannelLayoutLabel = new QLabel("Audio channel layout:", this);
    mOutputSettings->addContentWidget(mOutputFormatLabel);
    mOutputSettings->addContentWidget(mVideoCodecLabel);
    mOutputSettings->addContentWidget(mVideoPixelFormatLabel);
    mOutputSettings->addContentWidget(mVideoBitrateLabel);
    mOutputSettings->addContentWidget(mAudioCodecLabel);
    mOutputSettings->addContentWidget(mAudioSampleRateLabel);
    mOutputSettings->addContentWidget(mAudioSampleFormatLabel);
    mOutputSettings->addContentWidget(mAudioBitrateLabel);
    mOutputSettings->addContentWidget(mAudioChannelLayoutLabel);

    QWidget *outputSettingsLabelWidget = new QWidget();
    QHBoxLayout *outputSettingsLayout = new QHBoxLayout();
    outputSettingsLayout->setAlignment(Qt::AlignLeft);
    outputSettingsLayout->setSpacing(0);
    outputSettingsLayout->setMargin(0);

    outputSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);
    QLabel *outputSettingsLabel = new QLabel("Output Settings:", this);
    outputSettingsLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    outputSettingsLabel->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Minimum);
    outputSettingsLayout->addWidget(outputSettingsLabel);
    outputSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);

    mOutputSettingsButton = new QPushButton("Settings");
    mOutputSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Minimum);
    connect(mOutputSettingsButton, SIGNAL(pressed()),
            this, SLOT(openOutputSettingsDialog()));
    outputSettingsLayout->addWidget(mOutputSettingsButton);

    outputSettingsLayout->addSpacing(6*MIN_WIDGET_HEIGHT);

    QLabel *outputDestinationLabel = new QLabel("Output Destination:", this);
    outputDestinationLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    outputDestinationLabel->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Minimum);
    outputSettingsLayout->addWidget(outputDestinationLabel);
    outputSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);

    mOutputDestinationButton = new QPushButton("Destination");
    mOutputDestinationButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Minimum);
    connect(mOutputDestinationButton, SIGNAL(pressed()),
            this, SLOT(openOutputDestinationDialog()));
    outputSettingsLayout->addWidget(mOutputDestinationButton);


    outputSettingsLabelWidget->setLayout(outputSettingsLayout);

    mOutputSettings->setLabelWidget(outputSettingsLabelWidget);

    mContentLayout->addWidget(mOutputSettings);

    mContentLayout->setMargin(0);
    mContentLayout->setSpacing(0);
}

RenderInstanceWidget::RenderInstanceWidget(RenderInstanceSettings *settings,
                                           QWidget *parent) :
    RenderInstanceWidget(parent) {
    mSettings = settings;
    updateFromSettings();
}

RenderInstanceWidget::~RenderInstanceWidget() {
    delete mSettings;
}

void RenderInstanceWidget::updateFromSettings() {
    mNameLabel->setText("  " + mSettings->getName());
}

RenderInstanceSettings *RenderInstanceWidget::getSettings() {
    return mSettings;
}

void RenderInstanceWidget::openOutputSettingsDialog() {
    RenderSettingsDialog *dialog = new RenderSettingsDialog(*mSettings, this);
    if(dialog->exec()) {
        *mSettings = dialog->getSettings();
        mOutputSettingsButton->setText(QString(mSettings->getOutputFormat()->long_name));
        updateOutputDestinationFromCurrentFormat();
    }
    delete dialog;
}

void RenderInstanceWidget::updateOutputDestinationFromCurrentFormat() {
    QString outputDst = mSettings->getOutputDestination();
    if(outputDst.isEmpty()) {
        outputDst = QDir::homePath() + "/untitled";
    }
    AVOutputFormat *format = mSettings->getOutputFormat();
    QString tmpStr = QString(format->extensions);
    QStringList supportedExt = tmpStr.split(",");
    QString fileName = outputDst.split("/").last();
    QStringList dividedName = fileName.split(".");
    QString currExt;
    if(dividedName.count() > 1) {
        QString namePart = dividedName.at(dividedName.count() - 2);
        if(namePart.count() > 0) {
            currExt = dividedName.last();
        }
    }
    if(supportedExt.contains(currExt)) return;
    if(!supportedExt.isEmpty()) {
        QString firstSupported = supportedExt.first();
        if(!firstSupported.isEmpty()) {
            if(currExt.isEmpty()) {
                if(outputDst.right(1) == ".") {
                    outputDst = outputDst.left(outputDst.count() - 1);
                }
            } else {
                int extId = outputDst.lastIndexOf("." + currExt);
                outputDst.remove(extId, 1 + currExt.count());
            }
            outputDst += "." + firstSupported;
            mSettings->setOutputDestination(outputDst);
        }
    }
    mOutputDestinationButton->setText(outputDst);
}

void RenderInstanceWidget::openOutputDestinationDialog() {
    QString supportedExts;
    QString selectedExt;
    AVOutputFormat *format = mSettings->getOutputFormat();
    if(format != NULL) {
        QString tmpStr = QString(format->extensions);
        selectedExt = "." + tmpStr.split(",").first();
        tmpStr.replace(",", " *.");
        supportedExts = "Output File (*." + tmpStr + ")";
    }
    QString iniText = mSettings->getOutputDestination();
    if(iniText.isEmpty()) iniText = QDir::homePath() + "/untitled" + selectedExt;
    QString saveAs = QFileDialog::getSaveFileName(this, "Output Destination",
                               iniText,
                               supportedExts);
    if(saveAs.isEmpty()) return;
    mSettings->setOutputDestination(saveAs);
    mOutputDestinationButton->setText(saveAs);
}

void RenderInstanceWidget::openRenderSettingsDialog() {

}
