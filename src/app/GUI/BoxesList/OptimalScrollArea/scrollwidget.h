#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "minimalscrollwidget.h"
#include "smartPointers/ememory.h"
class SingleWidget;
class ScrollWidgetVisiblePart;
class SWT_Abstraction;
class SingleWidgetTarget;
class ScrollArea;

class ScrollWidget : public MinimalScrollWidget {
    Q_OBJECT
public:
    explicit ScrollWidget(ScrollArea * const parent);

    void updateHeight();
    virtual void updateAbstraction();

    void setMainTarget(SingleWidgetTarget *target);
    int getContentHeight() {
        return mContentHeight;
    }
private:
    void updateHeightAfterScrollAreaResize(const int parentHeight);
protected:
    virtual void createVisiblePartWidget();

    int mContentHeight = 0;
    qptr<SingleWidgetTarget> mMainTarget;
    stdptr<SWT_Abstraction> mMainAbstraction;
    ScrollWidgetVisiblePart *mVisiblePartWidget = nullptr;
};

#endif // SCROLLWIDGET_H
