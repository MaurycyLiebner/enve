#ifndef BOXSINGLEWIDGET_H
#define BOXSINGLEWIDGET_H

#include "OptimalScrollArea/singlewidget.h"
#include <QLabel>
#include <QHBoxLayout>

class BoxSingleWidget : public SingleWidget
{
    Q_OBJECT
public:
    explicit BoxSingleWidget(QWidget *parent = 0);

    void setTargetAbstraction(SingleWidgetAbstraction *abs);

    void setName(const QString &name);
signals:

public slots:

private:
    QLabel *mNameLabel;
    QHBoxLayout *mMainLayout;
};

#endif // BOXSINGLEWIDGET_H
