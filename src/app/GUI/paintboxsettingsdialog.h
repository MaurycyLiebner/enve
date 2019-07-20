#ifndef PAINTBOXSETTINGSDIALOG_H
#define PAINTBOXSETTINGSDIALOG_H
#include <QDialog>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>

class PaintBoxSettingsDialog : public QDialog
{
public:
    PaintBoxSettingsDialog(QWidget *parent = nullptr);
    ~PaintBoxSettingsDialog();

    int getFirstFrame();
    int getFrameStep();
    int getFrameCount();
    int getOverlapFrames();

    void setOverlapFrames(const int overlapFrames) {
        mOverlapFramesSpin->setValue(overlapFrames);
    }

    void setFrameStep(const int frameStep) {
        mFrameStepSpin->setValue(frameStep);
    }
private:
    QVBoxLayout *mMainLayout = nullptr;

    QHBoxLayout *mFrameStepLayout = nullptr;
    QHBoxLayout *mOverlapFramesLayout = nullptr;

    QLabel *mFrameStepLabel = nullptr;
    QLabel *mOverlapFramesLabel = nullptr;

    QSpinBox *mFrameStepSpin = nullptr;
    QSpinBox *mOverlapFramesSpin = nullptr;

    QHBoxLayout *mButtonsLayout = nullptr;
    QPushButton *mAcceptButton = nullptr;
    QPushButton *mCancelButton = nullptr;
};

#endif // PAINTBOXSETTINGSDIALOG_H
