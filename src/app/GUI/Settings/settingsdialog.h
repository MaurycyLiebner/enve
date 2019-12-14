#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class PerformanceSettingsWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget * const parent = nullptr);
private:
    void updateSettings();

    PerformanceSettingsWidget* mPerformanceSettingsWidget = nullptr;
};

#endif // SETTINGSDIALOG_H
