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
                                 QWidget *parent = nullptr);

    OutputSettingsProfile *getCurrentProfile() {
        if(OUTPUT_SETTINGS_PROFILES.isEmpty()) return nullptr;
        int indexT = mProfilesComboBox->currentIndex();
        if(indexT < 0) return nullptr;
        if(indexT >= OUTPUT_SETTINGS_PROFILES.count()) return nullptr;
        return OUTPUT_SETTINGS_PROFILES.at(indexT).get();
    }

    static QList<OutputSettingsProfileSPtr> OUTPUT_SETTINGS_PROFILES;
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

    OutputSettingsDisplayWidget *mOutputSettingsDisplayWidget = nullptr;

    QHBoxLayout *mButtonsLayout;
    QPushButton *mOkButton = nullptr;
    QPushButton *mCancelButton = nullptr;

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
