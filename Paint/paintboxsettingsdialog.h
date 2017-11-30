#ifndef PAINTBOXSETTINGSDIALOG_H
#define PAINTBOXSETTINGSDIALOG_H
#include <QDialog>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>

class PaintBoxSettingsDialog : public QDialog
{
public:
    PaintBoxSettingsDialog(QWidget *parent = NULL);
    ~PaintBoxSettingsDialog();

    int getFirstFrame();
    int getFrameStep();
    int getFrameCount();
    int getOverlapFrames();

    void setOverlapFrames(const int &overlapFrames) {
        mOverlapFramesSpin->setValue(overlapFrames);
    }

    void setFrameStep(const int &frameStep) {
        mFrameStepSpin->setValue(frameStep);
    }
private:
    QVBoxLayout *mMainLayout = NULL;

    QHBoxLayout *mFrameStepLayout = NULL;
    QHBoxLayout *mOverlapFramesLayout = NULL;

    QLabel *mFrameStepLabel = NULL;
    QLabel *mOverlapFramesLabel = NULL;

    QSpinBox *mFrameStepSpin = NULL;
    QSpinBox *mOverlapFramesSpin = NULL;

    QHBoxLayout *mButtonsLayout = NULL;
    QPushButton *mAcceptButton = NULL;
    QPushButton *mCancelButton = NULL;
};

#endif // PAINTBOXSETTINGSDIALOG_H
