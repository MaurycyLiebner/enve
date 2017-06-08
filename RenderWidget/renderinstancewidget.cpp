#include "renderinstancewidget.h"
#include "global.h"
#include <QFileDialog>

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
    setContentWidget(contWid);

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

    ClosableContainer *outputSettings = new ClosableContainer();

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

    QPushButton *outputSettingsButton = new QPushButton("Settings");
    outputSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Minimum);
    connect(outputSettingsButton, SIGNAL(pressed()),
            this, SLOT(openOutputSettingsDialog()));
    outputSettingsLayout->addWidget(outputSettingsButton);

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

    outputSettings->setLabelWidget(outputSettingsLabelWidget);

    mContentLayout->addWidget(outputSettings);

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

}

void RenderInstanceWidget::openOutputDestinationDialog() {
    QString saveAs = QFileDialog::getSaveFileName(this, "Output Destination",
                               "untitled.png",
                               "Output File (*.png)");
    mSettings->setOutputDestination(saveAs);
    mOutputDestinationButton->setText(saveAs);
}

void RenderInstanceWidget::openRenderSettingsDialog() {

}
