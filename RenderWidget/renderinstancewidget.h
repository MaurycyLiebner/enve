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
    QPushButton *mOutputDestinationButton;
    QLabel *mNameLabel;
    QVBoxLayout *mContentLayout = new QVBoxLayout();
    RenderInstanceSettings *mSettings;
signals:

private slots:
    void openOutputSettingsDialog();
    void openOutputDestinationDialog();
    void openRenderSettingsDialog();
};

#endif // RENDERINSTANCEWIDGET_H
