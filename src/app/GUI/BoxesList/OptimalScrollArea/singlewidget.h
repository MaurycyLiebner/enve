#ifndef SINGLEWIDGET_H
#define SINGLEWIDGET_H

#include <QWidget>
class SWT_Abstraction;
class ScrollWidgetVisiblePart;
class SingleWidgetTarget;

class SingleWidget : public QWidget {
    Q_OBJECT
public:
    SingleWidget(ScrollWidgetVisiblePart *parent = nullptr);
    virtual void setTargetAbstraction(SWT_Abstraction *abs);
    SWT_Abstraction *getTargetAbstraction() {
        return mTarget;
    }
    SingleWidgetTarget *getTarget() const;
protected:
    SWT_Abstraction *mTarget;
    ScrollWidgetVisiblePart *mParent;
};

#endif // SINGLEWIDGET_H
