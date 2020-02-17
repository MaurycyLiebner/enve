#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>

class eSettings;

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr);

    void add2HWidgets(QWidget* const widget1,
                      QWidget* const widget2);
    void addWidget(QWidget* const widget);
    void addLayout(QLayout* const layout);
    void addSeparator();

    virtual void applySettings() = 0;
    virtual void updateSettings() = 0;
protected:
    eSettings& mSett;
private:
    QVBoxLayout* mMainLauout = nullptr;
};

#endif // SETTINGSWIDGET_H
