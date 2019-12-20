#include "rendersettingsdisplaywidget.h"
#include "canvas.h"
#include "GUI/global.h"

RenderSettingsDisplayWidget::RenderSettingsDisplayWidget(QWidget * const parent) :
    QWidget(parent) {
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mSceneLabel = new QLabel("<b>Scene:</b>");
    mFrameRangeLabel = new QLabel("<b>Frame range:</b>");
    mResolutionLabel = new QLabel("<b>Resolution:</b>");
    mFpsLabel = new QLabel("<b>Fps:</b>");

    mSceneLabel->setFixedHeight(MIN_WIDGET_DIM);
    mFrameRangeLabel->setFixedHeight(MIN_WIDGET_DIM);
    mResolutionLabel->setFixedHeight(MIN_WIDGET_DIM);
    mFpsLabel->setFixedHeight(MIN_WIDGET_DIM);

    mMainLayout->addWidget(mSceneLabel);
    mMainLayout->addWidget(mFrameRangeLabel);
    mMainLayout->addWidget(mResolutionLabel);
    mMainLayout->addWidget(mFpsLabel);

    mMainLayout->setSpacing(0);
    mMainLayout->setAlignment(Qt::AlignTop);
}

void RenderSettingsDisplayWidget::setRenderSettings(
        const Canvas * const scene,
        const RenderSettings &settings) {
    mSceneLabel->setText("<b>Scene:</b> " +
                         (scene ? scene->prp_getName() : "-none-"));
    mFrameRangeLabel->setText(QString("<b>Frame range:</b> %1 - %2").
                              arg(settings.fMinFrame).arg(settings.fMaxFrame));
    mResolutionLabel->setText(QString("<b>Resolution:</b> %1%    %2 x %3").
                              arg(settings.fResolution*100, 0, 'f', 2).
                              arg(settings.fVideoWidth).
                              arg(settings.fVideoHeight));
    mFpsLabel->setText(QString("<b>Fps:</b> %1fps").arg(settings.fFps, 0, 'f', 2));
}
