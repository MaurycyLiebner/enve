#ifndef NEWCANVASDIALOG_H
#define NEWCANVASDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include "smartPointers/sharedpointerdefs.h"
class Canvas;
class ColorAnimator;
class ColorAnimatorButton;

class CanvasSettingsDialog : public QDialog {
public:
    CanvasSettingsDialog(Canvas* canvas,
                         QWidget *parent = nullptr);
    CanvasSettingsDialog(const QString &defName,
                         QWidget *parent = nullptr);
    CanvasSettingsDialog(const QString &currName,
                         const int &currWidth,
                         const int &currHeight,
                         const int &currFrameCount,
                         const qreal &currFps,
                         ColorAnimator* bgColorAnimator,
                         QWidget *parent = nullptr);

    int getCanvasWidth();
    int getCanvasHeight();
    QString getCanvasName();
    int getCanvasFrameCount();
    qreal getFps();

    void applySettingsToCanvas(Canvas *canvas);
private:
    QVBoxLayout *mMainLayout;

    QHBoxLayout *mNameLayout;
    QLabel *mNameEditLabel;
    QLineEdit *mNameEdit;

    QHBoxLayout *mSizeLayout;
    QLabel *mWidthLabel;
    QSpinBox *mWidthSpinBox;
    QLabel *mHeightLabel;
    QSpinBox *mHeightSpinBox;

    QHBoxLayout *mFrameCountLayout;
    QLabel *mFrameCountLabel;
    QSpinBox *mFrameCountSpinBox;

    QHBoxLayout *mFPSLayout;
    QLabel *mFPSLabel;
    QDoubleSpinBox *mFPSSpinBox;

    QHBoxLayout *mBgColorLayout;
    QLabel *mBgColorLabel;
    ColorAnimatorButton *mBgColorButton;

    QPushButton *mOkButton;
    QPushButton *mCancelButton;
    QHBoxLayout *mButtonsLayout;
};

#endif // NEWCANVASDIALOG_H
