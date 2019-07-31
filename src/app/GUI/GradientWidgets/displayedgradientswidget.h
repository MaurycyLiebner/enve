#ifndef DISPLACYEDGRADIENTSWIDGET_H
#define DISPLACYEDGRADIENTSWIDGET_H

#include "GUI/ColorWidgets/glwidget.h"
#include "GUI/global.h"
class Gradient;
class GradientWidget;

class DisplayedGradientsWidget : public GLWidget {
public:
    explicit DisplayedGradientsWidget(GradientWidget *gradientWidget,
                                      QWidget *parent = nullptr);

    void incTop(const int inc);
    void setTop(const int top);
    void updateTopGradientId();
    void setNumberGradients(const int n);
protected:
    void paintGL();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);

    int mContextMenuGradientId = -1;
    int mDisplayedTop = 0;
    int mTopGradientId = 0;
    int mHoveredGradientId = -1;

    int mNumberVisibleGradients = 6;

    GradientWidget *mGradientWidget;
};

#endif // DISPLACYEDGRADIENTSWIDGET_H
