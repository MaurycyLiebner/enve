#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
class SingleWidget;
class ScrollWidgetVisiblePart;
class ScrollWidget;
class SingleWidgetAbstraction;
class SingleWidgetTarget;

class ScrollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollWidget(QWidget *parent = 0);

    void updateHeight();
    void updateAfterContentChange();
    void setMainTarget(SingleWidgetTarget *target);
    void updateAbstraction();
signals:

public slots:
    void changeVisibleTop(const int &top);
    void changeVisibleHeight(const int &height);
    void setWidth(const int &width);
protected:
    virtual void createVisiblePartWidget();
    SingleWidgetTarget *mMainTarget = NULL;
    SingleWidgetAbstraction *mMainAbstraction = NULL;
    ScrollWidgetVisiblePart *mVisiblePartWidget;
};

#endif // SCROLLWIDGET_H
