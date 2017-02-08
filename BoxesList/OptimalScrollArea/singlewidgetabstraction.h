#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include <QWidget>
class ScrollWidgetVisiblePart;
class SingleWidgetTarget;
class SingleWidget;

class SingleWidgetAbstraction {
public:
    SingleWidgetAbstraction(SingleWidgetTarget *target,
                            ScrollWidgetVisiblePart *visiblePartWidget);
    ~SingleWidgetAbstraction() {
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            delete abs;
        }
    }

    bool setSingleWidgetAbstractions(const int &minY, const int &maxY,
                                     int currY, int currX,
                                     QList<SingleWidget*> *widgets,
                                     int *currentWidgetId);

    int getHeight();

    void addChildAbstraction(SingleWidgetAbstraction *abs);

    void setContentVisible(const bool &bT);

    SingleWidgetTarget *getTarget() {
        return mTarget;
    }

private:
    ScrollWidgetVisiblePart *mVisiblePartWidget;
    bool mContentVisible = true;
    SingleWidgetTarget *mTarget;

    QList<SingleWidgetAbstraction*> mChildren;
};

#endif // SINGLEWIDGETABSTRACTION_H
