#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget * const parent = nullptr);
private:
    void updateSettings();
    void updateAccPreferenceDesc();

    QCheckBox* mCpuThreadsCapCheck = nullptr;
    QSpinBox* mCpuThreadsCapSpin = nullptr;

    QCheckBox* mRamMBCapCheck = nullptr;
    QSpinBox* mRamMBCapSpin = nullptr;
    QSlider* mRamMBCapSlider = nullptr;

    QLabel* mAccPreferenceLabel = nullptr;
    QLabel* mAccPreferenceDescLabel = nullptr;
    QLabel* mAccPreferenceCpuLabel = nullptr;
    QSlider* mAccPreferenceSlider = nullptr;
    QLabel* mAccPreferenceGpuLabel = nullptr;

    QCheckBox* mPathGpuAccCheck = nullptr;

    QCheckBox* mHddCacheCheck = nullptr;

    QCheckBox* mHddCacheMBCapCheck = nullptr;
    QSpinBox* mHddCacheMBCapSpin = nullptr;
};

#endif // SETTINGSDIALOG_H
