#include "outputsettingsprofilesdialog.h"
#include "mainwindow.h"
#include "rendersettingsdialog.h"
QList<OutputSettingsProfile*> OutputSettingsProfilesDialog::OUTPUT_SETTINGS_PROFILES;

OutputSettingsProfilesDialog::OutputSettingsProfilesDialog(
        const OutputSettings &currentSettings,
        QWidget *parent) :
 QDialog(parent) {
    setWindowTitle("Output Settings Profile");
    setStyleSheet(MainWindow::getInstance()->styleSheet());
    mCurrentSettings = currentSettings;

    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mProfileLayout = new QHBoxLayout();
    mProfileLabel = new QLabel("Profile:", this);
    mProfilesComboBox = new QComboBox(this);
    mProfileLayout->addWidget(mProfileLabel);
    mProfileLayout->addWidget(mProfilesComboBox);

    mProfileButtonsLayout = new QHBoxLayout();
    mNewProfileButton = new QPushButton("New...", this);
    connect(mNewProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::createAndEditNewProfile);
    mDuplicateProfileButton = new QPushButton("Duplicate", this);
    connect(mDuplicateProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::duplicateCurrentProfile);
    mEditProfileButton = new QPushButton("Edit...", this);
    connect(mEditProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::editCurrentProfile);
    mDeleteProfileButton = new QPushButton("Delete", this);
    connect(mDeleteProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::deleteCurrentProfile);
    mProfileButtonsLayout->addWidget(mNewProfileButton);
    mProfileButtonsLayout->addWidget(mDuplicateProfileButton);
    mProfileButtonsLayout->addWidget(mEditProfileButton);
    mProfileButtonsLayout->addWidget(mDeleteProfileButton);

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    connect(mOkButton, &QPushButton::released,
            this, &QDialog::accept);
    connect(mCancelButton, &QPushButton::released,
            this, &QDialog::reject);

    mButtonsLayout->addWidget(mCancelButton, Qt::AlignLeft);
    mButtonsLayout->addWidget(mOkButton, Qt::AlignRight);

    mMainLayout->addLayout(mProfileLayout);
    mMainLayout->addLayout(mProfileButtonsLayout);
    mMainLayout->addLayout(mButtonsLayout);

    foreach(OutputSettingsProfile *profile, OUTPUT_SETTINGS_PROFILES) {
        mProfilesComboBox->addItem(profile->getName());
    }

    updateButtonsEnabled();
    mProfilesComboBox->setEditable(true);
    mProfilesComboBox->setInsertPolicy(QComboBox::NoInsert);
    if(mProfilesComboBox->count() == 0) {
        mProfilesComboBox->setDisabled(true);
    }
    connect(mProfilesComboBox, SIGNAL(editTextChanged(QString)),
            this, SLOT(setCurrentProfileName(QString)));
}

void OutputSettingsProfilesDialog::updateButtonsEnabled() {
    if(OUTPUT_SETTINGS_PROFILES.isEmpty()) {
        mDuplicateProfileButton->setDisabled(true);
        mEditProfileButton->setDisabled(true);
        mDeleteProfileButton->setDisabled(true);
        mProfilesComboBox->setDisabled(true);
    } else {
        mDuplicateProfileButton->setEnabled(true);
        mEditProfileButton->setEnabled(true);
        mDeleteProfileButton->setEnabled(true);
        mProfilesComboBox->setEnabled(true);
    }
}

void OutputSettingsProfilesDialog::setCurrentProfileName(const QString &name) {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(currentProfile == NULL) return;
    currentProfile->setName(name);
    mProfilesComboBox->setItemText(mProfilesComboBox->currentIndex(),
                                   name);
}

void OutputSettingsProfilesDialog::deleteCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(currentProfile == NULL) return;
    int currentId = mProfilesComboBox->currentIndex();
    mProfilesComboBox->removeItem(currentId);
    OUTPUT_SETTINGS_PROFILES.removeAt(currentId);
    updateButtonsEnabled();
}

void OutputSettingsProfilesDialog::duplicateCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(currentProfile == NULL) return;
    OutputSettingsProfile *newProfile = new OutputSettingsProfile();
    newProfile->setSettings(currentProfile->getSettings());
    newProfile->setName(currentProfile->getName() + "1");
    OUTPUT_SETTINGS_PROFILES.append(newProfile);
    mProfilesComboBox->addItem(newProfile->getName());
    mProfilesComboBox->setCurrentIndex(mProfilesComboBox->count() - 1);
}

void OutputSettingsProfilesDialog::createAndEditNewProfile() {
    OutputSettingsProfile *newProfile = new OutputSettingsProfile();
    OUTPUT_SETTINGS_PROFILES.append(newProfile);
    mProfilesComboBox->addItem(newProfile->getName());
    mProfilesComboBox->setCurrentIndex(mProfilesComboBox->count() - 1);
    updateButtonsEnabled();

    editCurrentProfile();
}

void OutputSettingsProfilesDialog::editCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(currentProfile == NULL) return;
    const OutputSettings &outputSettings = currentProfile->getSettings();
    RenderSettingsDialog *dialog = new RenderSettingsDialog(outputSettings,
                                                            this);
    if(dialog->exec()) {
        OutputSettings outputSettings = dialog->getSettings();
        currentProfile->setSettings(outputSettings);
    }
    delete dialog;
}
