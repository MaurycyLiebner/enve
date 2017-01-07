#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include <QWidget>
#include <QVBoxLayout>

class WidgetContainer : public QWidget
{
    Q_OBJECT
public:
    WidgetContainer(QWidget *parent = 0);
    virtual ~WidgetContainer() {}

    void setDetailsVisible(bool visible);

    virtual void initialize();
protected:
    void setTopWidget(QWidget *widget);
    void addChildWidget(QWidget *widget);

    QVBoxLayout *mAllLayout;
    QWidget *mDetailsWidget;
    QVBoxLayout *mChildWidgetsLayout;
};

#endif // WIDGETCONTAINER_H
