#ifndef GRADIENTSLISTWIDGET_H
#define GRADIENTSLISTWIDGET_H

#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "displayedgradientswidget.h"
class GradientWidget;

class GradientsListWidget : public ScrollArea
{
    Q_OBJECT
public:
    explicit GradientsListWidget(GradientWidget *gradientWidget,
                                 QWidget *parent = 0);

    void setNumberGradients(const int &n) {
        mDisplayedGradients->setNumberGradients(n);
    }

    DisplayedGradientsWidget *getDisplayedGradientsWidget() {
        return mDisplayedGradients;
    }

protected:
    void scrollContentsBy(int dx, int dy);
    DisplayedGradientsWidget *mDisplayedGradients;
signals:

public slots:
};

#endif // GRADIENTSLISTWIDGET_H
