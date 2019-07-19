#ifndef GRADIENTSLISTWIDGET_H
#define GRADIENTSLISTWIDGET_H

#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
class GradientWidget;
class DisplayedGradientsWidget;

class GradientsListWidget : public ScrollArea {
public:
    explicit GradientsListWidget(GradientWidget *gradientWidget,
                                 QWidget *parent = nullptr);

    void setNumberGradients(const int n);

    DisplayedGradientsWidget *getDisplayedGradientsWidget();

protected:
    void scrollContentsBy(int dx, int dy);
    DisplayedGradientsWidget *mDisplayedGradients;
};

#endif // GRADIENTSLISTWIDGET_H
