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
        if(overlapFrames == INT_MIN) {
            setOverlapFrames(mLastOverlapFrames);
        } else {
            mOverlapFramesSpin->setValue(overlapFrames);
        }
    }

    void setFrameStep(const int &frameStep) {
        if(frameStep == INT_MIN) {
            setFrameStep(mLastFrameStep);
        } else {
            mFrameStepSpin->setValue(frameStep);
        }
    }

    void setFrameCount(const int &frameCount) {
        if(frameCount == INT_MIN) {
            setFrameCount(mLastFrameCount);
        } else {
            mFrameCountSpin->setValue(frameCount);
        }
    }

    void setFirstFrame(const int &firstFrame) {
        if(firstFrame == INT_MIN) {
            setFirstFrame(mLastFirstFrame);
        } else {
            mFirstFrameSpin->setValue(firstFrame);
        }
    }
private:
    QVBoxLayout *mMainLayout = NULL;

    QHBoxLayout *mFirstFrameLayout = NULL;
    QHBoxLayout *mFrameStepLayout = NULL;
    QHBoxLayout *mFrameCountLayout = NULL;
    QHBoxLayout *mOverlapFramesLayout = NULL;

    QLabel *mFirstFrameLabel = NULL;
    QLabel *mFrameStepLabel = NULL;
    QLabel *mFrameCountLabel = NULL;
    QLabel *mOverlapFramesLabel = NULL;

    QSpinBox *mFirstFrameSpin = NULL;
    QSpinBox *mFrameStepSpin = NULL;
    QSpinBox *mFrameCountSpin = NULL;
    QSpinBox *mOverlapFramesSpin = NULL;

    static int mLastFirstFrame;
    static int mLastFrameStep;
    static int mLastFrameCount;
    static int mLastOverlapFrames;

    QHBoxLayout *mButtonsLayout = NULL;
    QPushButton *mAcceptButton = NULL;
    QPushButton *mCancelButton = NULL;
};

#endif // PAINTBOXSETTINGSDIALOG_H
