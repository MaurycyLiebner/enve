#include "renderwidget.h"
#include "canvas.h"
#include "global.h"
#include "renderinstancewidget.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "videoencoder.h"

RenderWidget::RenderWidget(QWidget *parent) : QWidget(parent) {
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(0);
    setLayout(mMainLayout);
    mRenderProgressBar = new QProgressBar(this);
    mRenderProgressBar->setStyleSheet("border-top: 0;"
                                      "border-right: 0;"
                                      "border-left: 0;");
    mRenderProgressBar->setFixedHeight(8);
    mRenderProgressBar->setValue(0);
    mMainLayout->addWidget(mRenderProgressBar);

    mButtonsLayout = new QHBoxLayout();
    mButtonsLayout->setMargin(MIN_WIDGET_DIM/2);
    mButtonsLayout->addSpacing(MIN_WIDGET_DIM/2);
    mCurrentRenderLabel = new QLabel("Current Render", this);
    mCurrentRenderLabel->setStyleSheet("background: rgb(30, 30, 30);"
                                       "font-weight: bold;"
                                       "padding-left: 10px;");
    mButtonsLayout->addWidget(mCurrentRenderLabel);

    mStartRenderButton = new QPushButton("Render", this);
    mButtonsLayout->addWidget(mStartRenderButton, Qt::AlignRight);
    mStartRenderButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(mStartRenderButton, SIGNAL(pressed()),
            this, SLOT(render()));

    mPauseRenderButton = new QPushButton("Pause", this);
    mButtonsLayout->addWidget(mPauseRenderButton, Qt::AlignRight);
    mPauseRenderButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    mStopRenderButton = new QPushButton("Stop", this);
    mStopRenderButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    mButtonsLayout->addWidget(mStopRenderButton, Qt::AlignRight);
    mButtonsLayout->addSpacing(MIN_WIDGET_DIM/2);
    connect(mStopRenderButton, SIGNAL(pressed()),
            this, SLOT(stopRendering()));

    mMainLayout->addLayout(mButtonsLayout);

    mContWidget = new QWidget(this);
    mContLayout = new QVBoxLayout(mContWidget);
    mContLayout->setMargin(0);
    mContWidget->setLayout(mContLayout);
    mContLayout->setAlignment(Qt::AlignTop);
    mScrollArea = new ScrollArea(this);
    mScrollArea->setStyleSheet("QScrollArea {"
                                   "border: 1px solid black;"
                               "}");
    mScrollArea->setWidget(mContWidget);
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mMainLayout->addWidget(mScrollArea);

    VideoEncoderEmitter *vidEmitter = VideoEncoder::getVideoEncoderEmitter();
    connect(vidEmitter, &VideoEncoderEmitter::encodingStarted,
            this, &RenderWidget::leaveOnlyInterruptionButtonsEnabled);

    connect(vidEmitter, &VideoEncoderEmitter::encodingFinished,
            this, &RenderWidget::leaveOnlyStartRenderButtonEnabled);
    connect(vidEmitter, &VideoEncoderEmitter::encodingFinished,
            this, &RenderWidget::sendNextForRender);

    connect(vidEmitter, &VideoEncoderEmitter::encodingInterrupted,
            this, &RenderWidget::clearAwaitingRender);
    connect(vidEmitter, &VideoEncoderEmitter::encodingInterrupted,
            this, &RenderWidget::leaveOnlyStartRenderButtonEnabled);

    connect(vidEmitter, &VideoEncoderEmitter::encodingFailed,
            this, &RenderWidget::leaveOnlyStartRenderButtonEnabled);
    connect(vidEmitter, &VideoEncoderEmitter::encodingFailed,
            this, &RenderWidget::sendNextForRender);

    connect(vidEmitter, &VideoEncoderEmitter::encodingStartFailed,
            this, &RenderWidget::leaveOnlyStartRenderButtonEnabled);
    connect(vidEmitter, &VideoEncoderEmitter::encodingStartFailed,
            this, &RenderWidget::sendNextForRender);
}

void RenderWidget::createNewRenderInstanceWidgetForCanvas(Canvas *canvas) {
    RenderInstanceSettings *settings = new RenderInstanceSettings(canvas);
    RenderInstanceWidget *wid = new RenderInstanceWidget(settings, this);
    mContLayout->addWidget(wid);
    mRenderInstanceWidgets << wid;
}

void RenderWidget::removeRenderInstanceWidget(RenderInstanceWidget *wid) {
    mRenderInstanceWidgets.removeOne(wid);
    delete wid;
}

void RenderWidget::setRenderedFrame(const int frame) {
    if(!mCurrentRenderedSettings) return;
    mRenderProgressBar->setValue(frame);
}

void RenderWidget::render(RenderInstanceSettings *settings) {
    const RenderSettings &renderSettings = settings->getRenderSettings();
    mRenderProgressBar->setRange(renderSettings.fMinFrame,
                                 renderSettings.fMaxFrame);
    mRenderProgressBar->setValue(renderSettings.fMinFrame);
    mCurrentRenderedSettings = settings;
    emit renderFromSettings(settings);
}

void RenderWidget::leaveOnlyInterruptionButtonsEnabled() {
    mStartRenderButton->setDisabled(true);
    mPauseRenderButton->setEnabled(true);
    mStopRenderButton->setEnabled(true);
}

void RenderWidget::leaveOnlyStartRenderButtonEnabled() {
    mStartRenderButton->setEnabled(true);
    mPauseRenderButton->setDisabled(true);
    mStopRenderButton->setDisabled(true);
}

void RenderWidget::disableButtons() {
    mStartRenderButton->setDisabled(true);
    mPauseRenderButton->setDisabled(true);
    mStopRenderButton->setDisabled(true);
}

void RenderWidget::enableButtons() {
    mStartRenderButton->setEnabled(true);
    mPauseRenderButton->setEnabled(true);
    mStopRenderButton->setEnabled(true);
}

void RenderWidget::render() {
    RenderInstanceWidget *firstWid = nullptr;
    for(RenderInstanceWidget *wid : mRenderInstanceWidgets) {
        if(!wid->isChecked()) continue;
        if(!firstWid) {
            firstWid = wid;
        } else {
            mAwaitingSettings << wid;
            wid->getSettings()->setCurrentState(RenderInstanceSettings::WAITING);
        }
    }
    if(firstWid) {
        disableButtons();
        firstWid->setDisabled(true);
        render(firstWid->getSettings());
    }
}
#include "videoencoder.h"
void RenderWidget::stopRendering() {
    disableButtons();
    clearAwaitingRender();
    VideoEncoder::sInterruptEncoding();
}

void RenderWidget::clearAwaitingRender() {
    for(RenderInstanceWidget *wid : mAwaitingSettings) {
        wid->getSettings()->setCurrentState(RenderInstanceSettings::NONE);
    }
    mAwaitingSettings.clear();
}

void RenderWidget::sendNextForRender() {
    if(mAwaitingSettings.isEmpty()) return;
    RenderInstanceWidget *wid = mAwaitingSettings.takeFirst();
    if(wid->isChecked()) {
        wid->setDisabled(true);
        render(wid->getSettings());
    } else {
        sendNextForRender();
    }
}
