#ifndef GRADIENTSLISTWIDGET_H
#define GRADIENTSLISTWIDGET_H

#include "BoxesList/OptimalScrollArea/scrollarea.h"
class GradientWidget;
class DisplayedGradientsWidget;

class GradientsListWidget : public ScrollArea
{
    Q_OBJECT
public:
    explicit GradientsListWidget(GradientWidget *gradientWidget,
                                 QWidget *parent = 0);

    void setNumberGradients(const int &n);

    DisplayedGradientsWidget *getDisplayedGradientsWidget();

protected:
    void scrollContentsBy(int dx, int dy);
    DisplayedGradientsWidget *mDisplayedGradients;
signals:

public slots:
};

#endif // GRADIENTSLISTWIDGET_H
