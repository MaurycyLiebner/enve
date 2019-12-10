// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#include "renderinstancewidget.h"
#include "GUI/global.h"
#include <QFileDialog>
#include <QMenu>
#include "rendersettingsdialog.h"
#include "outputsettingsprofilesdialog.h"
#include "outputsettingsdisplaywidget.h"
#include "Private/esettings.h"

RenderInstanceWidget::RenderInstanceWidget(QWidget *parent) :
    ClosableContainer(parent) {
    if(!OutputSettingsProfilesDialog::sOutputProfilesLoaded) {
        OutputSettingsProfilesDialog::sOutputProfilesLoaded = true;
        QDir(eSettings::sSettingsDir()).mkdir("OutputProfiles");
        const QString dirPath = eSettings::sSettingsDir() + "/OutputProfiles";
        QDirIterator dirIt(dirPath, QDirIterator::NoIteratorFlags);
        while(dirIt.hasNext()) {
            const auto path = dirIt.next();
            const QFileInfo fileInfo(path);
            if(!fileInfo.isFile()) continue;
            if(!fileInfo.completeSuffix().contains("eProf")) continue;
            const auto profile = enve::make_shared<OutputSettingsProfile>();
            try {
                profile->load(path);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
            OutputSettingsProfilesDialog::sOutputProfiles << profile;
        }
    }

    setCheckable(true);
    setObjectName("darkWidget");
    mNameLabel = new QLabel(this);
    mNameLabel->setTextFormat(Qt::RichText);
    mNameLabel->setFixedHeight(MIN_WIDGET_DIM);
    mNameLabel->setObjectName("darkWidget");

    setLabelWidget(mNameLabel);

    QWidget *contWid = new QWidget(this);
    contWid->setLayout(mContentLayout);
    mContentLayout->setAlignment(Qt::AlignTop);
    contWid->setObjectName("darkWidget");

    addContentWidget(contWid);

    const auto renderSettings = new ClosableContainer();

    QWidget *renderSettingsLabelWidget = new QWidget();
    renderSettingsLabelWidget->setObjectName("darkWidget");
    QHBoxLayout *renderSettingsLayout = new QHBoxLayout();
    renderSettingsLayout->setSpacing(0);
    renderSettingsLayout->setMargin(0);
    renderSettingsLayout->setAlignment(Qt::AlignLeft);

    renderSettingsLayout->addSpacing(MIN_WIDGET_DIM);
    QLabel *renderSettingsLabel = new QLabel("Render Settings:", this);
    //renderSettingsLabel->setObjectName("darkWidget");
    renderSettingsLabel->setFixedHeight(MIN_WIDGET_DIM);
    renderSettingsLayout->addWidget(renderSettingsLabel);
    renderSettingsLayout->addSpacing(MIN_WIDGET_DIM);

    const auto renderSettingsButton = new QPushButton("Settings");
    renderSettingsButton->setObjectName("renderSettings");
    renderSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Maximum);
    connect(renderSettingsButton, &QPushButton::pressed,
            this, &RenderInstanceWidget::openRenderSettingsDialog);
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
    outputSettingsLayout->setSpacing(0);
    outputSettingsLayout->setMargin(0);
    outputSettingsLayout->setAlignment(Qt::AlignTop);

    outputSettingsLayout->addSpacing(MIN_WIDGET_DIM);
    QLabel *outputSettingsLabel = new QLabel("Output Settings:", this);
    outputSettingsLabel->setFixedHeight(MIN_WIDGET_DIM);
    outputSettingsLabel->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    outputSettingsLayout->addWidget(outputSettingsLabel);
    outputSettingsLayout->addSpacing(MIN_WIDGET_DIM);

    mOutputSettingsProfilesButton = new OutputProfilesListButton(this);
    connect(mOutputSettingsProfilesButton, &OutputProfilesListButton::profileSelected,
            this, &RenderInstanceWidget::outputSettingsProfileSelected);
    mOutputSettingsProfilesButton->setObjectName("renderSettings");
    mOutputSettingsProfilesButton->setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
    mOutputSettingsProfilesButton->setIconSize(QSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM));

    mOutputSettingsButton = new QPushButton("Settings");
    mOutputSettingsButton->setObjectName("renderSettings");
    mOutputSettingsButton->setSizePolicy(QSizePolicy::Maximum,
                                         QSizePolicy::Maximum);
    connect(mOutputSettingsButton, &QPushButton::pressed,
            this, &RenderInstanceWidget::openOutputSettingsDialog);
    outputSettingsLayout->addWidget(mOutputSettingsProfilesButton);
    outputSettingsLayout->addWidget(mOutputSettingsButton);

    outputSettingsLayout->addSpacing(6*MIN_WIDGET_DIM);

    QLabel *outputDestinationLabel = new QLabel("Output Destination:", this);
    outputDestinationLabel->setFixedHeight(MIN_WIDGET_DIM);
    outputDestinationLabel->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Maximum);
    outputSettingsLayout->addWidget(outputDestinationLabel);
    outputSettingsLayout->addSpacing(MIN_WIDGET_DIM);

    mOutputDestinationButton = new QPushButton("Destination");
    mOutputDestinationButton->setObjectName("renderSettings");
    mOutputDestinationButton->setSizePolicy(QSizePolicy::Maximum,
                                            QSizePolicy::Maximum);
    connect(mOutputDestinationButton, &QPushButton::pressed,
            this, &RenderInstanceWidget::openOutputDestinationDialog);
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
    connect(mSettings, &RenderInstanceSettings::stateChanged,
            this, &RenderInstanceWidget::updateFromSettings);
    updateFromSettings();
}

RenderInstanceWidget::~RenderInstanceWidget() {
    delete mSettings;
}

void RenderInstanceWidget::updateFromSettings() {
    const auto renderState = mSettings->getCurrentState();
    bool enabled = renderState != RenderState::paused &&
       renderState != RenderState::rendering;
    setEnabled(enabled);
    QString nameLabelTxt = "&nbsp;&nbsp;" + mSettings->getName() +
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    if(renderState == RenderState::error) {
        nameLabelTxt += "<font color='red'>" +
                        mSettings->getRenderError() +
                        "</font>";
    } else if(renderState == RenderState::finished) {
        nameLabelTxt += "<font color='green'>finished</font>";
        mCheckBox->setChecked(false);
    } else if(renderState == RenderState::rendering) {
        nameLabelTxt += "rendering...";
    } else if(renderState == RenderState::waiting) {
        nameLabelTxt += "waiting";
    } else if(renderState == RenderState::paused) {
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
    if(!outputProfile) {
        const auto formatT = outputSettings.outputFormat;
        if(formatT) {
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
    const auto dialog = new RenderSettingsDialog(outputSettings, this);
    if(dialog->exec()) {
        mSettings->setOutputSettingsProfile(nullptr);
        OutputSettings outputSettings = dialog->getSettings();
        mSettings->setOutputRenderSettings(outputSettings);
        const auto outputFormat = outputSettings.outputFormat;
        if(!outputFormat) {
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
    if(outputDst.isEmpty()) outputDst = QDir::currentPath() + "/untitled";
    const OutputSettings &outputSettings = mSettings->getOutputRenderSettings();
    const auto format = outputSettings.outputFormat;
    if(!format) return;
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
    const auto format = outputSettings.outputFormat;
    if(format) {
        QString tmpStr(format->extensions);
        selectedExt = "." + tmpStr.split(",").first();
        tmpStr.replace(",", " *.");
        supportedExts = "Output File (*." + tmpStr + ")";
    }
    QString iniText = mSettings->getOutputDestination();
    if(iniText.isEmpty()) {
        iniText = QDir::currentPath() + "/untitled" + selectedExt;
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

#include "Private/esettings.h"
OutputProfilesListButton::OutputProfilesListButton(RenderInstanceWidget *parent) :
    QPushButton(parent) {
    mParentWidget = parent;
    setIcon(QIcon(eSettings::sIconsDir() + "/down-arrow.png"));
}

void OutputProfilesListButton::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton) {
        QMenu menu;
        int i = 0;
        for(const auto& profile :
            OutputSettingsProfilesDialog::sOutputProfiles) {
            QAction *actionT = new QAction(profile->getName());
            actionT->setData(QVariant(i));
            menu.addAction(actionT);
            i++;
        }
        if(OutputSettingsProfilesDialog::sOutputProfiles.isEmpty()) {
            menu.addAction("No profiles")->setEnabled(false);
        }
        menu.addSeparator();
        QAction *actionT = new QAction("Edit...");
        actionT->setData(QVariant(-1));
        menu.addAction(actionT);

        QAction *selectedAction = menu.exec(mapToGlobal(QPoint(0, height())));
        if(selectedAction) {
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
                        sOutputProfiles.at(profileId).get();
                emit profileSelected(profileT);
            }
        }
    }
}
