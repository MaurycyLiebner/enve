#include "renderinstancewidget.h"
#include "global.h"
#include <QFileDialog>
#include <QMenu>
#include "rendersettingsdialog.h"
#include "outputsettingsprofilesdialog.h"
#include "outputsettingsdisplaywidget.h"

RenderInstanceWidget::RenderInstanceWidget(QWidget *parent) :
    ClosableContainer(parent) {
    setCheckable(true);
    setObjectName("darkWidget");
    mNameLabel = new QLabel(this);
    mNameLabel->setTextFormat(Qt::RichText);
    mNameLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    mNameLabel->setObjectName("darkWidget");

    setLabelWidget(mNameLabel);

    QWidget *contWid = new QWidget(this);
    contWid->setLayout(mContentLayout);
    contWid->setObjectName("darkWidget");

    addContentWidget(contWid);

    ClosableContainer *renderSettings = new ClosableContainer();

    QWidget *renderSettingsLabelWidget = new QWidget();
    renderSettingsLabelWidget->setObjectName("darkWidget");
    QHBoxLayout *renderSettingsLayout = new QHBoxLayout();
    renderSettingsLayout->setAlignment(Qt::AlignLeft);
    renderSettingsLayout->setSpacing(0);
    renderSettingsLayout->setMargin(0);

    renderSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);
    QLabel *renderSettingsLabel = new QLabel("Render Settings:", this);
    //renderSettingsLabel->setObjectName("darkWidget");
    renderSettingsLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    renderSettingsLayout->addWidget(renderSettingsLabel);
    renderSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);

    QPushButton *renderSettingsButton = new QPushButton("Settings");
    renderSettingsButton->setObjectName("renderSettings");
    renderSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Minimum);
    connect(renderSettingsButton, SIGNAL(pressed()),
            this, SLOT(openRenderSettingsDialog()));
    renderSettingsLayout->addWidget(renderSettingsButton);

    renderSettingsLabelWidget->setLayout(renderSettingsLayout);
    renderSettings->setLabelWidget(renderSettingsLabelWidget);

    mContentLayout->addWidget(renderSettings);

    mOutputSettings = new ClosableContainer();
    mOutputSettingsDisplayWidget = new OutputSettingsDisplayWidget(this);
    mOutputSettings->addContentWidget(mOutputSettingsDisplayWidget);

    QWidget *outputSettingsLabelWidget = new QWidget();
    outputSettingsLabelWidget->setObjectName("darkWidget");
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

    mOutputSettingsProfilesButton = new OutputProfilesListButton(this);
    connect(mOutputSettingsProfilesButton,
            SIGNAL(profileSelected(OutputSettingsProfile*)),
            this,
            SLOT(outputSettingsProfileSelected(OutputSettingsProfile*)) );
    mOutputSettingsProfilesButton->setObjectName("renderSettings");
    mOutputSettingsProfilesButton->setFixedWidth(MIN_WIDGET_HEIGHT);

    mOutputSettingsButton = new QPushButton("Settings");
    mOutputSettingsButton->setObjectName("renderSettings");
    mOutputSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Minimum);
    connect(mOutputSettingsButton, SIGNAL(pressed()),
            this, SLOT(openOutputSettingsDialog()));
    outputSettingsLayout->addWidget(mOutputSettingsProfilesButton);
    outputSettingsLayout->addWidget(mOutputSettingsButton);

    outputSettingsLayout->addSpacing(6*MIN_WIDGET_HEIGHT);

    QLabel *outputDestinationLabel = new QLabel("Output Destination:", this);
    outputDestinationLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    outputDestinationLabel->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Minimum);
    outputSettingsLayout->addWidget(outputDestinationLabel);
    outputSettingsLayout->addSpacing(MIN_WIDGET_HEIGHT);

    mOutputDestinationButton = new QPushButton("Destination");
    mOutputDestinationButton->setObjectName("renderSettings");
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
    mSettings->setParentWidget(this);
    updateFromSettings();
}

RenderInstanceWidget::~RenderInstanceWidget() {
    delete mSettings;
}

void RenderInstanceWidget::updateFromSettings() {
    RenderInstanceSettings::RenderState renderState =
            mSettings->getCurrentState();
    bool enabled = renderState != RenderInstanceSettings::PAUSED &&
       renderState != RenderInstanceSettings::RENDERING;
    setEnabled(enabled);
    QString nameLabelTxt = "&nbsp;&nbsp;" + mSettings->getName() +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    if(renderState == RenderInstanceSettings::ERROR) {
        nameLabelTxt += "<font color='red'>" +
                        mSettings->getRenderError() +
                        "</font>";
    } else if(renderState == RenderInstanceSettings::FINISHED) {
        nameLabelTxt += "<font color='green'>finished</font>";
    } else if(renderState == RenderInstanceSettings::RENDERING) {
        nameLabelTxt += "rendering...";
    } else if(renderState == RenderInstanceSettings::WAITING) {
        nameLabelTxt += "waiting";
    } else if(renderState == RenderInstanceSettings::PAUSED) {
        nameLabelTxt += "paused";
    }
    mNameLabel->setText(nameLabelTxt);

    QString destinationTxt = mSettings->getOutputDestination();
    if(destinationTxt.isEmpty()) {
        destinationTxt = "Destination";
    }
    mOutputDestinationButton->setText(destinationTxt);
    const OutputSettings &outputSettings = mSettings->getOutputRenderSettings();
    OutputSettingsProfile *outputProfile = mSettings->getOutputSettingsProfile();
    QString outputTxt;
    if(outputProfile == nullptr) {
        AVOutputFormat *formatT = outputSettings.outputFormat;
        if(formatT != nullptr) {
            outputTxt = "Custom " + QString(formatT->long_name);
        } else {
            outputTxt = "Settings";
        }
    } else {
        outputTxt = outputProfile->getName();
    }
    mOutputSettingsButton->setText(outputTxt);
    mOutputSettingsDisplayWidget->setOutputSettings(outputSettings);
}

RenderInstanceSettings *RenderInstanceWidget::getSettings() {
    return mSettings;
}

void RenderInstanceWidget::openOutputSettingsDialog() {
    mSettings->copySettingsFromOutputSettingsProfile();
    const OutputSettings &outputSettings = mSettings->getOutputRenderSettings();
    RenderSettingsDialog *dialog = new RenderSettingsDialog(outputSettings,
                                                            this);
    if(dialog->exec()) {
        mSettings->setOutputSettingsProfile(nullptr);
        OutputSettings outputSettings = dialog->getSettings();
        mSettings->setOutputRenderSettings(outputSettings);
        AVOutputFormat *outputFormat = outputSettings.outputFormat;
        if(outputFormat == nullptr) {
            mOutputSettingsButton->setText("Settings");
        } else {
            mOutputSettingsButton->setText("Custom " +
                        QString(outputFormat->long_name));
        }
        mOutputSettingsDisplayWidget->setOutputSettings(outputSettings);
        updateOutputDestinationFromCurrentFormat();
    }
    delete dialog;
}

void RenderInstanceWidget::updateOutputDestinationFromCurrentFormat() {
    QString outputDst = mSettings->getOutputDestination();
    if(outputDst.isEmpty()) {
        outputDst = QDir::homePath() + "/untitled";
    }
    const OutputSettings &outputSettings = mSettings->getOutputRenderSettings();
    AVOutputFormat *format = outputSettings.outputFormat;
    if(format == nullptr) return;
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

void RenderInstanceWidget::outputSettingsProfileSelected(OutputSettingsProfile *profile) {
    mSettings->setOutputSettingsProfile(profile);
    updateOutputDestinationFromCurrentFormat();
    updateFromSettings();
}

void RenderInstanceWidget::openOutputDestinationDialog() {
    QString supportedExts;
    QString selectedExt;
    const OutputSettings &outputSettings = mSettings->getOutputRenderSettings();
    AVOutputFormat *format = outputSettings.outputFormat;
    if(format != nullptr) {
        QString tmpStr = QString(format->extensions);
        selectedExt = "." + tmpStr.split(",").first();
        tmpStr.replace(",", " *.");
        supportedExts = "Output File (*." + tmpStr + ")";
    }
    QString iniText = mSettings->getOutputDestination();
    if(iniText.isEmpty()) {
        iniText = QDir::homePath() + "/untitled" + selectedExt;
    }
    QString saveAs = QFileDialog::getSaveFileName(this, "Output Destination",
                               iniText,
                               supportedExts);
    if(saveAs.isEmpty()) return;
    mSettings->setOutputDestination(saveAs);
    mOutputDestinationButton->setText(saveAs);
}

void RenderInstanceWidget::openRenderSettingsDialog() {

}

OutputProfilesListButton::OutputProfilesListButton(RenderInstanceWidget *parent) :
    QPushButton(parent) {
    mParentWidget = parent;
    setIcon(QIcon(":/icons/down-arrow.png"));
}

void OutputProfilesListButton::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton) {
        QMenu menu;
        int i = 0;
        foreach(const auto& profile,
                OutputSettingsProfilesDialog::OUTPUT_SETTINGS_PROFILES) {
            QAction *actionT = new QAction(profile->getName());
            actionT->setData(QVariant(i));
            menu.addAction(actionT);
            i++;
        }
        if(OutputSettingsProfilesDialog::OUTPUT_SETTINGS_PROFILES.isEmpty()) {
            menu.addAction("No profiles")->setEnabled(false);
        }
        menu.addSeparator();
        QAction *actionT = new QAction("Edit...");
        actionT->setData(QVariant(-1));
        menu.addAction(actionT);

        QAction *selectedAction = menu.exec(mapToGlobal(QPoint(0, height())));
        if(selectedAction != nullptr) {
            int profileId = selectedAction->data().toInt();
            if(profileId == -1) {
                const OutputSettings &outputSettings =
                        mParentWidget->getSettings()->getOutputRenderSettings();
                OutputSettingsProfilesDialog *profilesDialog =
                        new OutputSettingsProfilesDialog(outputSettings, this);
                if(profilesDialog->exec()) {
                    OutputSettingsProfile *profileT =
                            profilesDialog->getCurrentProfile();
                    emit profileSelected(profileT);
                }
            } else {
                OutputSettingsProfile *profileT =
                        OutputSettingsProfilesDialog::
                        OUTPUT_SETTINGS_PROFILES.at(profileId).get();
                emit profileSelected(profileT);
            }
        }
    }
}
