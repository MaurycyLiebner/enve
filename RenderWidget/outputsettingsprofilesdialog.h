#ifndef OUTPUTSETTINGSPROFILESDIALOG_H
#define OUTPUTSETTINGSPROFILESDIALOG_H
#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include "renderinstancesettings.h"
class OutputSettingsDisplayWidget;
class OutputSettingsProfilesDialog : public QDialog {
    Q_OBJECT
public:
    OutputSettingsProfilesDialog(const OutputSettings &currentSettings,
                                 QWidget *parent = NULL);

    OutputSettingsProfile *getCurrentProfile() {
        if(OUTPUT_SETTINGS_PROFILES.isEmpty()) return NULL;
        int indexT = mProfilesComboBox->currentIndex();
        if(indexT < 0) return NULL;
        if(indexT >= OUTPUT_SETTINGS_PROFILES.count()) return NULL;
        return OUTPUT_SETTINGS_PROFILES.at(indexT);
    }

    static QList<OutputSettingsProfile*> OUTPUT_SETTINGS_PROFILES;
protected:
    QVBoxLayout *mMainLayout;

    QHBoxLayout *mProfileLayout;
    QLabel *mProfileLabel;
    QComboBox *mProfilesComboBox;

    QHBoxLayout *mProfileButtonsLayout;
    QPushButton *mNewProfileButton;
    QPushButton *mDuplicateProfileButton;
    QPushButton *mEditProfileButton;
    QPushButton *mDeleteProfileButton;

    OutputSettingsDisplayWidget *mOutputSettingsDisplayWidget = NULL;

    QHBoxLayout *mButtonsLayout;
    QPushButton *mOkButton = NULL;
    QPushButton *mCancelButton = NULL;

    OutputSettings mCurrentSettings;

    void updateButtonsEnabled();
protected slots:
    void currentProfileChanged();

    void setCurrentProfileName(const QString &name);
    void deleteCurrentProfile();
    void duplicateCurrentProfile();
    void createAndEditNewProfile();
    void editCurrentProfile();
};

#endif // OUTPUTSETTINGSPROFILESDIALOG_H
