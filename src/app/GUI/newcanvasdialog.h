#ifndef NEWCANVASDIALOG_H
#define NEWCANVASDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"

class Document;
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
                         const FrameRange &range,
                         const qreal fps,
                         ColorAnimator * const bg,
                         QWidget * const parent = nullptr);

    int getCanvasWidth() const;
    int getCanvasHeight() const;
    QString getCanvasName() const;
    FrameRange getFrameRange() const;
    qreal getFps() const;

    void applySettingsToCanvas(Canvas * const canvas) const;

    static void sNewCanvasDialog(Document &document, QWidget * const parent);
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

    QHBoxLayout *mFrameRangeLayout;
    QLabel *mFrameRangeLabel;
    QSpinBox *mMinFrameSpin;
    QSpinBox *mMaxFrameSpin;

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
