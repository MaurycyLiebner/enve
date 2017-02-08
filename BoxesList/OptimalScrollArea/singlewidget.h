#ifndef SINGLEWIDGET_H
#define SINGLEWIDGET_H

#include <QWidget>
class SingleWidgetAbstraction;

class SingleWidget : public QWidget
{
    Q_OBJECT
public:
    SingleWidget(QWidget *parent = 0);
    virtual void setTargetAbstraction(SingleWidgetAbstraction *abs);
protected:
    SingleWidgetAbstraction *mTarget;
};

#endif // SINGLEWIDGET_H
