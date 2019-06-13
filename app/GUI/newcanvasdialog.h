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
    CanvasSettingsDialog(Canvas * const canvas,
                         QWidget * const parent = nullptr);
    CanvasSettingsDialog(const QString &defName,
                         QWidget * const parent = nullptr);
    CanvasSettingsDialog(const QString &name,
                         const int width,
                         const int height,
                         const int frameCount,
                         const qreal fps,
                         ColorAnimator * const bg,
                         QWidget * const parent = nullptr);

    int getCanvasWidth() const;
    int getCanvasHeight() const;
    QString getCanvasName() const;
    int getCanvasFrameCount() const;
    qreal getFps() const;

    void applySettingsToCanvas(Canvas * const canvas) const;
private:
    Canvas * mTargetCanvas = nullptr;

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
