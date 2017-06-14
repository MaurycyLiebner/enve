#ifndef NEWCANVASDIALOG_H
#define NEWCANVASDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
class Canvas;

class CanvasSettingsDialog : public QDialog
{
public:
    CanvasSettingsDialog(Canvas *canvas,
                         QWidget *parent = NULL);
    CanvasSettingsDialog(const QString &defName,
                         QWidget *parent = NULL);
    CanvasSettingsDialog(const QString &currName,
                         const int &currWidth,
                         const int &currHeight,
                         const int &currFrameCount,
                         const qreal &currFps,
                         QWidget *parent = NULL);

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

    QPushButton *mOkButton;
    QPushButton *mCancelButton;
    QHBoxLayout *mButtonsLayout;
};

#endif // NEWCANVASDIALOG_H
