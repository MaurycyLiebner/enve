#ifndef MINIMALSCROLLWIDGET_H
#define MINIMALSCROLLWIDGET_H


#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>
class SingleWidget;
class MinimalScrollWidgetVisiblePart;
class ScrollArea;

class MinimalScrollWidget : public QWidget {
    Q_OBJECT
public:
    explicit MinimalScrollWidget(ScrollArea * const parent);

    virtual void updateHeight() = 0;

    void scrollParentAreaBy(const int by);
signals:

public slots:
    void changeVisibleTop(const int top);
    void changeVisibleHeight(const int height);
    void setWidth(const int width);
protected:
    virtual void createVisiblePartWidget() = 0;
    MinimalScrollWidgetVisiblePart *mMinimalVisiblePartWidget;
    ScrollArea *mParentScrollArea;
};


#endif // MINIMALSCROLLWIDGET_H
