#ifndef RENDERSETTINGSDIALOG_H
#define RENDERSETTINGSDIALOG_H
#include <QDialog>

#include "renderinstancesettings.h"

#include <QLabel>
#include <QDoubleSpinBox>
#include <QHBoxLayout>

class RenderSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit RenderSettingsDialog(const RenderSettings& settings,
                                  QWidget *parent = nullptr);

    RenderSettings getSettings() const;
private:
    void addSeparator();

    void connectDims();
    void disconnectDims();

    void updateValuesFromRes();
    void updateValuesFromWidth();
    void updateValuesFromHeight();

    void restoreInitialSettings();

    const RenderSettings mInitialSettings;

    QLabel *mResolutionLabel = nullptr;
    QDoubleSpinBox *mResolutionSpin = nullptr;

    QLabel *mWidthLabel = nullptr;
    QSpinBox *mWidthSpin = nullptr;

    QLabel *mHeightLabel = nullptr;
    QSpinBox *mHeightSpin = nullptr;

    QLabel *mFrameRangeLabel = nullptr;
    QSpinBox *mMinFrameSpin = nullptr;
    QSpinBox *mMaxFrameSpin = nullptr;

    QLabel *mFpsLabel = nullptr;
    QDoubleSpinBox *mFpsSpin = nullptr;

    QHBoxLayout *mButtonsLayout = nullptr;
    QPushButton *mOkButton = nullptr;
    QPushButton *mCancelButton = nullptr;
    QPushButton *mResetButton = nullptr;
signals:

public slots:
};

#endif // RENDERSETTINGSDIALOG_H
