#ifndef RENDERINSTANCEWIDGET_H
#define RENDERINSTANCEWIDGET_H

#include <QWidget>
#include "renderinstancesettings.h"
#include "BoxesList/boxsinglewidget.h"

class RenderInstanceWidget : public QWidget
{
    Q_OBJECT
public:
    RenderInstanceWidget(QWidget *parent = 0);
    RenderInstanceWidget(RenderInstanceSettings *settings,
                         QWidget *parent = 0);

    void updateFromSettings();
private:
    QVBoxLayout *mMainLayout;
    QHBoxLayout *mTitleLayout;
    QPushButton *mContentArrow;
    QLabel *mNameLabel;

    QWidget *mContentWidget;

    RenderInstanceSettings *mSettings;
signals:

public slots:
    void setContentVisible(const bool &bT);
};

#endif // RENDERINSTANCEWIDGET_H
