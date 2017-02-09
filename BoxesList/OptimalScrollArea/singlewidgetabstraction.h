#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include <QWidget>
class SingleWidgetTarget;
class SingleWidget;
class ScrollWidgetVisiblePart;

class SingleWidgetAbstraction {
public:
    SingleWidgetAbstraction(SingleWidgetTarget *target,
                            ScrollWidgetVisiblePart *visiblePart);
    ~SingleWidgetAbstraction();

    bool setSingleWidgetAbstractions(const int &minY, const int &maxY,
                                     int currY, int currX,
                                     QList<SingleWidget*> *widgets,
                                     int *currentWidgetId);

    int getHeight();

    void setContentVisible(const bool &bT);

    SingleWidgetTarget *getTarget() {
        return mTarget;
    }

    void addChildAbstractionForTarget(SingleWidgetTarget *target);
    void addChildAbstraction(SingleWidgetAbstraction *abs);
    void removeChildAbstractionForTarget(SingleWidgetTarget *target);

    void switchContentVisible();

    bool contentVisible();
private:
    ScrollWidgetVisiblePart *mVisiblePartWidget;

    bool mContentVisible = false;
    SingleWidgetTarget *mTarget;

    QList<SingleWidgetAbstraction*> mChildren;
};

#endif // SINGLEWIDGETABSTRACTION_H
