#ifndef OUTPUTSETTINGSPROFILESDIALOG_H
#define OUTPUTSETTINGSPROFILESDIALOG_H
#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include "renderinstancesettings.h"
class OutputSettingsDisplayWidget;
class OutputSettingsProfilesDialog : public QDialog {
public:
    OutputSettingsProfilesDialog(const OutputSettings &currentSettings,
                                 QWidget *parent = nullptr);

    OutputSettingsProfile *getCurrentProfile() {
        if(sOutputProfiles.isEmpty()) return nullptr;
        const int index = mProfilesComboBox->currentIndex();
        if(index < 0) return nullptr;
        if(index >= sOutputProfiles.count()) return nullptr;
        return sOutputProfiles.at(index).get();
    }

    static QList<stdsptr<OutputSettingsProfile>> sOutputProfiles;
    static bool sOutputProfilesLoaded;
protected:
    void updateButtonsEnabled();
    void currentProfileChanged();

    void setCurrentProfileName(const QString &name);
    void deleteCurrentProfile();
    void duplicateCurrentProfile();
    void createAndEditNewProfile();
    void editCurrentProfile();
    void saveCurrentProfile();

    QVBoxLayout *mInnerLayout;

    QHBoxLayout *mProfileLayout;
    QLabel *mProfileLabel;
    QComboBox *mProfilesComboBox;

    QHBoxLayout *mProfileButtonsLayout;
    QPushButton *mNewProfileButton;
    QPushButton *mDuplicateProfileButton;
    QPushButton *mEditProfileButton;
    QPushButton *mSaveProfileButton;
    QPushButton *mDeleteProfileButton;

    OutputSettingsDisplayWidget *mOutputSettingsDisplayWidget = nullptr;

    QHBoxLayout *mButtonsLayout;
    QPushButton *mOkButton = nullptr;
    QPushButton *mCancelButton = nullptr;

    QStatusBar* mStatusBar;

    OutputSettings mCurrentSettings;
};

#endif // OUTPUTSETTINGSPROFILESDIALOG_H
