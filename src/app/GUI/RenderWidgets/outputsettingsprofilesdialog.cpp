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

#include "outputsettingsprofilesdialog.h"
#include "GUI/mainwindow.h"
#include "outputsettingsdialog.h"
#include "outputsettingsdisplaywidget.h"
#include "GUI/global.h"
QList<stdsptr<OutputSettingsProfile>> OutputSettingsProfilesDialog::sOutputProfiles;
bool OutputSettingsProfilesDialog::sOutputProfilesLoaded = false;

OutputSettingsProfilesDialog::OutputSettingsProfilesDialog(
        const OutputSettings &currentSettings,
        QWidget *parent) :
 QDialog(parent) {
    setWindowTitle("Output Settings Profile");
    setStyleSheet(MainWindow::sGetInstance()->styleSheet());
    mCurrentSettings = currentSettings;

    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);

    mInnerLayout = new QVBoxLayout();

    mProfileLayout = new QHBoxLayout();
    mProfileLabel = new QLabel("Profile:", this);
    mProfilesComboBox = new QComboBox(this);
    mProfilesComboBox->setMinimumContentsLength(20);
    mProfileLayout->addWidget(mProfileLabel);
    mProfileLayout->addWidget(mProfilesComboBox);
    mProfileLayout->setAlignment(Qt::AlignHCenter);

    mProfileButtonsLayout = new QHBoxLayout();
    mNewProfileButton = new QPushButton("New...", this);
    mNewProfileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mNewProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::createAndEditNewProfile);
    mDuplicateProfileButton = new QPushButton("Duplicate", this);
    mDuplicateProfileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mDuplicateProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::duplicateCurrentProfile);
    mEditProfileButton = new QPushButton("Edit...", this);
    mEditProfileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mEditProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::editCurrentProfile);
    mSaveProfileButton = new QPushButton("Save", this);
    mSaveProfileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mSaveProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::saveCurrentProfile);
    mDeleteProfileButton = new QPushButton("Delete", this);
    mDeleteProfileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mDeleteProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::deleteCurrentProfile);
    mProfileButtonsLayout->addWidget(mNewProfileButton);
    mProfileButtonsLayout->addWidget(mDuplicateProfileButton);
    mProfileButtonsLayout->addWidget(mEditProfileButton);
    mProfileButtonsLayout->addWidget(mSaveProfileButton);
    mProfileButtonsLayout->addWidget(mDeleteProfileButton);
    mNewProfileButton->setObjectName("dialogButton");
    mDuplicateProfileButton->setObjectName("dialogButton");
    mEditProfileButton->setObjectName("dialogButton");
    mSaveProfileButton->setObjectName("dialogButton");
    mDeleteProfileButton->setObjectName("dialogButton");
    mProfileButtonsLayout->setAlignment(Qt::AlignHCenter);

    mOutputSettingsDisplayWidget = new OutputSettingsDisplayWidget(this);
//    mOutputSettingsDisplayWidget->setMinimumSize(10*MIN_WIDGET_HEIGHT,
//                                                 7*MIN_WIDGET_HEIGHT);
    //mOutputSettingsDisplayWidget->setAlwaysShowAll(true);

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mOkButton->setObjectName("dialogButton");
    mCancelButton->setObjectName("dialogButton");

    mOkButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mCancelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mOkButton, &QPushButton::released,
            this, &QDialog::accept);
    connect(mCancelButton, &QPushButton::released,
            this, &QDialog::reject);

    mButtonsLayout->addWidget(mCancelButton);
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->setAlignment(Qt::AlignRight);
    mInnerLayout->setAlignment(Qt::AlignTop);
    mInnerLayout->addLayout(mProfileLayout);
    mInnerLayout->addLayout(mProfileButtonsLayout);
    mInnerLayout->addWidget(mOutputSettingsDisplayWidget);
    mInnerLayout->addStretch(1);
    mInnerLayout->addLayout(mButtonsLayout);
    mainLayout->addLayout(mInnerLayout);
    mStatusBar = new QStatusBar(this);
    mainLayout->addWidget(mStatusBar);

    for(const auto& profile : sOutputProfiles) {
        mProfilesComboBox->addItem(profile->getName());
    }

    updateButtonsEnabled();
    mProfilesComboBox->setEditable(true);
    mProfilesComboBox->setInsertPolicy(QComboBox::NoInsert);
    if(mProfilesComboBox->count() == 0) {
        mProfilesComboBox->setDisabled(true);
    }
    connect(mProfilesComboBox, &QComboBox::editTextChanged,
            this, &OutputSettingsProfilesDialog::setCurrentProfileName);
    connect(mProfilesComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &OutputSettingsProfilesDialog::currentProfileChanged);

    currentProfileChanged();
}

void OutputSettingsProfilesDialog::updateButtonsEnabled() {
    if(sOutputProfiles.isEmpty()) {
        mDuplicateProfileButton->setDisabled(true);
        mEditProfileButton->setDisabled(true);
        mDeleteProfileButton->setDisabled(true);
        mSaveProfileButton->setDisabled(true);
        mProfilesComboBox->setDisabled(true);
    } else {
        mDuplicateProfileButton->setEnabled(true);
        mEditProfileButton->setEnabled(true);
        mDeleteProfileButton->setEnabled(true);
        mSaveProfileButton->setEnabled(true);
        mProfilesComboBox->setEnabled(true);
    }
}

void OutputSettingsProfilesDialog::currentProfileChanged() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) {
        mOutputSettingsDisplayWidget->hide();
        return;
    }
    mOutputSettingsDisplayWidget->show();
    mOutputSettingsDisplayWidget->setOutputSettings(currentProfile->getSettings());
}

void OutputSettingsProfilesDialog::setCurrentProfileName(const QString &name) {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    currentProfile->setName(name);
    mProfilesComboBox->setItemText(mProfilesComboBox->currentIndex(), name);
}

void OutputSettingsProfilesDialog::deleteCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    const int currentId = mProfilesComboBox->currentIndex();
    mProfilesComboBox->removeItem(currentId);
    sOutputProfiles.takeAt(currentId)->removeFile();
    updateButtonsEnabled();
    currentProfileChanged();
}

void OutputSettingsProfilesDialog::duplicateCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    auto newProfile = enve::make_shared<OutputSettingsProfile>();
    newProfile->setSettings(currentProfile->getSettings());
    newProfile->setName(currentProfile->getName() + " copy");
    sOutputProfiles.append(newProfile);
    mProfilesComboBox->addItem(newProfile->getName());
    mProfilesComboBox->setCurrentIndex(mProfilesComboBox->count() - 1);
}

void OutputSettingsProfilesDialog::createAndEditNewProfile() {
    OutputSettingsDialog *dialog = new OutputSettingsDialog(OutputSettings(), this);
    if(dialog->exec()) {
        auto newProfile = enve::make_shared<OutputSettingsProfile>();
        sOutputProfiles.append(newProfile);
        mProfilesComboBox->addItem(newProfile->getName());
        mProfilesComboBox->setCurrentIndex(mProfilesComboBox->count() - 1);
        updateButtonsEnabled();
        newProfile->setSettings(dialog->getSettings());
        currentProfileChanged();
    }
    delete dialog;
}

void OutputSettingsProfilesDialog::editCurrentProfile() {
    const auto currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    const OutputSettings &outputSettings = currentProfile->getSettings();
    const auto dialog = new OutputSettingsDialog(outputSettings, this);
    if(dialog->exec()) {
        currentProfile->setSettings(dialog->getSettings());
        currentProfileChanged();
    }
    delete dialog;
}

void OutputSettingsProfilesDialog::saveCurrentProfile() {
    const auto profile = getCurrentProfile();
    if(!profile) return;
    try {
        profile->save();
        mStatusBar->showMessage("Saved to '" + profile->path() + "'", 2000);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}
