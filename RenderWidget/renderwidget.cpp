#include "renderwidget.h"
#include "canvas.h"
#include "global.h"
#include "renderinstancewidget.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"

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
    mButtonsLayout->setMargin(MIN_WIDGET_HEIGHT/2);
    mButtonsLayout->addSpacing(MIN_WIDGET_HEIGHT/2);
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
    mButtonsLayout->addSpacing(MIN_WIDGET_HEIGHT/2);

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
}

void RenderWidget::createNewRenderInstanceWidgetForCanvas(Canvas *canvas) {
    RenderInstanceSettings *settings = new RenderInstanceSettings();
    settings->setTargetCanvas(canvas);
    settings->setName(canvas->getName());
    settings->setMinFrame(0);
    settings->setMaxFrame(canvas->getMaxFrame());
    RenderInstanceWidget *wid = new RenderInstanceWidget(settings, this);
    mContLayout->addWidget(wid);
    mRenderInstanceWidgets << wid;
}

void RenderWidget::removeRenderInstanceWidget(RenderInstanceWidget *wid) {
    mRenderInstanceWidgets.removeOne(wid);
    delete wid;
}

void RenderWidget::setRenderedFrame(const int &frame) {
    if(mCurrentRenderedSettings == NULL) return;
    mRenderProgressBar->setValue(frame);
}

void RenderWidget::render() {
    foreach(RenderInstanceWidget *wid, mRenderInstanceWidgets) {
        //if
        RenderInstanceSettings *settings = wid->getSettings();
        mRenderProgressBar->setRange(settings->minFrame(),
                                     settings->maxFrame());
        mRenderProgressBar->setValue(settings->minFrame());
        mCurrentRenderedSettings = settings;
        emit renderFromSettings(settings);
    }
}
