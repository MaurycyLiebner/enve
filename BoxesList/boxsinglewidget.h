#ifndef BOXSINGLEWIDGET_H
#define BOXSINGLEWIDGET_H

#include "OptimalScrollArea/singlewidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
class QrealAnimatorValueSlider;

class BoxSingleWidget : public SingleWidget
{
    Q_OBJECT
public:
    explicit BoxSingleWidget(QWidget *parent = 0);

    void setTargetAbstraction(SingleWidgetAbstraction *abs);

    void setName(const QString &name);

signals:

public slots:
    void switchContentVisible();
private:
    QPushButton *mContentButton;
    QLabel *mNameLabel;
    QHBoxLayout *mMainLayout;
    QrealAnimatorValueSlider *mValueSlider;
};

#endif // BOXSINGLEWIDGET_H
