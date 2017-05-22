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
class ScrollArea;

class ScrollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollWidget(ScrollArea *parent = 0);

    void updateHeight();
    void setMainTarget(SingleWidgetTarget *target);
    virtual void updateAbstraction();

    void scrollParentAreaBy(const int &by);
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
    ScrollArea *mParentScrollArea;
};

#endif // SCROLLWIDGET_H
