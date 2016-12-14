#ifndef WIDGETCONTAINER_H
#define WIDGETCONTAINER_H

#include <QWidget>
#include <QVBoxLayout>

class WidgetContainer : public QWidget
{
    Q_OBJECT
public:
    WidgetContainer(QWidget *parent = 0);

    void setTopWidget(QWidget *widget);
    void addChildWidget(QWidget *widget);
    void setDetailsVisible(bool visible);
protected:
    QVBoxLayout *mAllLayout;
    QWidget *mDetailsWidget;
    QVBoxLayout *mChildWidgetsLayout;
};

#endif // WIDGETCONTAINER_H
