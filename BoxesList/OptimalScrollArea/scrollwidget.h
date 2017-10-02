#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "minimalscrollwidget.h"
class SingleWidget;
class ScrollWidgetVisiblePart;
class SingleWidgetAbstraction;
class SingleWidgetTarget;
class ScrollArea;

class ScrollWidget : public MinimalScrollWidget {
    Q_OBJECT
public:
    explicit ScrollWidget(ScrollArea *parent = 0);

    void updateHeight();
    void setMainTarget(SingleWidgetTarget *target);
    virtual void updateAbstraction();
    const int &getContentHeight() {
        return mContentHeight;
    }
private slots:
    void updateHeightAfterScrollAreaResize(const int &parentHeight);
protected:
    int mContentHeight = 0;
    virtual void createVisiblePartWidget();
    SingleWidgetTarget *mMainTarget = NULL;
    SingleWidgetAbstraction *mMainAbstraction = NULL;
    ScrollWidgetVisiblePart *mVisiblePartWidget = NULL;
};

#endif // SCROLLWIDGET_H
