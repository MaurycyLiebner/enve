#include "outputsettingsprofilesdialog.h"
#include "GUI/mainwindow.h"
#include "rendersettingsdialog.h"
#include "outputsettingsdisplaywidget.h"
#include "GUI/global.h"
QList<stdsptr<OutputSettingsProfile>> OutputSettingsProfilesDialog::OUTPUT_SETTINGS_PROFILES;

OutputSettingsProfilesDialog::OutputSettingsProfilesDialog(
        const OutputSettings &currentSettings,
        QWidget *parent) :
 QDialog(parent) {
    setWindowTitle("Output Settings Profile");
    setStyleSheet(MainWindow::sGetInstance()->styleSheet());
    mCurrentSettings = currentSettings;

    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mProfileLayout = new QHBoxLayout();
    mProfileLabel = new QLabel("Profile:", this);
    mProfilesComboBox = new QComboBox(this);
    mProfilesComboBox->setMinimumContentsLength(20);
    mProfileLayout->addWidget(mProfileLabel);
    mProfileLayout->addWidget(mProfilesComboBox);
    mProfileLayout->setAlignment(Qt::AlignHCenter);

    mProfileButtonsLayout = new QHBoxLayout();
    mNewProfileButton = new QPushButton("New...", this);
    mNewProfileButton->setSizePolicy(QSizePolicy::Fixed,
                                     QSizePolicy::Fixed);
    connect(mNewProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::createAndEditNewProfile);
    mDuplicateProfileButton = new QPushButton("Duplicate", this);
    mDuplicateProfileButton->setSizePolicy(QSizePolicy::Fixed,
                                           QSizePolicy::Fixed);
    connect(mDuplicateProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::duplicateCurrentProfile);
    mEditProfileButton = new QPushButton("Edit...", this);
    mEditProfileButton->setSizePolicy(QSizePolicy::Fixed,
                                      QSizePolicy::Fixed);
    connect(mEditProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::editCurrentProfile);
    mDeleteProfileButton = new QPushButton("Delete", this);
    mDeleteProfileButton->setSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Fixed);
    connect(mDeleteProfileButton, &QPushButton::released,
            this, &OutputSettingsProfilesDialog::deleteCurrentProfile);
    mProfileButtonsLayout->addWidget(mNewProfileButton);
    mProfileButtonsLayout->addWidget(mDuplicateProfileButton);
    mProfileButtonsLayout->addWidget(mEditProfileButton);
    mProfileButtonsLayout->addWidget(mDeleteProfileButton);
    mNewProfileButton->setObjectName("dialogButton");
    mDuplicateProfileButton->setObjectName("dialogButton");
    mEditProfileButton->setObjectName("dialogButton");
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

    mOkButton->setSizePolicy(QSizePolicy::Fixed,
                             QSizePolicy::Minimum);
    mCancelButton->setSizePolicy(QSizePolicy::Fixed,
                                 QSizePolicy::Minimum);
    connect(mOkButton, &QPushButton::released,
            this, &QDialog::accept);
    connect(mCancelButton, &QPushButton::released,
            this, &QDialog::reject);

    mButtonsLayout->addWidget(mCancelButton);
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->setAlignment(Qt::AlignRight);
    mMainLayout->addLayout(mProfileLayout);
    mMainLayout->addLayout(mProfileButtonsLayout);
    mMainLayout->addWidget(mOutputSettingsDisplayWidget);
    mMainLayout->addLayout(mButtonsLayout);

    for(const auto& profile : OUTPUT_SETTINGS_PROFILES) {
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
    connect(mProfilesComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentProfileChanged()));

    currentProfileChanged();
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
    mProfilesComboBox->setItemText(mProfilesComboBox->currentIndex(),
                                   name);
}

void OutputSettingsProfilesDialog::deleteCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    int currentId = mProfilesComboBox->currentIndex();
    mProfilesComboBox->removeItem(currentId);
    OUTPUT_SETTINGS_PROFILES.removeAt(currentId);
    updateButtonsEnabled();
    currentProfileChanged();
}

void OutputSettingsProfilesDialog::duplicateCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    auto newProfile = enve::make_shared<OutputSettingsProfile>();
    newProfile->setSettings(currentProfile->getSettings());
    newProfile->setName(currentProfile->getName() + "1");
    OUTPUT_SETTINGS_PROFILES.append(newProfile);
    mProfilesComboBox->addItem(newProfile->getName());
    mProfilesComboBox->setCurrentIndex(mProfilesComboBox->count() - 1);
}

void OutputSettingsProfilesDialog::createAndEditNewProfile() {
    RenderSettingsDialog *dialog = new RenderSettingsDialog(OutputSettings(),
                                                            this);
    if(dialog->exec()) {
        auto newProfile = enve::make_shared<OutputSettingsProfile>();
        OUTPUT_SETTINGS_PROFILES.append(newProfile);
        mProfilesComboBox->addItem(newProfile->getName());
        mProfilesComboBox->setCurrentIndex(mProfilesComboBox->count() - 1);
        updateButtonsEnabled();
        OutputSettings outputSettings = dialog->getSettings();
        newProfile->setSettings(outputSettings);
        currentProfileChanged();
    }
    delete dialog;
}

void OutputSettingsProfilesDialog::editCurrentProfile() {
    OutputSettingsProfile *currentProfile = getCurrentProfile();
    if(!currentProfile) return;
    const OutputSettings &outputSettings = currentProfile->getSettings();
    RenderSettingsDialog *dialog = new RenderSettingsDialog(outputSettings,
                                                            this);
    if(dialog->exec()) {
        OutputSettings outputSettings = dialog->getSettings();
        currentProfile->setSettings(outputSettings);
        currentProfileChanged();
    }
    delete dialog;
}
