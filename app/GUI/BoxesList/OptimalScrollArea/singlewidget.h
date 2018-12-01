#ifndef SINGLEWIDGET_H
#define SINGLEWIDGET_H

#include <QWidget>
class SingleWidgetAbstraction;
class ScrollWidgetVisiblePart;

class SingleWidget : public QWidget {
    Q_OBJECT
public:
    SingleWidget(ScrollWidgetVisiblePart *parent = nullptr);
    virtual void setTargetAbstraction(SingleWidgetAbstraction *abs);
    SingleWidgetAbstraction *getTargetAbstraction() {
        return mTarget;
    }

protected:
    SingleWidgetAbstraction *mTarget;
    ScrollWidgetVisiblePart *mParent;
};

#endif // SINGLEWIDGET_H
