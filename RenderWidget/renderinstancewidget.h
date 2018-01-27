#ifndef RENDERINSTANCEWIDGET_H
#define RENDERINSTANCEWIDGET_H

#include "closablecontainer.h"
#include "renderinstancesettings.h"
#include "BoxesList/boxsinglewidget.h"

class RenderInstanceWidget : public ClosableContainer
{
    Q_OBJECT
public:
    RenderInstanceWidget(QWidget *parent = 0);
    RenderInstanceWidget(RenderInstanceSettings *settings,
                         QWidget *parent = 0);
    ~RenderInstanceWidget();

    void updateFromSettings();
    RenderInstanceSettings *getSettings();
private:
    ClosableContainer *mOutputSettings;
    QLabel *mOutputFormatLabel;
    QLabel *mVideoCodecLabel;
    QLabel *mVideoPixelFormatLabel;
    QLabel *mVideoBitrateLabel;
    QLabel *mAudioCodecLabel;
    QLabel *mAudioSampleRateLabel;
    QLabel *mAudioSampleFormatLabel;
    QLabel *mAudioBitrateLabel;
    QLabel *mAudioChannelLayoutLabel;

    QPushButton *mOutputDestinationButton;
    QPushButton *mOutputSettingsButton;
    QLabel *mNameLabel;
    QVBoxLayout *mContentLayout = new QVBoxLayout();
    RenderInstanceSettings *mSettings;
signals:

protected:
    void updateOutputDestinationFromCurrentFormat();
private slots:
    void openOutputSettingsDialog();
    void openOutputDestinationDialog();
    void openRenderSettingsDialog();
};

#endif // RENDERINSTANCEWIDGET_H
